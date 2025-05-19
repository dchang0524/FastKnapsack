#pragma once

#include "constants.h"

/// Represents one “solution” (a multiset of coin‐types plus its aggregate stats).
class solution {
public:
    int size;               ///< total number of coins used
    int value;              ///< total profit
    int weight;             ///< total weight
    map<int,int> svec; ///< map from coin‐index → count

    /// Default‐construct an empty solution.
    solution();

    /// Add one coin of index i, with given weight & profit.
    void addCoin(int i, int wgt, int prof);

    /// Dump contents for debugging.
    void dump() const;

    /// Lexicographically compare two solutions by ascending coin‐index order.
    /// Returns true iff s1(this) < s2.
    bool lexCmp(const solution& s2);

    //Copies s1 over to s2
    void copy(solution &s2);
};
