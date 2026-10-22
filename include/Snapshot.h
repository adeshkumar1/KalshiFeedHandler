#pragma once

#include <cstdint>
#include <vector>

using namespace std;

struct Position {
    double price;  // in cents
    int64_t quantity;
};

struct Snapshot {
    vector<Position> asks;
    vector<Position> bids;
};
