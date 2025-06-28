#include <bits/stdc++.h>
#include "convolution.h"
#include "dp_structs.h"
using namespace std;

// Counters for asymptotic blocks
unsigned long long count_O1       = 0;  // number of O(1) blocks
unsigned long long count_On_Conv = 0;  // number of boolean convolutions

// wrap boolCnv to count one O(N·√N) block per call
vector<int> countedBoolCnv(const vector<int>& a, const vector<int>& b) {
    ++count_On_Conv;
    return boolCnv(a, b);
}

vector<int> minimum_witness_boolCnv_ordered(
    const vector<int>& a,
    const vector<int>& b,
    const vector<int>& w,
    const vector<int>& order
) {
    int n  = (int)order.size();
    int sq = int(ceil(sqrt(n)));
    // partition into √n groups
    vector<vector<int>> a_P(sq+1, vector<int>(a.size()));
    vector<vector<int>> id(sq+1);

    for (int i = 0; i < n; ++i) {
        ++count_O1;
        if (a[w[order[i]]] == 0) { continue; }
        a_P[i/sq][w[order[i]]] = 1;
        id[i/sq].push_back(order[i]);
    }

    vector<vector<int>> groups(sq+1);
    vector<int> visited(n + b.size() - 1);

    // one convolution per group: each counted as O(N·√N)
    for (int g = 0; g <= sq; ++g) {
        auto c_g = countedBoolCnv(a_P[g], b);
        for (int i = 0; i < (int)c_g.size(); ++i) {
            ++count_O1;
            if (c_g[i] && !visited[i]) {
                groups[g].push_back(i);
                visited[i] = 1;
            }
        }
    }

    vector<int> minW(n + b.size() - 1, -1);
    vector<int> inv(n);
    for (int i = 0; i < n; ++i) {
        ++count_O1;
        inv[order[i]] = i;
    }

    // scan each group (all O(1) work in inner loops)
    for (int g = 0; g <= sq; ++g) {
        for (int coin : id[g]) {
            int wi = w[coin];
            for (int result : groups[g]) {
                ++count_O1;
                int j = result - wi;
                if (0 <= j && j < (int)b.size() && b[j]) {
                    int pos = inv[coin];
                    if (minW[result] < 0 || pos < minW[result]) {
                        minW[result] = pos;
                    }
                }
            }
        }
    }

    return minW;
}

void kernelComputation_coinchange_simple(
    int n,
    int u,
    const vector<int>& w,
    const vector<int>& p,
    vector<int>& order,
    int /*t*/,
    vector<solution>& sol
) {
    int k  = int(floor(2.0 * log2(u) + 1.0));
    int KU = k*u + 1;
    ++count_O1; // computing k, KU

    sol.assign(max(KU, 1), solution());
    ++count_O1; // assign

    // initial reachable vector
    vector<int> v(KU), f(u+1);
    v[0]=1; f[0]=1;
    count_O1 += KU + u + 2; // initializing v, f, and setting v[0] = f[0] = 1

    for (int i = 1; i <= n; ++i) {
        ++count_O1; // loop
        f[w[order[i]]] = 1;
    }

    vector<int> inverseOrder(n+1);
    for (int i = 1; i <= n; ++i) {
        ++count_O1;
        inverseOrder[order[i]] = i;
    }

    vector<int> vPrime;
    for (int iter = 1; iter <= k; ++iter) {
        ++count_O1; // outer loop

        // 1) boolean convolution: O(N·√N)
        vPrime = countedBoolCnv(v, f);
        // 2) min‐witness also calls countedBoolCnv O(√N) times => each counted as O(N·√N)
        vector<int> minW = minimum_witness_boolCnv_ordered(f, v, w, order);

        // 3) reconstruct: a KU‐length scan of O(1) per entry
        for (int c = 1; c < KU; ++c) {
            ++count_O1;
            if (vPrime[c] && !v[c]) {
                v[c] = 1;
                int wi       = minW[c];
                int coinIdx  = inverseOrder[wi];
                int prev     = c - w[coinIdx];
                if (prev >= 0) {
                    count_O1 += (ll)log2(c); // copy and addCoin
                    sol[prev].copy(sol[c]);
                }
                sol[c].addCoin(wi, w[coinIdx], -1);
                count_O1 += (ll)log2(max((ll)log2(c), (ll)1));
            }
        }
    }
}



int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, u, T;
    if (!(cin >> n >> u >> T)) return 0;

    vector<int> w(n+1), p(n+1), order(n+1);
    for (int i = 1; i <= n; ++i) {
        cin >> w[i] >> p[i];
        order[i] = i;
    }

    vector<solution> sol;
    auto t0 = chrono::high_resolution_clock::now();
    kernelComputation_coinchange_simple(n, u, w, p, order, T, sol);
    auto t1 = chrono::high_resolution_clock::now();

    double kt = chrono::duration<double>(t1 - t0).count();

    cerr << "Kernel time:            " << kt       << " s\n";
    cerr << "Count of O(1) blocks:   " << count_O1       << "\n";
    cerr << "Count of O(N·√N) blocks:" << count_On_Conv << "\n";

    // estimate total ignoring constants:
    double N = double(u);
    double est_heavy = count_On_Conv * (N * sqrt(N) * log2(N));
    double est_simple= count_O1;
    cerr << "Estimated total ops (ignore consts): "
         << (est_simple + est_heavy) << "\n";

    return 0;
}