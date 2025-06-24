#include "witness.h"

/**
 * Computes the minimum witnesses of a boolean convolution for each result element
 * The `order` vector specifies the lexicographical order of the indices. 
 */
vector<int> minimum_witness_boolCnv_ordered(vector<int>& a, vector<int>& b, vector<int>& order) {
    //create sqrt(n) vectors depending on what division of sqrt(n) the index is in
    int n = a.size();
    int sqrt_n = (int)ceil(sqrt(n));
    vector<vector<int>> a_P(sqrt_n + 1, vector<int>(a.size())); //division of a into O(sqrt(n)) parts
    for (int i = 0; i < n; i++) {
        if (a[order[i]] == 0) {
            continue;
        }
        a_P[i / sqrt_n][order[i]] = 1;
    }
    // vector<int> b_P(b.size());
    // for (int i = 0; i < b.size(); i++) {
    //     b_P[i] = b[i];
    // }
    vector<vector<int>> groups(a_P.size()); //group[i] contains the result elements that have their minimum witness in group i
    vector<int> visited(n + b.size() - 1, 0);
    for (int g = 0; g < sqrt_n; g++) {
        vector<int> c_g = boolCnv(a_P[g], b);
        for (int i = 0; i < c_g.size(); i++) {
            if (c_g[i] == 1 && visited[i] == 0) {
                groups[g].push_back(i);
                visited[i] = 1;
            }
        }
    }

    vector<int> min_witness(n + b.size() - 1, -1);
    // int lastProcessed = 0;
    // for (int g = 0; g < a_P.size(); g++) {
    //     for (int i = lastProcessed + 1; i < lastProcessed + a_P[g].size() && i <= n; i++) {
    //         if (a[order[i]] == 0) {
    //             continue;
    //         }
    //         for (int result : groups[g]) {
    //             if (min_witness[result] != -1 && b[result - order[i]] == 1) {
    //                 min_witness[result] = i;
    //             }
    //         }
    //     }
    //     lastProcessed += a_P[g].size();
    // }

    vector<int> inverseOrder(order.size());
    for (int i = 0; i < order.size(); i++) {
        inverseOrder[order[i]] = i;
    }
    for (int g = 0; g < a_P.size(); g++) {
        for (int i = 0; i < a_P[g].size(); i++) {
            for (int result : groups[g]) {
                if (a_P[g][i] == 1 && result - i >= 0 && result - i < b.size() && b[result - i] == 1) {
                    int idx = inverseOrder[i];
                    if (min_witness[result] == -1 || idx < min_witness[result]) {
                        min_witness[result] = idx;
                    }
                }
            }
        }
        
    }
    return min_witness;
}
