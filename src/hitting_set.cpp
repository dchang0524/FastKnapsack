#include "hitting_set.h"
#include <queue>
#include <stdexcept>

vector<int> computeHittingSet(
    const vector<vector<int>>& sets, int u, int R, int n
) {
    if (u != static_cast<int>(sets.size()) || R <= 0 || n < 0)
        throw invalid_argument("invalid hitting-set dimensions");
    vector<vector<int>> incidence(n);
    for (int set = 0; set < u; ++set) {
        if (static_cast<int>(sets[set].size()) < R)
            throw invalid_argument("hitting-set input below threshold");
        for (int element : sets[set]) {
            if (element < 0 || element >= n)
                throw invalid_argument("hitting-set element out of range");
            incidence[element].push_back(set);
        }
    }
    vector<int> coverage(n);
    priority_queue<pair<int, int>> queue;
    for (int element = 0; element < n; ++element) {
        coverage[element] = static_cast<int>(incidence[element].size());
        if (coverage[element]) queue.emplace(coverage[element], element);
    }
    vector<unsigned char> covered(u), selected(n);
    vector<int> result;
    int left = u;
    while (left) {
        while (!queue.empty() &&
               (selected[queue.top().second] ||
                queue.top().first != coverage[queue.top().second]))
            queue.pop();
        if (queue.empty() || queue.top().first == 0)
            throw runtime_error("hitting-set input contains an uncovered empty set");
        int element = queue.top().second;
        queue.pop();
        selected[element] = 1;
        result.push_back(element);
        for (int set : incidence[element]) if (!covered[set]) {
            covered[set] = 1;
            --left;
            for (int other : sets[set]) if (!selected[other]) {
                --coverage[other];
                queue.emplace(coverage[other], other);
            }
        }
    }
    return result;
}
