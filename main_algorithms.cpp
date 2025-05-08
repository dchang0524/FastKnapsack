#include <bits/stdc++.h>
using namespace std;

#define pb push_back
#define mp make_pair

typedef long long ll;
typedef pair<int, int> pii;
typedef pair<ll, ll> pll;

const int NEG_INF = -1000000000; // Negative infinity placeholder

int n; //number of coins
int u; //maximum weight of a coin
vector<int> w(n+1); //weights of coins. +1 to size since coins are 1-indexed
vector<int> p(n+1); //profits(value) of each coin
vector<int> order(n+1); //lexicographical order of the coins

int t; //maximum target weight
struct solution { //class for representing a solution
    int size;
    int value;
    int weight;
    vector<int> svec; //vector for representing the actual solution
    // Constructor to initialize the struct with a given size
    solution(int n) : size(0), value(0), weight(0), svec(n+1) {}
};
vector<solution> sol(t + 1, solution(n)); //sol[i] <- solution to sum = i under order and maximizing profit.
vector<unordered_set<int>> supp; //supp[i] <- support(sol[i])

//For Lemma 5.7
vector<int> computeHittingSet(const vector<vector<int>>& sets, int R, int n) {
    int u = sets.size();
    // 1) Build inverted index: for each x in [0..n), inv[x] = list of i with x ∈ sets[i]
    vector<vector<int>> inv(n);
    inv.reserve(n);
    for (int i = 0; i < u; i++) {
        for (int x : sets[i]) {
            inv[x].push_back(i);
        }
    }

    vector<int> freq(n, 0);
    for (int x = 0; x < n; x++) {
        freq[x] = inv[x].size();
    }

    vector<bool> covered(u, false);
    int remaining = u;
    vector<int> H;
    H.reserve((int)ceil((double)n / R * log(u + 1)));

    while (remaining > 0) {
        int best = max_element(freq.begin(), freq.end()) - freq.begin();
        if (freq[best] == 0) {
            break;
        }
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


//Naive Boolean‐count convolution, replace with FFT
vector<int> boolCountConv(const vector<int>& a, const vector<int>& b) {
    int n = a.size(), m = b.size(), N = n + m - 1;
    vector<int> c(N, 0);
    for (int w = 0; w < n; w++) {
        if (!a[w]) continue;
        for (int x = 0; x < m; x++) {
            if (b[x]) c[w + x]++;
        }
    }
    return c;
}

//Naive integer‐sum convolution, FFT
vector<long long> intSumConv(const vector<long long>& aSum, const vector<int>& b) {
    int n = aSum.size(), m = b.size(), N = n + m - 1;
    vector<long long> s(N, 0);
    for (int w = 0; w < n; w++) {
        if (aSum[w] == 0) continue;
        for (int x = 0; x < m; x++) {
            if (b[x]) s[w + x] += aSum[w];
        }
    }
    return s;
}


//witness peeling algorithm for Lemma 5.6
vector<vector<int>> peelWitnesses(
    const vector<vector<int>>& F,
    vector<vector<int>>& z,
    vector<vector<int>>& mA,
    int n,
    int k
) {
    int R = F.size(), N = z[0].size();
    vector<vector<int>> appearsIn(n);
    for (int A = 0; A < R; A++)
        for (int w : F[A])
            appearsIn[w].push_back(A);

    vector<vector<int>> W(N);
    vector<queue<int>> Q(N);
    for (int i = 0; i < N; i++)
        for (int A = 0; A < R; A++)
            if (z[A][i] == 1)
                Q[i].push(A);

    bool peeled = true;
    while (peeled) {
        peeled = false;
        for (int i = 0; i < N; i++) {
            if ((int)W[i].size() < k && !Q[i].empty()) {
                int A = Q[i].front(); Q[i].pop();
                int w = mA[A][i];
                W[i].push_back(w);
                for (int Ap : appearsIn[w]) {
                    if (z[Ap][i] > 0) {
                        z[Ap][i]--;
                        mA[Ap][i] -= w;
                        if (z[Ap][i] == 1 && (int)W[i].size() < k)
                            Q[i].push(Ap);
                    }
                }
                peeled = true;
            }
        }
    }
    return W;
}

//k-reconstruction for lemma 5.6
vector<vector<int>> k_reconstruction(
    const vector<int>& a, 
    const vector<int>& b, 
    const vector<vector<int>>& F, 
    int k
) {
    int n = a.size(), m = b.size(), N = n + m - 1;
    int R = F.size();
    vector<vector<int>>  z(R, vector<int>(N, 0));
    vector<vector<int>> mA(R, vector<int>(N, 0));

    for (int A = 0; A < R; A++) {
        vector<int> aPrime(n, 0);
        for (int w : F[A])
            aPrime[w] = a[w];

        z[A] = boolCountConv(aPrime, b);

        vector<long long> aSum(n, 0);
        for (int w : F[A]) 
            if (a[w]) 
                aSum[w] = w;

        auto sums = intSumConv(aSum, b);
        for (int i = 0; i < N; i++)
            mA[A][i] = (int)sums[i];
    }

    return peelWitnesses(F, z, mA, n, k);
}


//Compares the lexicorgraphical order of s1 and s2. 1 if s1 < s2, 0 otherwise
int lexCmp(struct solution s1, struct solution s2) {
    for (int i = 0; i < s1.svec.size(); i++) {
        if (s1.svec[i] > s2.svec[i]) {
            return 0;
        }
    }
    return 1;
}

// (max, +) convolution
vector<int> maxPlusCnv(const vector<int>& a, const vector<int>& b) {
    int n = a.size(), m = b.size();
    // result length = n + m - 1
    vector<int> c(n + m - 1, INT_MIN);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            int idx = i + j;
            c[idx] = max(c[idx], a[i] + b[j]);
        }
    }
    return c;
}

