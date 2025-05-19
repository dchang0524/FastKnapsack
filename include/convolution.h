#ifndef CONVOLUTION_H
#define CONVOLUTION_H

#include "constants.h"

// Naïve boolean‐count convolution (replaceable with FFT)
vector<int> boolCountConv(const vector<int>& a, const vector<int>& b);

// Naïve integer‐sum convolution
vector<long long> intSumConv(const vector<long long>& aSum, const vector<int>& b);

// (max, +) convolution
vector<ll> maxPlusCnv(const vector<ll>& a, const vector<ll>& b);

// Boolean OR‐convolution (0/1 result)
vector<int> boolCnv(const vector<int>& a, const vector<int>& b);

#endif // CONVOLUTION_H
