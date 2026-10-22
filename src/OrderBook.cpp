#include <Orderbook.h>

void OrderBook::snapshot(const Snapshot& snapshot) {
    for (const Position& position : snapshot.bids) {
        bids[position.price] += position.quantity;
    }

    for (const Position& position : snapshot.asks) {
        bids[position.price] += position.quantity;
    }
}

void OrderBook::update(const Position& position, Side side) {
    array<int64_t, 100> book = (side == Side::BID) ? bids : asks;
    book[position.price] += position.quantity;
}

