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
    int n, //the number of coins
    int u, //the maximum weight of coins
    const vector<int>& w, //the weights of coins
    const vector<int>& p, //the profits of coins
    const vector<int>& order, //the lexicographical order
    int t, ///the bound for the range we want to compute solutions for 
    vector<solution>& sol //sol[c] <- properties related to solution for the weight sum = c
) {
    int k = (int)floor(2.0*log2(u) + 1.0);
    int KU = k * u + 1;
    sol.assign(KU, solution(n));
    vector<int> v(KU, NEG_INF), f(u+1, NEG_INF);
    v[0] = 0;
    for (int i = 1; i <= n; i++) {
        f[w[order[i]]] = p[order[i]];
    }

    //a convolution is used to update the optimal solutions
    //the minimum lexicorgraphic order is maintained by using the minimum witnesses for each solution
    vector<int> v_w, vPrime, minW;
    vector<ll> aSum;
    for (int iter = 1; iter <= k; iter++) {
        vPrime = maxPlusCnv(v, f);
        v_w.assign(v.size(), NEG_INF);
        for (size_t i = 0; i < v.size(); i++) {
            if (v[i] > NEG_INF) v_w[i] = (n+1)*v[i];
        }
        minW = maxPlusCnv(v_w, f);
        for (int i = 1; i < KU; i++) {
            if (i < (int)vPrime.size() && vPrime[i] > NEG_INF) {
                v[i] = vPrime[i];
                int cidx = order[minW[i]%(n+1)];
                int prev = i - w[cidx];
                if (prev >= 0) {
                    sol[i] = sol[prev];
                    sol[i].svec[cidx]++;
                    sol[i].weight += w[cidx];
                    sol[i].value  += p[cidx];
                    sol[i].size++;
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
