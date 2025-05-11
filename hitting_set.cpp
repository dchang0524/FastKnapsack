#include "hitting_set.h"
#include <vector>
#include <algorithm>
#include <cmath>
using namespace std;

vector<int> computeHittingSet(
    const vector<vector<int>>& sets,
    int R,
    int n
) {
    int u = sets.size();
    // 1) Build inverted index
    vector<vector<int>> inv(n);
    for (int i = 0; i < u; i++) {
        for (int x : sets[i]) {
            inv[x].push_back(i);
        }
    }
    // 2) freq[x] = number of uncovered sets containing x
    vector<int> freq(n);
    for (int x = 0; x < n; x++) {
        freq[x] = inv[x].size();
    }
    vector<bool> covered(u, false);
    int remaining = u;
    vector<int> H;
    H.reserve((int)ceil((double)n / R * log(u + 1)));
    // 3) Greedy pick
    while (remaining > 0) {
        int best = max_element(freq.begin(), freq.end()) - freq.begin();
        if (freq[best] == 0) break;
        H.push_back(best);
        for (int i : inv[best]) {
            if (!covered[i]) {
                covered[i] = true;
                remaining--;
                for (int y : sets[i]) {
                    freq[y]--;
                }
            }
        }
        freq[best] = 0;
    }
    return H;
}