// boolean convolution, swap with FFT later
vector<int> boolCnv(const vector<int>& a, const vector<int>& b) {
    int n = a.size(), m = b.size();
    vector<int> c(n + m - 1, 0);
    for (int i = 0; i < n; ++i) {
        if (!a[i]) continue;
        for (int j = 0; j < m; ++j) {
            if (b[j]) {
                c[i + j] = 1;
            }
        }
    }
    return c;
}

//Algorithm 1: Witness Propagation.
//Computes solutions assuming we know the supp(sol[j]), the kernel of a solution to a subset sum
void propagation() {
    for (int j = 1; j <= t; j++) {
        struct solution *curr = &sol[j];
        if (sol[j].size != 0) { //checks if sol[j] isn't empty
            for (int x : supp[j]) {
                if (j + w[x] > t) {
                    continue;
                }
                curr->svec[x]++;
                curr->value += p[x];
                curr->weight += w[x];
                struct solution *next = &sol[j+w[x]];
                if (next->size == 0 || curr->value > next->value
                        || (curr->value == next->value && lexCmp(*curr, *next))) {
                    *next = *curr;
                }
                curr->svec[x]--;
                curr->value -= p[x];
                curr->weight -= w[x];
            }
        }
    }
}

//Algorithm 2: Kernel Computation with Minimum Witness
//For All-Target Unbounded Knapsack
    //For All-Target CoinChange and Residue Table, we must 
        //change v and f to boolean arrays
        //Replace (min, +) convolution to boolean convolution
        //Compute minimum witnesses during the boolean convolution using Algorithm 3 or Algortihm 4
void kernelComputation() {
    int k = static_cast<int>(floor(2.0 * log2(u) + 1.0));
    vector<int> v(k*u + 1, NEG_INF), f(u + 1, NEG_INF);
    sol = vector<struct solution>(k*u + 1, solution(n));
    v[0] = 0;
    for (int i = 1; i <= n; i++) {
        f[w[order[i]]] = p[order[i]];
    }

    /////////For computing mininum witnesses
    vector<int> v_w(v.size(), NEG_INF); 
    vector<int> f_w(f.size(), NEG_INF);
    for (int i = 1; i <= n; i++) {
        f_w[w[order[i]]] = (n + 1) * f[w[order[i]]] - i;
    }
    ///////////

    for (int j = 1; j <= k; j++) {
        vector<int> vPrime = maxPlusCnv(v, f);
        vector<int> minWit;
        for (int i = 0; i < v_w.size(); i++) {
            if (v[i] != NEG_INF) {
                v_w[i] = (n+1) * v[i];
            }
        }  
        minWit = maxPlusCnv(v_w, f_w);
        for (int i = 1; i <= k*u; i++) {
            if (vPrime[i] != NEG_INF) {
                v[i] = vPrime[i];
                int c = order[minWit[i] % (n+1)];
                if (i - w[c] <= 0) {
                    continue;
                }
                sol[i] = sol[i - w[c]];
                sol[i].svec[c]++;
                sol[i].weight += w[c];
                sol[i].value += p[c];
                sol[i].size++;
            }
        }

    }
}

