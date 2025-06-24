#include "witness.h"
/**
 * Computes the minimum witnesses of a boolean convolution for each result element
 * The `order` vector specifies the lexicographical order of the indices. 
 */
vector<int> minimum_witness_boolCnv_ordered(vector<int>& a, vector<int>& b, const vector<int>& w, vector<int>& order) {
    //create sqrt(n) vectors depending on what division of sqrt(n) the index is in
    //print a, b, w, order
    // std::cerr << "a: ";
    // for (int i = 0; i < a.size(); i++) {
    //     std::cerr << a[i] << " ";
    // }
    // std::cerr << "\nb: ";
    // for (int i = 0; i < b.size(); i++) {
    //     std::cerr << b[i] << " ";
    // }
    // std::cerr << "\nw: ";
    // for (int i = 0; i < w.size(); i++) {
    //     std::cerr << w[i] << " ";
    // }
    // std::cerr << "\norder: ";
    // for (int i = 0; i < order.size(); i++) {
    //     std::cerr << order[i] << " ";
    // }
    // cerr << endl;

    int n = order.size();
    int sqrt_n = (int)ceil(sqrt(n));
    vector<vector<int>> a_P(sqrt_n + 1, vector<int>(a.size())); //division of a into O(sqrt(n)) parts
    vector<vector<int>> id(sqrt_n + 1, vector<int>());
    for (int i = 0; i < n; i++) {
        if (a[w[order[i]]] == 0) {
            continue;
        }
        a_P[i / sqrt_n][w[order[i]]] = 1;
        id[i/sqrt_n].push_back(order[i]);
    }

    // for (int g  = 0; g < a_P.size(); g++) {
    //     std::cerr << "a_P[" << g << "]: ";
    //     for (int i = 0; i < a_P[g].size(); i++) {
    //         std::cerr << a_P[g][i] << " ";
    //     }
    //     std::cerr << endl;
    // }

    vector<vector<int>> groups(a_P.size()); //group[i] contains the result elements that have their minimum witness in group i

    vector<int> visited(n + b.size() - 1, 0);
    for (int g = 0; g < a_P.size(); g++) {
        vector<int> c_g = boolCnv(a_P[g], b);
        for (int i = 0; i < c_g.size(); i++) {
            if (c_g[i] == 1 && visited[i] == 0) {
                groups[g].push_back(i);
                visited[i] = 1;
            }
        }
    }

    // //print groups
    // for (int g = 0; g < groups.size(); g++) {
    //     std::cerr << "groups[" << g << "]: ";
    //     for (int i = 0; i < groups[g].size(); i++) {
    //         std::cerr << groups[g][i] << " ";
    //     }
    //     std::cerr << endl;
    // }

    vector<int> min_witness(n + b.size() - 1, -1);
    vector<int> inverseOrder(order.size());
    for (int i = 0; i < order.size(); i++) {
        inverseOrder[order[i]] = i;
    }
    for (int g = 0; g < a_P.size(); g++) {
        for (int i : id[g]) {
            for (int result : groups[g]) {
                if (a_P[g][w[i]] == 1 && result - w[i] >= 0 && result - w[i] < b.size() && b[result - w[i]] == 1) {
                    int idx = inverseOrder[i];
                    if (min_witness[result] == -1 || idx < min_witness[result]) {
                        min_witness[result] = idx;
                    }
                }
            }
        }
    }
    //print the minimum witnesses
    // std::cerr << "Minimum witnesses: ";
    // for (int i = 0; i < min_witness.size(); i++) {
    //     std::cerr << min_witness[i] << " ";
    // }
    // std::cerr << endl;
    
    return min_witness;
}