#ifndef PEELING_H
#define PEELING_H

#include "constants.h"
#include "convolution.h"

// Performs the "peel" to extract up to k witnesses per slot.
// F: family of R subsets, z[A][i]=count, mA[A][i]=sum
vector<vector<int>> peelWitnesses(
    const vector<vector<int>>& F,
    vector<vector<int>>& z,
    vector<vector<int>>& mA,
    int universeSize,
    int k
);

// Wraps the two‐convolutions + peel to solve the k‐reconstruction problem.
vector<vector<int>> k_reconstruction(
    const vector<int>& a,
    const vector<int>& b,
    const vector<vector<int>>& F,
    int k
);

#endif // PEELING_H
