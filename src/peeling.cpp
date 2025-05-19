#include "peeling.h"

vector<vector<int>> peelWitnesses(
    vector<vector<int>>& F,
    vector<vector<int>>& z,
    vector<vector<int>>& mA,
    int universeSize,
    int k
) {
    int R = (int)F.size();
    int N = (int)z[0].size();
    // build inverted‐index: which subsets each w belongs to
    vector<vector<int>> inv(universeSize);
    for (int A = 0; A < R; A++) {
        for (int w : F[A]) {
            inv[w].push_back(A);
        }
    }
    vector<vector<int>> W(N);
    vector<queue<int>> Q(N);
    for (int i = 0; i < N; i++) {
        for (int A = 0; A < R; A++) {
            if (z[A][i] == 1) {
                Q[i].push(A);
            }
        }
    }
    bool again = true;
    while (again) {
        again = false;
        for (int i = 0; i < N; i++) {
            if ((int)W[i].size() < k && !Q[i].empty()) {
                int A = Q[i].front(); Q[i].pop();
                int w = mA[A][i];
                W[i].push_back(w);
                for (int Ap : inv[w]) {
                    if (z[Ap][i] > 0) {
                        z[Ap][i]--;
                        mA[Ap][i] -= w;
                        if (z[Ap][i] == 1 && (int)W[i].size() < k) {
                            Q[i].push(Ap);
                        }
                    }
                }
                again = true;
            }
        }
    }
    return W;
}

vector<vector<int>> k_reconstruction(
    vector<int>& a,
    vector<int>& b,
    vector<vector<int>>& F,
    int k
) {
    int n = (int)a.size();
    int m = (int)b.size();
    int N = n + m - 1;
    int R = (int)F.size();
    // convolution tables
    vector<vector<int>> z(R, vector<int>(N, 0));
    vector<vector<int>> mA(R, vector<int>(N, 0));
    // fill them via two naive convs
    for (int A = 0; A < R; A++) {
        // mask a
        vector<int> aPrime(n, 0);
        for (int w : F[A]) aPrime[w] = a[w];
        z[A] = boolCountConv(aPrime, b);
        // mask a→aSum
        vector<long long> aSum(n, 0);
        for (int w : F[A]) if (a[w]) aSum[w] = w;
        auto sums = intSumConv(aSum, b);
        for (int i = 0; i < N; i++) mA[A][i] = (int)sums[i];
    }
    return peelWitnesses(F, z, mA, n, k);
}
