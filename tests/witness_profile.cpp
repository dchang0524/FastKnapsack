#include "witness.h"
#include <iomanip>
#include <string>

// A benchmark, not a ctest: intentionally exceeds the enumeration threshold.
// Independently scanning candidate positions supplies the exact cardinalities.
int main(int argc, char** argv) {
    if (argc != 4) return 2;
    const string family = argv[1];
    const int length = stoi(argv[2]);
    mt19937_64 rng(stoull(argv[3]));
    vector<int> a(length), b(2 * length);
    vector<unsigned char> requested(a.size() + b.size() - 1, 1);
    if (family == "dense-broad" || family == "dense-narrow") {
        fill(a.begin(), a.end(), 1);
        fill(b.begin(), b.end(), 1);
        if (family == "dense-narrow") {
            fill(requested.begin(), requested.end(), 0);
            for (int s = length - 1; s < 2 * length; ++s) requested[s] = 1;
        }
    } else if (family == "random-half" || family == "random-sparse") {
        bernoulli_distribution present(family == "random-half" ? 0.5 : 0.02);
        for (int& v : a) v = present(rng);
        for (int& v : b) v = present(rng);
    } else if (family == "periodic") {
        for (int i = 0; i < length; ++i) a[i] = i % 7 == 0;
        for (int j = 0; j < 2 * length; ++j) b[j] = j % 11 == 0;
    } else if (family == "separated-blocks") {
        for (int i = 0; i < length; ++i) a[i] = i < length / 8 || i >= 7 * length / 8;
        for (int j = 0; j < 2 * length; ++j) b[j] = j < length / 4 || j >= 7 * length / 4;
    } else return 2;
    const int k = static_cast<int>(ceil(log2(requested.size())));
    using Clock = chrono::steady_clock;
    const auto start_direct = Clock::now();
    vector<vector<int>> reference(requested.size());
    for (size_t s = 0; s < requested.size(); ++s) if (requested[s]) {
        const size_t first = s >= b.size() ? s - b.size() + 1 : 0;
        const size_t last = min(s, a.size() - 1);
        for (size_t i = first; i <= last && reference[s].size() < size_t(k); ++i)
            if (a[i] && b[s - i]) reference[s].push_back(static_cast<int>(i));
    }
    const double direct_seconds = chrono::duration<double>(Clock::now() - start_direct).count();
    reset_randomized_witness_stats();
    const auto start_random = Clock::now();
    const auto recovered = k_witnesses_boolean_randomized(a, b, k, &requested);
    const double random_seconds = chrono::duration<double>(Clock::now() - start_random).count();
    for (size_t s = 0; s < recovered.size(); ++s) {
        if (recovered[s].size() != reference[s].size()) return 3;
        unordered_set<int> seen;
        for (int i : recovered[s])
            if (i < 0 || i >= length || s < size_t(i) || s - i >= b.size() ||
                !a[i] || !b[s - i] || !seen.insert(i).second) return 4;
    }
    const auto stats = randomized_witness_stats();
    // This benchmark is invalid if a shortcut was taken.
    if (stats.sampling_calls != 1 || stats.requested_enumeration_calls ||
        stats.pair_enumeration_calls) return 5;
    cout << setprecision(12)
         << "{\"random_seconds\":" << random_seconds
         << ",\"direct_seconds\":" << direct_seconds
         << ",\"k\":" << k
         << ",\"sampling_calls\":" << stats.sampling_calls
         << ",\"dilution_chains\":" << stats.dilution_chains
         << ",\"weighted_convolutions\":" << stats.weighted_convolutions
         << ",\"validation\":\"exact\"}\n";
}
