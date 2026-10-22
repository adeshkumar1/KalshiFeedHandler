#pragma once

#include <OrderBook.h>
#include <string>

using namespace std;

class KalshiClient {
private:
    OrderBook order_book;
    const string market_ticker;
    const string private_key_path;
    const string kalshi_api_key;

public:
    KalshiClient(const string& kalshi_api_key, const string& private_key_path, const string& market_ticker) : market_ticker(market_ticker), private_key_path(private_key_path), kalshi_api_key(kalshi_api_key) {}
    void connect();
};