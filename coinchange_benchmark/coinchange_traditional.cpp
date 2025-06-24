// coinchange_traditional.cpp
#include <bits/stdc++.h>
using namespace std;

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n,u,T;
    if(!(cin>>n>>u>>T)) return 0;
    vector<int> w(n), p(n);
    for(int i=0;i<n;i++){
        cin>>w[i]>>p[i]; // p[i] is ignored here
    }

    const int INF = 1e9;
    vector<int> dp(T+1, INF);
    dp[0] = 0;
    for(int i=0;i<n;i++){
        for(int t=w[i]; t<=T; t++){
            if(dp[t-w[i]]!=INF)
                dp[t] = min(dp[t], dp[t-w[i]] + 1);
        }
    }
    for(int t=0; t<=T; t++){
        cout << (dp[t]==INF ? -1 : dp[t]) << "\n";
    }
    return 0;
}
