#include "algorithms.h"
#include "convolution.h"
#include "dp_structs.h"
#include "peeling.h"
#include "hitting_set.h"
#include <cmath>
#include <algorithm>
#include <queue>
#include <unordered_set>
using namespace std;

typedef long long ll;

// Algorithm 1: Witness Propagation
/**
 * Standard Knapsack-DP given some set of solutions (called "kernels")
 * Gurantees optimal solution due to Optimal Substructure Combinatorial Property
 * For All-Target Unbounded Knapsack and All-Target Coinchange
 */
void propagation(
    const vector<int>& w, //weight of each coin
    const vector<int>& p, //the profit of each coin
    int t, //the bound for the range we want to compute solutions for
    vector<solution>& sol, //sol[c] <- properties related to solution for the weight sum = c
    const vector<vector<int>>& supp //supp[c] <- the coins involved in sol[c]
) {
    for (int j = 1; j <= t; j++) {
        if (sol[j].size == 0) continue;
        solution tmp = sol[j];
        for (int x : supp[j]) {
            if (j + w[x] > t) continue;
            tmp.svec[x]++;
            tmp.value += p[x];
            tmp.weight += w[x];
            solution& nxt = sol[j + w[x]];
            if (nxt.size == 0
                || tmp.value > nxt.value
                || (tmp.value == nxt.value && lexCmp(tmp, nxt))) //lexCmp should take long n time once we replace svec with a map
            {
                nxt = tmp; //when we replace svec to maps, the copying should take log n time since supports are logarithmically sized
            }
            tmp.svec[x]--;
            tmp.value -= p[x];
            tmp.weight -= w[x];
        }
    }
}

// Algorithm 2: Kernel Computation
/**
 * Computes the x-kernels for x in 1, ..., 2*logu + 1
 * A k-kernel is the set of solutions total of k (not necessarily distinct) coins
 * This specific implementation is only for All-Target Unbounded Knapsack
 * For All-Target CoinChange and Residue Table replace (max, +) convolutions with Boolean convolution, and use algorithm 4 for finding minimum witnesses
 * For CoinChange, the value of a solution is just the number of convolutions iterated
 * For Residue Table, the value of a solution is the sum itself
 */
void kernelComputation(
    int n,                              // number of coins
    int u,                              // maximum coin weight
    const vector<int>& w,               // weights of the coins (1-indexed)
    const vector<int>& p,               // profits of the coins  (1-indexed)
    const vector<int>& order,           // lexicographical order σ[1..n]
    int /*t*/,                          // (unused) global target bound
    vector<solution>& sol              // output: sol[c] for c∈[0..k·u]
) {
    int k  = static_cast<int>(floor(2.0 * log2(u) + 1.0));
    int KU = k * u + 1;

    // prepare sol[0..KU-1]
    sol.assign(KU, solution(n));

    // v[c] = best profit for capacity c so far
    vector<int> v(KU, NEG_INF), f(u+1, NEG_INF);
    v[0] = 0;
    for (int i = 1; i <= n; i++) {
        f[w[order[i]]] = p[order[i]];
    }

    // prep for min-witness tracking
    vector<int> f_w(u+1, NEG_INF);
    for (int i = 1; i <= n; i++) {
        // store (n+1)*f[w] - i so that in the convolution
        // max_plus(v_w, f_w)[c] ≡ (n+1)*v'[c] - i
        f_w[w[order[i]]] = (n + 1) * f[w[order[i]]] - i;
    }

    vector<int> vPrime, v_w(KU, NEG_INF), minW;
    for (int iter = 1; iter <= k; iter++) {
        // 1) (max,+) convolve to get new profits
        vPrime = maxPlusCnv(v, f);

        // 2) build scaled v for witness tracking
        fill(v_w.begin(), v_w.end(), NEG_INF);
        for (int c = 0; c < KU; c++) {
            if (v[c] > NEG_INF) 
                v_w[c] = (n + 1) * v[c];
        }

        // 3) convolve to get encoded witnesses
        minW = maxPlusCnv(v_w, f_w);

        // 4) reconstruct each kernel solution
        for (int c = 1; c < KU; c++) {
            if (c < (int)vPrime.size() && vPrime[c] > NEG_INF) {
                // accept new profit
                v[c] = vPrime[c];

                // recover the “-i” index from minW[c], then flip it to +i
                int negI      = minW[c] % (n+1);
                int witnessI  = ((n+1) - negI) % (n+1);
                if (witnessI == 0) continue;           // no valid coin

                int coinIdx = order[witnessI];
                int prev    = c - w[coinIdx];
                if (prev >= 0) {
                    sol[c] = sol[prev];
                    sol[c].svec[coinIdx]++;
                    sol[c].weight += w[coinIdx];
                    sol[c].value  += p[coinIdx];
                    sol[c].size++;
                }
            }
        }
    }
}

