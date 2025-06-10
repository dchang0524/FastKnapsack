// src/traditional.cpp
#include <bits/stdc++.h>
using namespace std;
typedef long long ll;

// a very straightforward unbounded‐knapsack: 
// dp[c] = max profit with total weight exactly c, or -inf if impossible.
int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n,u,t;
    if(!(cin>>n>>u>>t)) return 0;
    vector<int> w(n+1), p(n+1);
    for(int i=1;i<=n;i++){
        cin>>w[i]>>p[i];
    }
    const int NEG_INF = -1000000000;
    vector<ll> dp(t+1, NEG_INF);
    dp[0] = 0;
    // classic unbounded DP: for each coin, for c from w…t
    for(int i=1;i<=n;i++){
        for(int c=w[i]; c<=t; ++c){
            if(dp[c-w[i]]>NEG_INF)
                dp[c] = max(dp[c], dp[c-w[i]] + p[i]);
        }
    }
    // output exactly the same format as your optimized solver
    for(int c=0;c<=t;c++){
        if(dp[c]==NEG_INF) 
            cout<<-1000000000<<"\n";
        else
            cout<<dp[c]<<"\n";
    }
    return 0;
}
