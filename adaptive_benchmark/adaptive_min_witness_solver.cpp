#include <bits/stdc++.h>
#include "algorithms.h"   // defines adaptiveMinWitness_randomized(...)
using namespace std;

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int p, n;
    cin >> p >> n;
    vector<vector<int>> a(p, vector<int>(n)), b(p, vector<int>(n));
    for(int i = 0; i < p; i++)
      for(int j = 0; j < n; j++)
        cin >> a[i][j];
    for(int i = 0; i < p; i++)
      for(int j = 0; j < n; j++)
        cin >> b[i][j];

    vector<int> w(n), order(n);
    for(int i = 0; i < n; i++) cin >> w[i];
    for(int i = 0; i < n; i++) cin >> order[i];

    // compute the raw convolution counts
    int r = 2*n - 1;
    vector<vector<int>> c(p, vector<int>(r));
    for(int i = 0; i < p; i++)
      c[i] = convolution(a[i], b[i]);

    // call your existing adaptive routine
    auto res = adaptiveMinWitness_randomized(a, b, c, w, order);

    // output p lines of r integers each (−1 means “no witness”)
    for(int i = 0; i < p; i++){
      for(int j = 0; j < r; j++){
        cout << res[i][j] << (j+1<r?' ':'\n');
      }
    }
    return 0;
}
