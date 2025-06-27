#include "witness.h"
/**
 * Computes the minimum witnesses of a boolean convolution for each result element
 * The `order` vector specifies the lexicographical order of the indices. 
 */
vector<int> minimum_witness_boolCnv_ordered(vector<int>& a, vector<int>& b, const vector<int>& w, vector<int>& order) {
    //create sqrt(n) vectors depending on what division of sqrt(n) the index is in
    int n = order.size();
    int sqrt_n = (int)ceil(sqrt(n));
    vector<vector<int>> a_P(sqrt_n + 1, vector<int>(a.size())); //division of a into O(sqrt(n)) parts
    vector<vector<int>> id(sqrt_n + 1, vector<int>());
    for (int i = 0; i < n; i++) {
        if (a[w[order[i]]] == 0) {
            continue;
        }
        a_P[i / sqrt_n][w[order[i]]] = 1;
        id[i/sqrt_n].push_back(order[i]);
    }

    vector<vector<int>> groups(a_P.size()); //group[i] contains the result elements that have their minimum witness in group i

    vector<int> visited(n + b.size() - 1, 0);
    for (int g = 0; g < a_P.size(); g++) {
        vector<int> c_g = boolCnv(a_P[g], b);
        for (int i = 0; i < c_g.size(); i++) {
            if (c_g[i] == 1 && visited[i] == 0) {
                groups[g].push_back(i);
                visited[i] = 1;
            }
        }
    }

    vector<int> min_witness(n + b.size() - 1, -1);
    vector<int> inverseOrder(order.size());
    for (int i = 0; i < order.size(); i++) {
        inverseOrder[order[i]] = i;
    }
    for (int g = 0; g < a_P.size(); g++) {
        for (int i : id[g]) {
            for (int result : groups[g]) {
                if (a_P[g][w[i]] == 1 && result - w[i] >= 0 && result - w[i] < b.size() && b[result - w[i]] == 1) {
                    int idx = inverseOrder[i];
                    if (min_witness[result] == -1 || idx < min_witness[result]) {
                        min_witness[result] = idx;
                    }
                }
            }
        }
    }    
    return min_witness;
}

/**
 * Uniformly samples a witness for each result element, in expected O(n log^2 n) time
 */
vector<int> randomized_witness_sampling(vector<int>& a, vector<int>& b) {
    // cerr << "Randomized witness sampling started" << endl;
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    mt19937 rng(seed);
    bernoulli_distribution coin(0.5);
    
    vector<int> c = convolution(a, b);

    vector<int> aVal(a.size(), 0);
    for (int i = 0; i < a.size(); ++i) {
        if (a[i] > 0) {
            aVal[i] = i;
        }
    }
    vector<int> cVal;
    cVal = convolution(aVal, b);
    vector<int> witness(c.size(), -1);
    int need = 0;
    int cnt = 0;

    for (int i = 0; i < sz(c); ++i) {
        if (c[i] > 0) {
            need++;
            if (c[i] == 1) {
                witness[i] = cVal[i];
                cnt++;
            }
        }
    }

    // for (int i = 0; i < sz(c); ++i) {
    //         cerr << c[i] << " ";
    // }
    // cerr << endl;
    // cerr << "Need: " << need << ", cnt: " << cnt << endl;
    
    while (cnt < need) {
        int K = (int)ceil(log2(sz(a)));
        vector<int> aDiluted(a.size(), 0);
        rep(i, 0, sz(a)) {
            aDiluted[i] = a[i];
        }
        for (int k = 0; k < K; ++k) {
            rep(i, 0, sz(aDiluted)) {
                int bit = coin(rng) ? 1 : 0;
                aDiluted[i] = aDiluted[i] & bit;          
            }
            rep(i, 0, sz(aDiluted)) {
                if (aDiluted[i] > 0) {
                    aVal[i] = i;
                } else {
                    aVal[i] = 0;
                }
            }
            c = convolution(aDiluted, b);
            cVal = convolution(aVal, b);
            rep(i, 0, sz(c)) {
                if (c[i] == 1 && witness[i] == -1) {
                    witness[i] = cVal[i];
                    cnt++;
                }
            }
        }

        // for (int i = 0; i < sz(aDiluted); ++i) {
        //     cerr << aDiluted[i] << " ";
        // }
        // cerr << endl;
        // cerr << "Need: " << need << ", cnt: " << cnt << endl;
    }
    return witness;
}

/**
 * Finds the minimum witness with respect to a random order in expected O~(n) time.
*/
vector<int> minimum_witness_random(vector<int>& a, vector<int>& b, vector<int>& w, vector<int>& order) {
    unordered_map<int, int> ind; //converts weight to order index
    for (int i = 1; i < sz(order); ++i) {
        ind[w[order[i]]] = i;
    }

    // for (auto mp : ind) {
    //     cerr << mp.first << " -> " << mp.second << endl;
    // }

    vector<unordered_set<int>> witSets(sz(a) + sz(b) - 1);
    vector<int> min_witness(sz(a) + sz(b) - 1, -1);
    vector<int> found(sz(a) + sz(b) - 1, 0);
    int l = 1;
    vector<int> aPref(sz(a), 0);
    while (l <= sz(order)-1) {
        // cerr << "Current l: " << l << endl;

        for (int i = 1; i <= l; ++i) {
            aPref[w[order[i]]] = 1;
        }
        // cerr << "aPref: " << endl;
        // for (int i = 0; i < sz(aPref); ++i) {
        //     cerr << aPref[i] << " ";
        // }
        // cerr << endl;
        vector<int> c = convolution(aPref, b);
        // cerr << "c: " << endl;
        // for (int i = 0; i < sz(c); ++i) {
        //     cerr << c[i] << " ";
        // }
        // cerr << endl;
        int need = 0;
        for (int i = 0; i < sz(c); ++i) {
            if (c[i] > 0 && found[i] == 0) {
                need++;
            }
        }
        int cnt = 0;
        while (cnt < need) {
            vector<int> sample = randomized_witness_sampling(aPref, b);
            // cerr << "Sample: " << endl;
            // for (int i = 0; i < sz(sample); ++i) {
            //     cerr << sample[i] << " ";
            // }
            // cerr << endl;
            for (int i = 0; i < sz(sample); ++i) {
                if (sample[i] != -1 && found[i] == 0) {
                    // cerr << "Found witness for " << i << ": " << ind[sample[i]] << endl;
                    witSets[i].insert(ind[sample[i]]);
                    if (witSets[i].size() == c[i]) {
                        found[i] = 1;
                        min_witness[i] = *witSets[i].begin();
                        cnt++;
                    }
                }
            }
        }
        // for (int i = 0; i < sz(c); ++i) {
        //     cerr << "witnees for " << i << ": ";
        //     for (int i : witSets[i]) {
        //         cerr << i << " ";
        //     }
        //     cerr << endl;
        // }
        if (l == sz(order) - 1) {
            break;
        }
        l = min(sz(order) - 1, l * 2);
    }
    return min_witness;
}