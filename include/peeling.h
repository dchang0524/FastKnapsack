#ifndef PEELING_H
#define PEELING_H

#include "constants.h"

/**
 * Randomized k-matches reconstruction.
 * text: binary string of length n ("0"/"1").
 * pat:  binary pattern string of length m.
 * k:    number of ones to recover per alignment.
 * 
 * Returns a vector of length (n-m+1), where each entry is a sorted list
 * of recovered positions in [0, m).
 */
std::vector<std::vector<int>> k_reconstruct_randomized(
    string &text,
    string &pat,
    int k
);

/**
 * Randomized k-wtitness
 */
vector<std::vector<int>> k_find_witnesses_randomized(
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