//Algorithm 3: Minimum Witness 
//Finding Under Random Order, not needed as we have a deterministic algorithm for  finding minimum witnesses using adaptive order (algorithm 4)
// vector<int> mininimumWitnessFinding(vector<int> order, vector<int> a, vector<int> b) {
//     int n = a.size(), m = b.size();
//     unordered_map<int, int> orderMap;
//     for (int i = 1; i <= n; i++) {
//         orderMap[i] = order[i];
//     }
//     vector<set<pii>> x(n+m-1); //x[i] <- {(order[k], k) | a[k] = b[i-k] = 1}
//     vector<int> d(n+m-1); //d[i] <- smallest lexicographical element in x[i]

//     for (int l = 1; l < 2*n; l *= 2) {
//         vector<int> aPrime(n);
//         for (int i = 0; i < l; i++) {
//             aPrime[order[i]] = a[order[i]];
//         }
//         vector<int> cPrime = boolCnv(aPrime, b);

//         bool updated = true;
//         while (updated) {
//             updated = false;
//             ///////
//             //Uniformly sample an array of witnesses d′ from a′, b and c′
//             vector<int> dPrime(n + m - 1);
//             for (int i = 0; i <= n+m-2; i++) {
                
//             }
//             ////////////////
//             for (int j = 0; j <= n + m - 2; j++) {
//                 if (cPrime[j] > 0 && d[j] == -1) {
//                     x[j].insert(mp(orderMap[dPrime[j]], dPrime[j]));
//                     if (x[j].size() == cPrime[j]) {
//                         d[j] = x[j].begin()->second;
//                     }
//                 }
//             }

//         }
//     }

// }

//Algorithm 4: Adaptive Minimum Witness
//Find minimum witnesses deterministically, only works for boolean convolutions
void adaptiveMinWitness(
    const vector<vector<int>>& A,
    const vector<vector<int>>& B,
    vector<vector<int>>& c,
    vector<int>& sigma
) {
    int pCount = A.size(), n = sigma.size(),
        m = B[0].size(), N = n + m - 1;

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
        int t = max(1,(int)L.size()),
            k = 2*((int)ceil(log2(t)))+5;

        //k-witness
        for (auto [p,q] : L) {
            if (!done[p]) {
                // mask A[p]
                vector<int> aMask(n,0);
                for (int i = 0; i < prefix; i++)
                    aMask[sigma[i]] = A[p][sigma[i]];
                // family = single subset = σ[0..prefix-1]
                vector<vector<int>> F(1);
                F[0].assign(sigma.begin(), sigma.begin()+prefix);
                peelCache[p] = k_reconstruction(aMask, B[p], F, k);
                done[p] = true;
            }
        }
        //hitting sets
        vector<vector<int>> hitSets;
        hitSets.reserve(L.size());
        vector<pii> survivors;
        survivors.reserve(L.size());

        for (auto [p,q] : L) {
            auto &W = peelCache[p][q];
            if ((int)W.size() >= k) {
                hitSets.push_back(W);
            } else {
                survivors.emplace_back(p,q);
            }
        }

        if (!hitSets.empty()) {
            auto H = computeHittingSet(hitSets, k, n);
            // 4) reorder sigma[0..block-1] so H comes first
            vector<int> front, back;
            front.reserve(H.size());
            back.reserve(block - H.size());
            unordered_set<int> inH(H.begin(), H.end());
            for (int i = 0; i < block; i++) {
                if (inH.count(sigma[i])) front.push_back(sigma[i]);
                else back.push_back(sigma[i]);
            }
            // rebuild prefix
            for (int i = 0; i < (int)front.size(); i++) sigma[i] = front[i];
            for (int i = 0; i < (int)back.size(); i++)  sigma[(int)front.size()+i] = back[i];
        }

        vector<pair<int,int>> newL;
        newL.reserve(survivors.size());
        for (auto [p,q] : survivors) {
            auto &W = peelCache[p][q];
            bool allSuf = !W.empty();
            for (int w : W) {
                int r = pos[w];
                if (!(r >= prefix && r < block)) { allSuf = false; break; }
            }
            if (allSuf && !W.empty()) {
                int best = W[0];
                for (int w : W)
                    if (pos[w] < pos[best]) best = w;
                c[p][q] = best;
            } else {
                newL.emplace_back(p,q);
            }
        }
        L.swap(newL);
        if (L.empty()) break;
    }
}
