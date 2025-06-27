#include <bits/stdc++.h>
#include "convolution.h"
#include "witness.h"
using namespace std;
using namespace std::chrono;

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if(!(cin>>n)) return 0;

    vector<int> a(n), b(n), w(n), order(n);
    for(int i=0;i<n;i++) cin>>a[i];
    for(int i=0;i<n;i++) cin>>b[i];
    for(int i=0;i<n;i++) cin>>w[i];
    for(int i=0;i<n;i++) cin>>order[i];

    auto t0 = high_resolution_clock::now();
    vector<int> witness = minimum_witness_random(a, b, w, order);
    auto t1 = high_resolution_clock::now();
    double secs = duration<double>(t1 - t0).count();
    cerr << "RandomizedMinWitness took " << secs << " s\n";

    for(int x : witness) cout << x << " ";
    cout << "\n";
    return 0;
}
