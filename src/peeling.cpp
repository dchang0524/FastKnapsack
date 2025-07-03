#include "convolution.h"
#include "peeling.h"

// Randomized k-aligned-ones reconstruction
// text: string of '0'/'1' length n
// pat:   pattern string '0'/'1' length m
// k:     number of ones to recover per alignment
// Returns vector of length (n-m+1), each a sorted list of recovered positions in [0,m).
vector<vector<int>> k_reconstruct_randomized(string &text,
                                  string &pat,
                                  int k) {
    int n = text.size();
    int m = pat.size();
    int L = n - m + 1;
    
    // convert to int arrays
    vector<int> a(n), p(m);
    for(int i = 0; i < n; ++i) a[i] = text[i] - '0';
    for(int j = 0; j < m; ++j) p[j] = pat[j] - '0';

    // full intersection sizes via convolution
    vector<int> full_size = convolution(a, p);

    // random engine
    mt19937_64 gen(random_device{}());
    const double alpha = 1.0/(2*exp(1));

    // Phase I: bounded peeler (threshold = 4k)
    int kp = 4*k;
    int logkp = (int)ceil(log2(double(kp)));
    double logrec = log(1.0/(1 - alpha));
    double ln_nk = log(double(n) * kp);

    // build F1
    vector<vector<bool>> F1;
    for(int j = 0; j <= logkp; ++j) {
        int rj = (int)ceil(((1ULL << (j+3)) * ln_nk) / logrec);
        double prob = 1.0 / (1u << j);
        uniform_real_distribution<double> dist(0.0,1.0);
        while(rj-- > 0) {
            vector<bool> subset(m,false);
            for(int idx=0; idx<m; ++idx)
                if(p[idx] && dist(gen) < prob)
                    subset[idx] = true;
            F1.push_back(move(subset));
        }
    }
    int F1sz = F1.size();

    // precompute ISIZE/ISUM for F1
    vector<vector<int>> size1(F1sz, vector<int>(L));
    vector<vector<int>> sum1(F1sz, vector<int>(L));
    for(int idx=0; idx<F1sz; ++idx) {
        vector<int> b(m), bsum(m);
        for(int j=0; j<m; ++j)
            if(F1[idx][j] && p[j]) { b[j]=1; bsum[j]=j; }
        auto szv = convolution(a,b);
        auto ssv = convolution(a,bsum);
        for(int i=0;i<L;++i){ size1[idx][i]=szv[i]; sum1[idx][i]=ssv[i]; }
    }

    // Phase II: k-separator
    int log4k = (int)ceil(log2(double(4*k)));
    int logm  = (int)ceil(log2(double(m)));
    double ln2n = log(double(2*n));
    vector<vector<bool>> F2;
    for(int j = log4k; j <= logm; ++j) {
        double term1 = (16.0/alpha) * ln2n;
        double term2 = (2.0/alpha) * (j * double(k)) / (j - 1 - log2(double(k)));
        int rj = (int)ceil(term1 + term2);
        double prob = 1.0 / (1u << j);
        uniform_real_distribution<double> dist2(0.0,1.0);
        while(rj-- > 0) {
            vector<bool> subset(m,false);
            for(int idx=0; idx<m; ++idx)
                if(p[idx] && dist2(gen) < prob)
                    subset[idx] = true;
            F2.push_back(move(subset));
        }
    }
    int F2sz = F2.size();
    vector<vector<int>> size2(F2sz, vector<int>(L));
    vector<vector<int>> sum2(F2sz, vector<int>(L));
    for(int idx=0; idx<F2sz; ++idx) {
        vector<int> b(m), bsum(m);
        for(int j=0; j<m; ++j)
            if(F2[idx][j] && p[j]) { b[j]=1; bsum[j]=j; }
        auto szv = convolution(a,b);
        auto ssv = convolution(a,bsum);
        for(int i=0;i<L;++i){ size2[idx][i]=szv[i]; sum2[idx][i]=ssv[i]; }
    }

    // reconstruction containers
    vector<vector<int>> recovered(L);
    vector<vector<bool>> seen(L, vector<bool>(m,false));

    // Phase I peeling
    for(int i=0;i<L;++i){
        int need = min(k, full_size[i]);
        bool progress = true;
        while((int)recovered[i].size() < need && progress){
            progress = false;
            for(int idx=0; idx<F1sz; ++idx){
                if(size1[idx][i]==1){
                    int x = sum1[idx][i];
                    if(x>=0 && x<m && !seen[i][x]){
                        seen[i][x]=true;
                        recovered[i].push_back(x);
                        // update
                        for(int j2=0;j2<F1sz;++j2)
                            if(F1[j2][x]){
                                --size1[j2][i];
                                sum1[j2][i]-=x;
                            }
                        progress = true;
                        break;
                    }
                }
            }
        }
    }
    // Phase II scanning
    for(int i=0;i<L;++i){
        while((int)recovered[i].size() < k){
            bool found = false;
            for(int idx=0; idx<F2sz; ++idx){
                if(size2[idx][i]==1){
                    int x = sum2[idx][i];
                    if(x>=0 && x<m && !seen[i][x]){
                        seen[i][x]=true;
                        recovered[i].push_back(x);
                        found=true;
                        break;
                    }
                }
            }
            if(!found) break;
        }
        sort(recovered[i].begin(), recovered[i].end());
    }
    return recovered;
}

