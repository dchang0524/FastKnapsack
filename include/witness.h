#ifndef WITNESS_H
#define WITNESS_H

#include "constants.h"
#include "convolution.h"
//algorithms related to witnesses in boolean convolutions

//finding the minimum witness in O(n^(1.5) log n) time under a lexicoraphical order
//can also easily extended to find K minimum witnesses in O(n^(1.5) * sqrt(K) * log n) time under
//Can also easily extended to any convolutions that can be computed in O(n log n) time and the entry in 'b' corresponding to a witness in 'a' and a result element in 'b' can be found in O(1) time  
vector<int> minimum_witness_boolCnv_ordered(vector<int>& a, vector<int>& b, vector<int>& order);
#endif