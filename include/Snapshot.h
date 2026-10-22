#pragma once

#include <vector>

using namespace std;

struct Position {
    int64_t price; // in cents
    int64_t quantity;
};

struct Snapshot {
    vector<Position> asks;
    vector<Position> bids;
};