#include "witness.h"

/**
 * Computes the minimum witnesses of a boolean convolution for each result element
 * The `order` vector specifies the lexicographical order of the indices. 
 */
vector<int> minimum_witness_boolCnv_ordered(vector<int>& a, vector<int>& b, vector<int>& order) {
    vector<vector<int>> a_P(order.size(), vector<int>(a.size()));
    //create sqrt(n) vectors depending on what division of sqrt(n) the index is in
    int n = a.size();
    int sqrt_n = (int)ceil(sqrt(n));
    for (int i = 0; i < n; i++) {
        a_P[i / sqrt_n][order[i]] = 1;
    }
    vector<int> b_P(b.size());
    for (int i = 0; i < b.size(); i++) {
        b_P[i] = b[i];
    }
    vector<vector<int>> groups(sqrt_n); //group[i] contains the result elements that have their minimum witness in group i
    vector<int> visited(a.size(), 0);
    for (int g = 0; g < sqrt_n; g++) {
        vector<int> c_g = boolCnv(a_P[g], b_P);
        for (int i = 0; i < c_g.size(); i++) {
            if (c_g[i] == 1 && visited[i] == 0) {
                groups[g].push_back(i);
                visited[i] = 1;
            }
        }
    }
    unordered_set<int> bSet;
    for (int i = 0; i < b.size(); i++) {
        if (b[i] == 1) {
            bSet.insert(i);
        }
    }
    //since a_P[g] is sparse, change it into a hashset for faster access
    vector<unordered_set<int>> a_Set(sqrt_n);
    for (int g = 0; g < sqrt_n; g++) {
        for (int i = 0; i < a_P[g].size(); i++) {
            if (a_P[g][i] == 1) {
                a_Set[g].insert(i);
            }
        }
    }
    //create an inverse array for order to compare minimum witnesses
    vector<int> inverseOrder(order.size());
    for (int i = 0; i < order.size(); i++) {
        inverseOrder[order[i]] = i;
    }

    vector<int> min_witness(n + b.size() - 1, -1);
    for (int g = 0; g < sqrt_n; g++) {
        for (int i : a_Set[g]) {
            //a_Set[g] <- the nonzero elements in the g-th group
            for (int j = 0; j < groups[g].size(); j++) {
                //groups[g][j] <- the j-th result element in the g-th group
                if (bSet.count(groups[g][j] - i) == 0) {
                    continue;
                } else {
                    if (min_witness[groups[g][j]] == -1 || 
                        inverseOrder[i] < min_witness[groups[g][j]]) {
                        min_witness[groups[g][j]] = inverseOrder[i];
                    }
                }
                
            }
        }
    }
    return min_witness;
}
