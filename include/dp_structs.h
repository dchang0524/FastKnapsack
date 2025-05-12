#ifndef DP_STRUCTS_H
#define DP_STRUCTS_H

#include <vector>
using namespace std;

const int NEG_INF = -1000000000; // Negative infinity placeholder

// Represents a solution vector (counts m_i) with its total value & weight.
struct solution {
    int size;            // sum of m_i
    int value;           // total profit
    int weight;          // total weight
    vector<int> svec;    // multiplicities m_i, change to a map later

    solution(int n=0)
      : size(0), value(0), weight(0), svec(n+1,0) {}
};

// Lexicographical compare: return true if s1 < s2 under some fixed order.
bool lexCmp(const solution& s1, const solution& s2);

#endif // DP_STRUCTS_H
