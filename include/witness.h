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

// The Lingas--Persson grouped-convolution extension: return the first
// min(k, witness count) first-array indices under a 0-based permutation.
vector<vector<int>> k_minimum_witnesses_boolCnv_ordered(
    const vector<int>& a, const vector<int>& b,
    const vector<int>& order, int k);

/**
 * Uniformly samples a witness for each result element, in expected O(n log^2 n) time
 */
vector<int> randomized_witness_sampling(vector<int>& a, vector<int>& b);

/**
 * Finds the minimum witness with respect to a random order in expected O~(n) time.
 */
vector<int> minimum_witness_random(vector<int>& a, vector<int>& b, const vector<int>& w, vector<int>& order);

vector<vector<int>> randomized_k_witness(vector<int>& a, vector<int>& b, int k, const vector<int>& w, vector<int>& order);
vector<vector<int>> optimized_k_witness(vector<int>& a, vector<int>& b, int k, const vector<int>& w, vector<int>& order);

// For each convolution output s, return exactly min(k, |{i:a[i] && b[s-i]}|)
// distinct indices i of a. The randomized implementation is Las Vegas: it
// verifies every candidate and repeats adaptive dilution chains until exact.
vector<vector<int>> k_witnesses_boolean_randomized(
    const vector<int>& a, const vector<int>& b, int k,
    const vector<unsigned char>* requested = nullptr);

// Optional, per-thread benchmark diagnostics. Disabled until explicitly reset
// with enabled=true; observing a run does not change its branch decisions.
struct RandomizedWitnessStats {
    unsigned long long requested_enumeration_calls = 0;
    unsigned long long pair_enumeration_calls = 0;
    unsigned long long sampling_calls = 0;
    unsigned long long dilution_chains = 0;
    unsigned long long weighted_convolutions = 0;
    int max_k = 0;
};
void reset_randomized_witness_stats(bool enabled = true);
RandomizedWitnessStats randomized_witness_stats();

// Deterministic k-reconstruction via Alon--Naor dilution and an explicit
// c-wise almost-independent space. The sample-space enumeration is
// polylogarithmic in the input length but can have a large constant.
vector<vector<int>> k_witnesses_boolean_deterministic(
    const vector<int>& a, const vector<int>& b, int k,
    const vector<unsigned char>* requested = nullptr);

// Randomized bounded peeler plus large-set separator from Aumann et al.
// Returns exact results; a new random family is drawn until all requested
// outputs have min(k, witness count) distinct positions.
vector<vector<int>> k_witnesses_boolean_optimized(
    const vector<int>& a, const vector<int>& b, int k,
    const vector<unsigned char>* requested = nullptr);
#endif
