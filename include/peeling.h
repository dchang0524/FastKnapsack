#ifndef PEELING_H
#define PEELING_H

#include "constants.h"

// For alignment i, return min(k, matches) distinct pattern positions j
// with text[i+j] == pat[j] == '1'. Results are sorted and exact.
std::vector<std::vector<int>> k_reconstruct_randomized(
    string &text,
    string &pat,
    int k
);

// Deterministic k-reconstruction using the explicit c-wise almost-independent
// dilution space and exact convolution witness counts.
std::vector<std::vector<int>> k_reconstruct_deterministic(
    string &text,
    string &pat,
    int k
);

// Binary-array version of the same alignment contract.
vector<std::vector<int>> k_find_witnesses_randomized(
    vector<int> &a,
    vector<int> &b,
    int k
);

vector<std::vector<int>> k_find_witnesses_deterministic(
    vector<int> &a,
    vector<int> &b,
    int k
);

vector<vector<int>> k_find_witnesses_knapsack(
    vector<int> &a,
    vector<int> &b,
    vector<int> &order,
    vector<int> &w,
    int k
);

#endif
