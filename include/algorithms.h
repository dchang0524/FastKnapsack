#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <vector>
#include "dp_structs.h"
using namespace std;

// Algorithm 1: Witness Propagation
void propagation(
    const vector<int>& w,
    const vector<int>& p,
    int t,
    vector<solution>& sol
);

// Algorithm 2: Kernel Computation with Minimum Witness
void kernelComputation(
    int n, int u,
    const vector<int>& w,
    const vector<int>& p,
    const vector<int>& order,
    int t,
    vector<solution>& sol
);

// Algorithm 4: Adaptive Minimum Witness (boolean‐only version)
void adaptiveMinWitness(
    const vector<vector<int>>& A,
    const vector<vector<int>>& B,
    vector<vector<int>>& c,
    vector<int>& sigma
);

#endif // ALGORITHMS_H
