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
vector<int> minimum_witness_boolCnv_ordered(vector<int>& a, vector<int>& b, const vector<int>& w, vector<int>& order);

/**
 * Uniformly samples a witness for each result element, in expected O(n log^2 n) time
 */
vector<int> randomized_witness_sampling(vector<int>& a, vector<int>& b);

/**
 * Finds the minimum witness with respect to a random order in expected O~(n) time.
 */
vector<int> minimum_witness_random(vector<int>& a, vector<int>& b, const vector<int>& w, vector<int>& order);

vector<vector<int>> randomized_k_witness(vector<int>& a, vector<int>& b, int k, const vector<int>& w, vector<int>& order);
#endif