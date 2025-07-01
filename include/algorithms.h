#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include "constants.h"
#include "dp_structs.h"
#include "convolution.h"
#include "dp_structs.h"
#include "witness.h"
#include "hitting_set.h"

// Algorithm 1: Witness Propagation
void propagation(
    const vector<int>& w,
    const vector<int>& p,
    int t,
    vector<solution>& sol, 
    const vector<int>& order
);

// Algorithm 2: Kernel Computation
void kernelComputation_knapsack(
    int n, int u,
    const vector<int>& w,
    const vector<int>& p,
    const vector<int>& order,
    int t,
    vector<solution>& sol
);

void kernelComputation_coinchange_simple(
    int n, int u,
    const vector<int>& w,
    const vector<int>& p,
    vector<int>& order,
    int t,
    vector<solution>& sol
);

void kernelComputation_coinchange_randomized(
    int n,                              // number of coins
    int u,                              // maximum coin weight
    const vector<int>& w,               // weights of the coins (1-indexed)
    const vector<int>& p,               // profits of the coins  (1-indexed)
    vector<int>& order,           // lexicographical order σ[1..n]
    int t,                          // (unused) global target bound
    vector<solution>& sol              // output: sol[c] for c∈[0..k·u]
);

void kernelComputation_coinchange(
    int n, int u,
    const vector<int>& w,
    const vector<int>& p,
    vector<int>& order,
    int t,
    vector<solution>& sol
);

// Algorithm 4: Adaptive Minimum Witness
void adaptiveMinWitness(
    vector<vector<int>>& a,
    vector<vector<int>>& b,
    vector<vector<int>>& c,
    vector<int>& sigma
);

#endif // ALGORITHMS_H
