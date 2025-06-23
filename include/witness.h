#ifndef WITNESS_H
#define WITNESS_H

#include "constants.h"
#include "convolution.h"
//algorithms related to witnesses in boolean convolutions

/**
 * finding the minimum witness in O(n^(1.5) log n) time under a lexicoraphical order for boolean convolutions
 * Can be easily extended to find K minimum witnesses in O(n^(1.5) * sqrt(K) * log n) time
 * Can also be easily extended to some other convolutions, like polynomial convolutions
 */
vector<int> minimum_witness_boolCnv_ordered(vector<int>& a, vector<int>& b, vector<int>& order);
#endif