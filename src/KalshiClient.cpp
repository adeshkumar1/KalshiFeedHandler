#include <KalshiClient.h>
#include <openssl/evp.h>
#include <openssl/pem.h>

#include <boost/asio/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <chrono>
#include <cstdint>
#include <iostream>

#include "OrderBook.h"
/*
connect should:
1. load private key
2. create websocket headers
3. subscribe to orderbook
4. process messages
*/

string KalshiClient::sign_message(const string& msg) {
    BIO* file = BIO_new_file(private_key_path.c_str(), "rb");
    if (!file) {
        throw std::runtime_error("Could not open private key file");
    }

    EVP_PKEY* key = PEM_read_bio_PrivateKey(file, nullptr, nullptr, nullptr);

    BIO_free(file);

    if (!key) {
        throw std::runtime_error("Could not read private key");
    }

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) {
        EVP_PKEY_free(key);
        throw std::runtime_error("Could not allocate signing context");
    }

    if (EVP_DigestSignInit(ctx, nullptr, nullptr, nullptr, key) != 1) {
        EVP_MD_CTX_free(ctx);
        EVP_PKEY_free(key);
        throw std::runtime_error("Could not initialize signing");
    }

    const auto* data = reinterpret_cast<const unsigned char*>(msg.data());

    size_t signature_size = 0;
    if (EVP_DigestSign(ctx, nullptr, &signature_size, data, msg.size()) != 1) {
        EVP_MD_CTX_free(ctx);
        EVP_PKEY_free(key);
        throw std::runtime_error("Could not determine signature size");
    }

    std::vector<unsigned char> signature(signature_size);

    if (EVP_DigestSign(ctx, signature.data(), &signature_size, data, msg.size()) != 1) {
        EVP_MD_CTX_free(ctx);
        EVP_PKEY_free(key);
        throw std::runtime_error("Could not sign message");
    }

    signature.resize(signature_size);

    EVP_MD_CTX_free(ctx);
    EVP_PKEY_free(key);

    string encoded(4 * ((signature.size() + 2) / 3) + 1, '\0');

    int length = EVP_EncodeBlock(reinterpret_cast<unsigned char*>(encoded.data()), signature.data(),
                                 static_cast<int>(signature.size()));

    if (length < 0) {
        throw std::runtime_error("Could not encode signature");
    }

    encoded.resize(length);  // Remove the trailing null terminator.
    return encoded;
}

json KalshiClient::create_headers() {
    json headers;

    uint64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                             std::chrono::system_clock::now().time_since_epoch())
                             .count();
    string msg_string = to_string(timestamp) + "GET" + "/trade-api/ws/v2";

    string signature = sign_message(msg_string);

    headers["Content-Type"] = "application/json";
    headers["KALSHI-ACCESS-KEY"] = kalshi_api_key;
    headers["KALSHI-ACCESS-SIGNATURE"] = signature;
    headers["KALSHI-ACCESS-TIMESTAMP"] = to_string(timestamp);

    return headers;
}

void KalshiClient::connect() {
    namespace net = boost::asio;
    namespace beast = boost::beast;
    namespace websocket = beast::websocket;
    namespace ssl = net::ssl;
    using tcp = net::ip::tcp;

    const string host = "external-api-ws.kalshi.com";
    const string port = "443";
    const string path = "/trade-api/ws/v2";

    net::io_context io;
    ssl::context tls(ssl::context::tls_client);

    tls.set_default_verify_paths();
    tls.set_verify_mode(ssl::verify_peer);

    tcp::resolver resolver(io);

    websocket::stream<beast::ssl_stream<beast::tcp_stream>> ws(io, tls);

    auto endpoints = resolver.resolve(host, port);
    beast::get_lowest_layer(ws).connect(endpoints);

    // Verify that the certificate belongs to this hostname.
    ws.next_layer().set_verify_callback(ssl::host_name_verification(host));

    // Tell the server which hostname we're connecting to.
    if (SSL_set_tlsext_host_name(ws.next_layer().native_handle(), host.c_str()) != 1) {
        throw std::runtime_error("Could not set TLS server name");
    }

    ws.next_layer().handshake(ssl::stream_base::client);
    json ws_headers = create_headers();

    ws.set_option(websocket::stream_base::decorator([ws_headers](websocket::request_type& request) {
        for (const auto& [name, value] : ws_headers.items()) {
            request.set(name, value.get<string>());
        }
    }));

    ws.handshake(host, path);

    json subscription = {{"id", 1},
                         {"cmd", "subscribe"},
                         {"params",
                          {{"channels", json::array({"orderbook_delta"})},
                           {"market_tickers", json::array({market_ticker})}}}};

    string message = subscription.dump();

    ws.text(true);
    ws.write(net::buffer(message));

    beast::flat_buffer buffer;

    while (true) {
        boost::system::error_code ec;
        ws.read(buffer, ec);  // Blocks until a complete message arrives.

        if (ec == websocket::error::closed) {
            break;
        }
        if (ec) {
            throw boost::system::system_error(ec);
        }

        string text = beast::buffers_to_string(buffer.data());
        buffer.consume(buffer.size());  // Clear before the next read.

        json incoming = json::parse(text);
        std::cout << incoming.dump(2) << '\n';

        string msg_type = incoming["type"];

        if (msg_type == "orderbook_snapshot") {
            {
                const auto& levels = incoming["msg"]["yes_dollars_fp"];
                for (const auto& level : levels) {
                    string price_str = level[0];
                    string quantity_str = level[1];

                    double price = stod(price_str);
                    int64_t quantity = stoll(quantity_str);

                    order_book.update({price, quantity}, Side::BID);
                }
            }
            {
                const auto& levels = incoming["msg"]["no_dollars_fp"];
                for (const auto& level : levels) {
                    string price_str = level[0];
                    string quantity_str = level[1];

                    double price = stod(price_str);
                    int64_t quantity = static_cast<int64_t>(stod(quantity_str));

                    order_book.update({price, quantity}, Side::ASK);
                }
            }
        } else if (msg_type == "orderbook_delta") {
            auto& msg = incoming["msg"];

            Side side = (incoming["msg"]["side"] == "yes") ? Side::BID : Side::ASK;

            string price_str = msg["price_dollars"];
            string quantity_str = msg["delta_fp"];

            double price = stod(price_str);
            int64_t quantity = static_cast<int64_t>(stod(quantity_str));

            order_book.update({price, quantity}, side);
        }
    }

    return;
}
