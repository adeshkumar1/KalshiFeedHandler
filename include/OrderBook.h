#pragma once

#include <array>
#include <Snapshot.h>

using namespace std;

enum class Side {
    BID,
    ASK
};

struct OrderBook {
    // indexed by cents
    array<int64_t, 100> bids {};
    array<int64_t, 100> asks {};

    OrderBook() = default;

    void snapshot(const Snapshot& snapshot);
    void update(const Position& position, Side side);
};
