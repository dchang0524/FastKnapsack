#include <bits/stdc++.h>
#include "witness.h"
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

    // Time the optimized minimum‐witness routine
    auto t0 = chrono::high_resolution_clock::now();
    vector<int> res = minimum_witness_boolCnv_ordered(a, b, w, order);
    auto t1 = chrono::high_resolution_clock::now();
    double secs = chrono::duration<double>(t1 - t0).count();
    cerr << "Optimized witness took " << secs << " s\n";

    // Emit the witness vector
    for(int x : res) cout << x << " ";
    cout << "\n";
    return 0;
}
