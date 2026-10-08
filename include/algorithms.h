#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include "constants.h"
#include "dp_structs.h"
#include "convolution.h"
#include "dp_structs.h"
#include "witness.h"
#include "hitting_set.h"
#include <functional>

using MaxPlusBackend = std::function<vector<ll>(
    const vector<ll>&, const vector<ll>&)>;

// Algorithm 1: Witness Propagation
void propagation(
    const vector<int>& w,
    const vector<int>& p,
    int t,
    vector<solution>& sol, 
    const vector<int>& order
);

// Algorithm 2: Kernel Computation
// The paper's knapsack running time is conditional on this exact max-plus
// convolution backend. The built-in maxPlusCnv backend is quadratic.
void kernelComputation_knapsack(
    int n, int u,
    const vector<int>& w,
    const vector<int>& p,
    const vector<int>& order,
    int t,
    vector<solution>& sol,
    const MaxPlusBackend& convolve = maxPlusCnv
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
    vector<solution>& sol,
    bool randomized = false,
    bool optimized_peeling = false
);

void kernelComputation_coinchange_optimized(
    int n, int u, const vector<int>& w, const vector<int>& p,
    vector<int>& order, int t, vector<solution>& sol
);

// Algorithm 4: Adaptive Minimum Witness
// `order` is a 0-based permutation of positions in the first input arrays.
// Returns the minimum first-array witness at every convolution output and
// changes `order` to the permutation constructed by Algorithm 4.
vector<vector<int>> adaptiveMinWitness(
    const vector<vector<int>>& a,
    const vector<vector<int>>& b,
    vector<int>& order,
    bool randomized = false,
    const vector<vector<unsigned char>>* requested = nullptr,
    bool optimized_peeling = false
);

vector<vector<int>> adaptiveMinWitness_optimized(
    const vector<vector<int>>& a,
    const vector<vector<int>>& b,
    vector<int>& order,
    const vector<vector<unsigned char>>* requested = nullptr
);

vector<vector<int>> adaptiveMinWitness_randomized(
    vector<vector<int>>& a,
    vector<vector<int>>& b,
    vector<vector<int>>& c,
    vector<int>& w,
    vector<int>& order
);

#endif
