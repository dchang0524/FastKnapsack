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
    vector<solution>& sol, //sol[c] <- properties related to solution for the weight sum = c
    const vector<int>& order //the lexicographical order of the coins
) {
    for (int j = 1; j <= t; j++) {
        if (sol[j].size == 0) continue;
        for (auto const C : sol[j].svec) { //use the svec instead of supp
            int x = C.first;
            int nxt = j + w[order[x]];
            if (nxt > t) continue;
            sol[j].svec[x]++;
            sol[j].value += p[order[x]];
            sol[j].weight += w[order[x]];
            if (sol[nxt].size == 0
                || sol[j].value > sol[nxt].value
                || (sol[j].value == sol[nxt].value && sol[j].lexCmp(sol[nxt]))) //lexCmp should take (log n)^2 time
            {
                sol[j].copy(sol[nxt]);
            }
            sol[j].svec[x]--;
            sol[j].value -= p[order[x]];
            sol[j].weight -= w[order[x]];
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
    int KU = k * u + 1;

    // prepare sol[0..KU-1]
    sol.assign(max(KU, t + 1), solution());

    // v[c] = best profit for capacity c so far
    vector<ll> v(KU, NEG_INF), f(u+1, NEG_INF);
    v[0] = 0;
    for (int i = 1; i <= n; i++) {
        f[w[order[i]]] = p[order[i]];
    }

    //For min-witness tracking
    vector<ll> f_w(u+1, NEG_INF);
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

    vector<ll> vPrime, v_w(KU, NEG_INF), minW;

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

void kernelComputation_coinchange_simple(
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

    // v[c] = can reach capacity c so far
    vector<int> v(KU), f(u+1);
    v[0] = 1;
    f[0] = 1;
    for (int i = 1; i <= n; i++) {
        f[w[order[i]]] = 1;
    }

    vector<int> inverseOrder(n+1);
    for (int i = 1; i <= n; i++) {
        inverseOrder[order[i]] = i;
    }

    vector<int> vPrime;
    for (int iter = 1; iter <= k; iter++) { //compute iter-kernel
        // 1) boolean convolve to get new reachable capacities
        vPrime = boolCnv(v, f);

        // 2) Find minimum witness for each reachable capacity
        vector<int> minW = minimum_witness_boolCnv_ordered(f, v, w, order);
        // 3) reconstruct each kernel solution
        for (int c = 1; c < KU; c++) {
            if (vPrime[c] == 1 && v[c] == 0) {
                // cerr << "first time reaching capacity " << c << " at " << iter << endl;
                // accept new reachable capacity
                v[c] = vPrime[c];

                // find the minimum witness 
                int witnessI = minW[c];
                int coinIdx = inverseOrder[witnessI];
                int prev    = c - w[coinIdx];
                if (prev >= 0) {
                    sol[prev].copy(sol[c]);
                }
                sol[c].addCoin(witnessI, w[coinIdx], -1); //note in coinchange, the profit array is just -1
            }
        }
    }
}

void kernelComputation_coinchange_randomized(
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

    // v[c] = can reach capacity c so far
    vector<int> v(KU), f(u+1);
    v[0] = 1;
    f[0] = 1;
    for (int i = 1; i <= n; i++) {
        f[w[order[i]]] = 1;
    }

    vector<int> inverseOrder(n+1);
    for (int i = 1; i <= n; i++) {
        inverseOrder[order[i]] = i;
    }

    vector<int> vPrime;
    random_shuffle(order.begin() + 1, order.end());
    for (int iter = 1; iter <= k; iter++) { //compute iter-kernel
        // 1) boolean convolve to get new reachable capacities
        vPrime = boolCnv(v, f);

        // 2) Find minimum witness for each reachable capacity
        //randomize order
        vector<int> minW = minimum_witness_random(f, v, w, order);
        // 3) reconstruct each kernel solution
        for (int c = 1; c < KU; c++) {
            if (vPrime[c] == 1 && v[c] == 0) {
                // cerr << "first time reaching capacity " << c << " at " << iter << endl;
                // accept new reachable capacity
                v[c] = vPrime[c];

                // find the minimum witness 
                int witnessI = minW[c];
                int coinIdx = inverseOrder[witnessI];
                int prev    = c - w[coinIdx];
                if (prev >= 0) {
                    sol[prev].copy(sol[c]);
                }
                sol[c].addCoin(witnessI, w[coinIdx], -1); //note in coinchange, the profit array is just -1
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
    vector<vector<int>> b(k, vector<int>(u+1)); //Make the optimization b is always f if needed
    vector<vector<int>> c(k, vector<int>(KU + u));
    c[0] = v;
    for (int iter = 1; iter <= k; iter++) { //compute iter-kernel
        vPrime = boolCnv(v, f);
        for (int i = 1; i <= n; i++) {
            if (vPrime[i]) {
                if (sol[i].value == 0) {
                    sol[i].value = -iter;
                } else {
                    sol[i].value = max((int)sol[i].value, -iter);
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
    //adaptiveMinWitness(a, b, c, order);
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
    vector<vector<int>>& a,
    vector<vector<int>>& b,
    vector<vector<int>>& c,
    vector<int>& w,
    vector<int>& order
) {
    //assuming c also contains zero elements bigger than the index of the last nonzero element
    int k = 2 * static_cast<int>(ceil(log2(c.size()) + log2(c[0].size()))); //2*log(pt)
    vector<int> newOrder;
    int size = order.size();
    unordered_set<int> orderSet;
    for (int i = 1; i <= order.size(); i++) {
        orderSet.insert(i);
    }
    while (size > 1) { //size will be <= size/2 after every iteration
        vector<vector<vector<int>>> witnesses(c.size(), vector<vector<int>>(c[0].size())); // witnesses[i][j] = witness of a[i][j]
        for (int i = 0; i < c.size(); i++) {
            //k-reconstruction
            auto witnesses = randomized_k_witness(a[i], b[i], k, w, order);
            //for entries with < k witnesses, store their witnesses in witnesses[i][j]
            //for entries with k witnesses, store its witnesses somewhere to compute their hitting set
        }
        //compute hitting set of the stored sets
        //find elements in (1,...,n) that aren't in the hitting set, order them in any way in sigma[|hitting set| + 1:]
        //remove coins not in the hitting set from a and order
        //recompute c
    }
    //if witnesses[i][j].size() > 1, manually compute its minimum witness using sigma
}

vector<vector<int>> adaptiveMinWitness_randomized(
    vector<vector<int>>& a,
    vector<vector<int>>& b,
    vector<vector<int>>& c,
    vector<int>& w,
    vector<int>& order
) {
    int p = a.size();
    if (p == 0) return {};
    int n = order.size();
    int r = (int)c[0].size();      // = 2*n - 1

    // fixed k = 2·⌈log₂(p) + log₂(r)⌉
    int k = 2 * static_cast<int>(ceil(log2(p) + log2(r)));

    // output array, initialized to -1
    vector<vector<int>> res(p, vector<int>(r, -1));

    vector<int> newOrder;
    int size = n;

    // temp to tie-break by final order
    vector<int> posMap(n);

    // scratch for storing <k witnesses
    vector<vector<vector<int>>> witnesses(p, vector<vector<int>>(r));

    while (size > 1) {
      // 1) collect witnesses for each row i
      vector<vector<int>> bigSets;
      bigSets.reserve(p * r);

      for (int i = 0; i < p; ++i) {
        auto rec = randomized_k_witness(a[i], b[i], k, w, order);
        for (int j = 0; j < r; ++j) {
          if (rec[j].size() < (size_t)k) {
            witnesses[i][j] = move(rec[j]);
          } else {
            bigSets.push_back(move(rec[j]));
          }
        }
      }

      // 2) if no “big” sets remain, we can finalize everything now
      if (bigSets.empty()) {
        // build position map for final tie-break
        for (int idx = 0; idx < size; ++idx)
          posMap[ order[idx] ] = idx;

        // tie-break each multi-candidate entry
        for (int i = 0; i < p; ++i) {
          for (int j = 0; j < r; ++j) {
            if (c[i][j] > 0 && witnesses[i][j].size() > 1) {
              int best = witnesses[i][j][0];
              for (int x : witnesses[i][j])
                if (posMap[x] < posMap[best])
                  best = x;
              res[i][j] = best;
            }
            // also fill singleton cases:
            else if (c[i][j] > 0 && witnesses[i][j].size() == 1) {
              res[i][j] = witnesses[i][j][0];
            }
          }
        }
        return res;
      }

      // 3) compute hitting set H of all bigSets
      auto H = computeHittingSet(bigSets, (int)bigSets.size(), k, size);

      // 4) rebuild `order` so that H is the new prefix
      vector<bool> inH(n,false);
      for (int x : H) inH[x] = true;
      newOrder.clear();
      newOrder.reserve(H.size());
      for (int x : order)
        if (inH[x])
          newOrder.push_back(x);
      size = (int)newOrder.size();    // now ≤ old size/2
      order.swap(newOrder);

      // 5) restrict a,b to the new prefix of length `size` and recompute c
      r = 2*size - 1;
      for (int i = 0; i < p; ++i) {
        vector<int> ai(size), bi(size);
        for (int t = 0; t < size; ++t) {
          ai[t] = a[i][ order[t] ];
          bi[t] = b[i][ order[t] ];
        }
        auto ci = convolution(ai, bi);
        c[i].swap(ci);
      }
      // resize our witness‐scratch to match new r
      witnesses.assign(p, vector<vector<int>>(r));
    }

    // If we exit because size<=1, then each remaining c[i][j]>0 has at most one coin
    // in scope, so that singleton (or -1) is the answer:
    for (int i = 0; i < p; ++i) {
      for (int j = 0; j < r; ++j) {
        if (c[i][j] > 0 && witnesses[i][j].size() == 1) {
          res[i][j] = witnesses[i][j][0];
        }
      }
    }
    return res;
}

