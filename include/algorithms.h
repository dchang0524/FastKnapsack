#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include "constants.h"
#include "dp_structs.h"
#include "convolution.h"
#include "dp_structs.h"
#include "peeling.h"
#include "hitting_set.h"

// Algorithm 1: Witness Propagation
void propagation(
    const vector<int>& w,
    const vector<int>& p,
    int t,
    vector<solution>& sol
);

// Algorithm 2: Kernel Computation with Minimum Witness
void kernelComputation_knapsack(
    int n, int u,
    const vector<int>& w,
    const vector<int>& p,
    const vector<int>& order,
    int t,
    vector<solution>& sol
);

void kernelComputation_coinchange(
    int n, int u,
    vector<int>& w,
    vector<int>& p,
    vector<int>& order,
    int t,
    vector<solution>& sol
);

// Algorithm 4: Adaptive Minimum Witness (boolean‐only version)
void adaptiveMinWitness(
    vector<vector<int>>& a,
    vector<vector<int>>& b,
    vector<vector<int>>& c,
    vector<int>& sigma
);

#endif // ALGORITHMS_H
