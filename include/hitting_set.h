#ifndef HITTING_SET_H
#define HITTING_SET_H

#include "constants.h"

// Given `sets` (u subsets of [0..n)), each of size ≥ R,
// returns a hitting set H ⊆ [0..n) with |H| ≤ ⌈(n/R)·ln(u)⌉.
vector<int> computeHittingSet(
    const vector<vector<int>>& sets,
    int R,
    int n
);

#endif // HITTING_SET_H
