#ifndef CONVOLUTION_H
#define CONVOLUTION_H

#include "constants.h"

#define rep(i, a, b) for(int i = a; i < (b); ++i)
#define all(x) begin(x), end(x)
#define sz(x) (int)(x).size()
typedef vector<int> vi;
typedef complex<double> C;
typedef vector<double> vd;

void fft(vector<C>& a);
vd conv(const vd& a, const vd& b);
vector<int> convolution(const vector<int>& a, const vector<int>& b);

// (max, +) convolution
vector<ll> maxPlusCnv(const vector<ll>& a, const vector<ll>& b);

// Boolean OR‐convolution (0/1 result)
vector<int> boolCnv(const vector<int>& a, const vector<int>& b);

#endif // CONVOLUTION_H