// Algorithm 4: Adaptive Minimum Witness
/**
 * Computes the minimum witness of a solution
 */
void adaptiveMinWitness(
    const vector<vector<int>>& A,
    const vector<vector<int>>& B,
    vector<vector<int>>& c,
    vector<int>& sigma
) {
    int pCount = (int)A.size();
    int n      = (int)sigma.size();
    int m      = (int)B[0].size();
    int N      = n + m - 1;

    vector<pair<int,int>> L;
    for (int p = 0; p < pCount; p++)
      for (int q = 0; q < N; q++)
        if (c[p][q] > 0) L.emplace_back(p,q);

    vector<int> pos(n);
    for (int i = 0; i < n; i++) pos[sigma[i]] = i;

    vector<bool> done(pCount,false);
    vector<vector<vector<int>>> peelCache(pCount);

    for (int block = n; block >= 1; block /= 2) {
        int prefix = block/2;
        int tsize  = max(1,(int)L.size());
        int k      = 2*((int)ceil(log2(tsize)))+5;

        // compute k‐witness once per p
        for (auto [p,q] : L) {
            if (!done[p]) {
                vector<int> aMask(n,0);
                for (int i = 0; i < prefix; i++)
                    aMask[sigma[i]] = A[p][sigma[i]];
                vector<vector<int>> F(1);
                F[0].assign(sigma.begin(), sigma.begin()+prefix);
                peelCache[p] = k_reconstruction(aMask, B[p], F, k);
                done[p] = true;
            }
        }
        // split into sets needing hitting‐set vs survivors
        vector<vector<int>> hitSets;
        vector<pair<int,int>> survivors;
        for (auto [p,q] : L) {
            auto &W = peelCache[p][q];
            if ((int)W.size() >= k) hitSets.push_back(W);
            else              survivors.emplace_back(p,q);
        }
        // hitting‐set & reorder sigma
        if (!hitSets.empty()) {
            auto H = computeHittingSet(hitSets, k, n);
            vector<int> front, back;
            front.reserve(H.size());
            back.reserve(block - H.size());
            unordered_set<int> inH(H.begin(), H.end());
            for (int i = 0; i < block; i++) {
                if (inH.count(sigma[i])) front.push_back(sigma[i]);
                else                      back.push_back(sigma[i]);
            }
            for (int i = 0; i < (int)front.size(); i++) sigma[i] = front[i];
            for (int i = 0; i < (int)back.size();  i++)
                sigma[(int)front.size() + i] = back[i];
        }
        // peel off solved slots
        vector<pair<int,int>> newL;
        for (auto [p,q] : survivors) {
            auto &W = peelCache[p][q];
            bool allSuf = !W.empty();
            for (int wgt : W) {
                int r = pos[wgt];
                if (!(r >= prefix && r < block)) { allSuf = false; break; }
            }
            if (allSuf && !W.empty()) {
                int best = W[0];
                for (int wgt : W)
                    if (pos[wgt] < pos[best]) best = wgt;
                c[p][q] = best;
            } else {
                newL.emplace_back(p,q);
            }
        }
        L.swap(newL);
        if (L.empty()) break;
    }
}
