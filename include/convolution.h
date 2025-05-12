#ifndef CONVOLUTION_H
#define CONVOLUTION_H

#include <vector>
using namespace std;

// Naïve boolean‐count convolution (replaceable with FFT)
vector<int> boolCountConv(const vector<int>& a, const vector<int>& b);

// Naïve integer‐sum convolution
vector<long long> intSumConv(const vector<long long>& aSum, const vector<int>& b);

// (max, +) convolution
vector<int> maxPlusCnv(const vector<int>& a, const vector<int>& b);

// Boolean OR‐convolution (0/1 result)
vector<int> boolCnv(const vector<int>& a, const vector<int>& b);

#endif // CONVOLUTION_H