/**
 * Randomized k-witnesses
 */
vector<vector<int>> k_find_witnesses_randomized(
    vector<int> &a,
    vector<int> &b,
    int k
) {
    int n = (int)a.size();
    int m = (int)b.size();
    if (n < m) return {};

    // Build binary strings from the input vectors
    string text; text.reserve(n);
    string pat;  pat.reserve(m);
    for (int x : a) text.push_back(x ? '1' : '0');
    for (int y : b) pat.push_back(y ? '1' : '0');

    // Run the randomized reconstruction
    auto recovered = k_reconstruct_randomized(text, pat, k);

    // recovered[i] now holds up to k positions j in [0,m) where a[i+j] & b[j] == 1
    return recovered;
}

/**
 * Randomized k-witnesses knapsack
 */
vector<vector<int>> k_find_witnesses_knapsack(
    vector<int> &a,
    vector<int> &b,
    vector<int> &order,
    vector<int> &w,
    int k
) {
    int U = (int)a.size();
    int S = (int)b.size();
    int C = (int)order.size() - 1; // order is 1-indexed

    // 1) Use the generic k-witness finder on the binary vectors a, b
    auto rec = k_find_witnesses_randomized(a, b, k);
    int L = (int)rec.size();  // alignment count = U - S + 1

    // 2) Build mapping from weight -> list of randomized-order indices
    vector<vector<int>> weightToOIdx(U);
    for (int oidx = 1; oidx <= C; ++oidx) {
        int coin = order[oidx];      // real coin index
        int wt = w[coin];            // its weight
        if (wt >= 0 && wt < U && a[wt]) {
            weightToOIdx[wt].push_back(oidx);
        }
    }

    // 3) Invert order: randomized-order index -> real coin index
    vector<int> invOrder(C+1);
    for (int oidx = 1; oidx <= C; ++oidx) {
        invOrder[oidx] = order[oidx];
    }

    // 4) Collect up to k real coin indices per convolution-sum position
    vector<vector<int>> witnesses(U + S - 1);
    for (int i = 0; i < L; ++i) {
        // alignment i corresponds to convolution position i+(S-1)
        int s = i + (S - 1);
        auto &out = witnesses[s];
        for (int j : rec[i]) {
            int wt = i + j;  // weight position in a
            for (int oidx : weightToOIdx[wt]) {
                out.push_back(invOrder[oidx]);
                if ((int)out.size() >= k) break;
            }
            if ((int)out.size() >= k) break;
        }
    }

    return witnesses;
}


