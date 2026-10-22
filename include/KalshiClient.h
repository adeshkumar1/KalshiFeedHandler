#pragma once

#include <OrderBook.h>

#include <nlohmann/json.hpp>
#include <string>

using namespace std;
using json = nlohmann::json;

class KalshiClient {
private:
    OrderBook order_book;

    const string market_ticker;
    const string private_key_path;
    const string kalshi_api_key;

    const string WS_URL = "wss://external-api-ws.demo.kalshi.co/trade-api/ws/v2";

    string sign_message(const string& msg);
    json create_headers();

public:
    KalshiClient(const string& kalshi_api_key, const string& private_key_path,
                 const string& market_ticker)
        : market_ticker(market_ticker),
          private_key_path(private_key_path),
          kalshi_api_key(kalshi_api_key) {}
    void connect();
};
