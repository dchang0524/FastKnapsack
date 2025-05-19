#include "algorithms.h"

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
    vector<solution>& sol //sol[c] <- properties related to solution for the weight sum = c
) {
    for (int j = 1; j <= t; j++) {
        if (sol[j].size == 0) continue;
        for (auto const C : sol[j].svec) { //use the svec instead of supp
            int x = C.first;
            int nxt = j + w[x];
            if (nxt > t) continue;
            sol[j].svec[x]++;
            sol[j].value += p[x];
            sol[j].weight += w[x];
            if (sol[nxt].size == 0
                || sol[j].value > sol[nxt].value
                || (sol[j].value == sol[nxt].value && sol[j].lexCmp(sol[nxt]))) //lexCmp should take (log n)^2 time
            {
                sol[j].copy(sol[nxt]);
            }
            sol[j].svec[x]--;
            sol[j].value -= p[x];
            sol[j].weight -= w[x];
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
void kernelComputation_knapsack(
    int n,                              // number of coins
    int u,                              // maximum coin weight
    const vector<int>& w,               // weights of the coins (1-indexed)
    const vector<int>& p,               // profits of the coins  (1-indexed)
    const vector<int>& order,           // lexicographical order σ[1..n]
    int t,                          // (unused) global target bound
    vector<solution>& sol              // output: sol[c] for c∈[0..k·u]
) {
    int k  = static_cast<int>(floor(2.0 * log2(u) + 1.0));
    // cout << "kernel size " << k << endl; 
    int KU = k * u + 1;

    // prepare sol[0..KU-1]
    sol.assign(max(KU, t + 1), solution());

    // v[c] = best profit for capacity c so far
    vector<int> v(KU, NEG_INF), f(u+1, NEG_INF);
    v[0] = 0;
    for (int i = 1; i <= n; i++) {
        f[w[order[i]]] = p[order[i]];
    }
    // cout << "v, f initialized" << endl;

    //For min-witness tracking
    vector<int> f_w(u+1, NEG_INF);
    for (int i = 1; i <= n; i++) {
        // store (n+1)*f[w] - i so that in the convolution
        // max_plus(v_w, f_w)[c] ≡ (n+1)*v'[c] - i
        f_w[w[order[i]]] = (n + 1) * f[w[order[i]]] - i;
    }
    //compute the inverse of order so that we can use the order to find the minimum witness
    vector<int> inverseOrder(n+1);
    for (int i = 1; i <= n; i++) {
        inverseOrder[order[i]] = i;
    }

    vector<int> vPrime, v_w(KU, NEG_INF), minW;

    for (int iter = 1; iter <= k; iter++) { //compute iter-kernel
        // 1) (max,+) convolve to get new profits
        vPrime = maxPlusCnv(v, f); //has size = KU + u - 1

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
            if (vPrime[c] > NEG_INF) {
                // accept new profit
                v[c] = vPrime[c];

                // recover the “-i” index from minW[c], then flip it to +i
                int negI      = minW[c] % (n+1);
                int witnessI  = ((n+1) - negI) % (n+1);
                if (witnessI == 0) continue;           // no valid coin

                int coinIdx = inverseOrder[witnessI];
                int prev    = c - w[coinIdx];
                if (prev >= 0) {
                    sol[prev].copy(sol[c]);
                }
                sol[c].addCoin(coinIdx, w[coinIdx], p[coinIdx]);
            }
        }
    }
}

void kernelComputation_coinchange(
    int n,                              // number of coins
    int u,                              // maximum coin weight
    const vector<int>& w,               // weights of the coins (1-indexed)
    const vector<int>& p,               // profits of the coins  (1-indexed)
    vector<int>& order,           // lexicographical order σ[1..n]
    int t,                          // (unused) global target bound
    vector<solution>& sol              // output: sol[c] for c∈[0..k·u]
) {
    int k  = static_cast<int>(floor(2.0 * log2(u) + 1.0));
    // cout << "kernel size " << k << endl; 
    int KU = k * u + 1;

    // prepare sol[0..KU-1]
    sol.assign(max(KU, t + 1), solution());

    // v[c] = best profit for capacity c so far
    vector<int> v(KU, NEG_INF), f(u+1, NEG_INF);
    v[0] = 0;
    for (int i = 1; i <= n; i++) {
        f[w[order[i]]] = p[order[i]];
    }
    // cout << "v, f initialized" << endl;

    
    vector<int> vPrime;
    vector<vector<int>> a(k, vector<int>(KU));
    vector<vector<int>> b(k, vector<int>(u+1)); //Make the optimkzation b is always f if needed
    vector<vector<int>> c(k, vector<int>(KU + u));
    c[0] = v;
    for (int iter = 1; iter <= k; iter++) { //compute iter-kernel
        vPrime = boolCnv(v, f);
        for (int i = 1; i <= n; i++) {
            if (vPrime[i]) {
                if (sol[i].value == 0) {
                    sol[i].value = -iter;
                } else {
                    sol[i].value = max(sol[i].value, -iter);
                }
            }
        }
        a[iter] = v;
        b[iter] = f;
        c[iter] = vPrime;
        for (int i = 0; i < v.size(); i++) {
            v[i] = vPrime[i];
        }
    }
    adaptiveMinWitness(a, b, c, order);
    //reorder arrays so that with respect to adaptive lex order so that (1, ..., n) is the lex order
    for (int iter = 1; iter <= k; iter++) {
        for (int i = 1; i <= KU; i++) {
            if (c[iter][i] && !c[iter-1][i]) {
                int minWit = i; // minimum witness placeholder
                if (i-w[minWit] >= 0) {
                    sol[i-w[minWit]].copy(sol[i]);
                } 
                sol[i].addCoin(minWit, w[minWit], p[minWit]); //make sure this is also reordered so that (1,...,n) is the lex order
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
