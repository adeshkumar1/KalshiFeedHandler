#include <KalshiClient.h>

#include <iostream>

int main(int argc, char** argv) {
    if (argc != 3) {
        cout << "invalid amount of arguements, usage: ./KalshiFeedHandler <private_key_path> "
                "<market_ticker>"
             << '\n';
        return -1;
    }

    char* raw_ptr = getenv("KALSHI_API_KEY");
    if (raw_ptr == nullptr) {
        cout << "make sure KALSHI_API_KEY is set." << '\n';
        return -1;
    }

    string kalshi_api_key (raw_ptr);

    string private_key_path = argv[1];
    string market_ticker = argv[2];

    KalshiClient kalshi_client(kalshi_api_key, private_key_path, market_ticker);
    kalshi_client.connect();
    return 0;
}
