#include <bits/stdc++.h>
#include "convolution.h"
#include "witness.h"
using namespace std;
using namespace std::chrono;

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, k;
    if(!(cin >> n >> k)) return 0;

    vector<int> a(n), b(n), w(n), order(n);
    for(int i = 0; i < n; i++) cin >> a[i];
    for(int i = 0; i < n; i++) cin >> b[i];
    for(int i = 0; i < n; i++) cin >> w[i];
    for(int i = 0; i < n; i++) cin >> order[i];

    auto t0 = high_resolution_clock::now();
    vector<vector<int>> witnesses = randomized_k_witness(a, b, k, w, order);
    auto t1 = high_resolution_clock::now();
    double secs = duration<double>(t1 - t0).count();
    cerr << "randomized_k_witness (n=" << n << ", k=" << k << ") took " << secs << " s\n";

    // print output: each line begins with count then indices
    for(const auto &vec : witnesses){
        cout << vec.size();
        for(int x : vec) cout << ' ' << x;
        cout << '\n';
    }
    return 0;
}