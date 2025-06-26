#include <bits/stdc++.h>
using namespace std;

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if(!(cin >> n)) return 0;
    vector<int> a(n), b(n), w(n), order(n);
    for(int i = 0; i < n; i++) cin >> a[i];
    for(int i = 0; i < n; i++) cin >> b[i];
    for(int i = 0; i < n; i++) cin >> w[i];
    for(int i = 0; i < n; i++) cin >> order[i];

    // Build inverse order: order[pos] = idx  ⇒  inv[idx] = pos
    vector<int> inv(n);
    for(int pos = 0; pos < n; pos++){
        inv[ order[pos] ] = pos;
    }

    int R = n + n - 1;
    vector<int> res(R, -1);

    // Time the naive O(n²) witness finder
    auto t0 = chrono::high_resolution_clock::now();
    for(int c = 0; c < R; c++){
        int best = -1;
        for(int i = 0; i < n; i++){
            int j = c - w[i];
            if(j < 0 || j >= n) continue;
            if(a[i] && b[j]){
                int pos = inv[i];
                if(best < 0 || pos < best) best = pos;
            }
        }
        res[c] = best;
    }
    auto t1 = chrono::high_resolution_clock::now();
    double secs = chrono::duration<double>(t1 - t0).count();
    cerr << "Naive witness took " << secs << " s\n";

    // Emit the witness vector
    for(int x : res) cout << x << " ";
    cout << "\n";
    return 0;
}
