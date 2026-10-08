#ifndef PROBABILITY_SPACE_H
#define PROBABILITY_SPACE_H

#include <cstdint>
#include <vector>

// Naor--Naor composition: a small-bias space on the seed of a linear
// c-wise independent polynomial generator. The distribution is uniform over
// sample indices [0, size()). For every <= c output coordinates and every
// bit pattern, the probability differs from uniform by at most atom_error().
class AlmostIndependentSpace {
public:
    AlmostIndependentSpace(int variables, int wise, double atom_error);
    uint64_t size() const;
    int variables() const;
    int wise() const;
    double atom_error() const;
    std::vector<unsigned char> sample(uint64_t index) const;

private:
    int variables_, wise_, coefficient_bits_, seed_bits_, outer_bits_;
    double atom_error_;
    uint64_t size_;
    uint64_t coefficient_modulus_, outer_modulus_;
    std::vector<std::vector<uint64_t>> influence_;
};

#endif
