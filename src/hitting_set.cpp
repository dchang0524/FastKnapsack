#include "hitting_set.h"
#include <vector>
#include <list>
#include <algorithm>

using namespace std;

/**
 * Fast greedy hitting-set via bucket queue for O(total input size) time.
 */
vector<int> computeHittingSet(
    const vector<vector<int>>& sets,
    int u, // number of sets
    int R,
    int n  // universe size
) {
    // Inverse index: element -> list of sets containing it
    vector<vector<int>> elementToSets(n + 1);
    for (int s = 0; s < u; ++s) {
        for (int e : sets[s]) {
            if (e >= 1 && e <= n) {
                elementToSets[e].push_back(s);
            }
        }
    }

    vector<bool> covered(u, false);
    int uncoveredCount = u;
    vector<int> hittingSet;

    // Compute initial cover counts and bucket structure
    vector<int> coverCount(n + 1, 0);
    for (int e = 1; e <= n; ++e) {
        coverCount[e] = static_cast<int>(elementToSets[e].size());
    }
    // Buckets indexed by coverCount, range [0..u]
    vector<list<int>> buckets(u + 1);
    vector<list<int>::iterator> pos(n + 1);
    int maxCount = 0;
    for (int e = 1; e <= n; ++e) {
        int c = coverCount[e];
        buckets[c].push_front(e);
        pos[e] = buckets[c].begin();
        maxCount = max(maxCount, c);
    }

    // Greedy selection loop
    while (uncoveredCount > 0 && maxCount > 0) {
        // Pick element with current highest coverage
        int elem = buckets[maxCount].front();
        buckets[maxCount].pop_front();
        coverCount[elem] = 0; // mark as selected
        hittingSet.push_back(elem);

        // Cover all sets hit by this element and update cover counts
        for (int s : elementToSets[elem]) {
            if (!covered[s]) {
                covered[s] = true;
                --uncoveredCount;
                // Decrement cover count for each element in this set
                for (int e2 : sets[s]) {
                    int c2 = coverCount[e2];
                    if (c2 > 0) {
                        buckets[c2].erase(pos[e2]);
                        buckets[c2 - 1].push_front(e2);
                        pos[e2] = buckets[c2 - 1].begin();
                        --coverCount[e2];
                    }
                }
            }
        }
        // Move maxCount down to next non-empty bucket
        while (maxCount > 0 && buckets[maxCount].empty()) {
            --maxCount;
        }
    }
    return hittingSet;
}
