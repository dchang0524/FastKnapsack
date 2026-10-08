#include "probability_space.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace {
int degree(uint64_t polynomial) {
    if (!polynomial) return -1;
    return 63 - __builtin_clzll(polynomial);
}
uint64_t remainder(uint64_t value, uint64_t modulus) {
    const int m = degree(modulus);
    while (degree(value) >= m)
        value ^= modulus << (degree(value) - m);
    return value;
}
uint64_t gcd(uint64_t a, uint64_t b) {
    while (b) {
        uint64_t next = remainder(a, b);
        a = b;
        b = next;
    }
    return a;
}
uint64_t multiply(uint64_t a, uint64_t b, uint64_t modulus) {
    uint64_t product = 0;
    while (b) {
        if (b & 1) product ^= a;
        a <<= 1;
        b >>= 1;
    }
    return remainder(product, modulus);
}
bool irreducible(uint64_t f, int m) {
    if (m == 1) return f == 3;
    uint64_t x = 2;
    for (int i = 1; i <= m; ++i) {
        x = multiply(x, x, f);
        if (i <= m / 2 && m % i == 0 && gcd(x ^ 2, f) != 1)
            return false;
    }
    return x == 2;
}
uint64_t modulus_for(int bits) {
    if (bits < 1 || bits > 31)
        throw std::invalid_argument("finite-field degree outside 1..31");
    if (bits == 1) return 3;
    const uint64_t leading = uint64_t(1) << bits;
    for (uint64_t tail = 3; tail < leading; tail += 2) {
        uint64_t candidate = leading | tail;
        if (irreducible(candidate, bits)) return candidate;
    }
    throw std::runtime_error("irreducible binary polynomial not found");
}
uint64_t avalanche(uint64_t value) {
    value ^= value >> 30;
    value *= 0xbf58476d1ce4e5b9ULL;
    value ^= value >> 27;
    value *= 0x94d049bb133111ebULL;
    return value ^ (value >> 31);
}
int ceil_log2(uint64_t n) {
    int bits = 0;
    uint64_t power = 1;
    while (power < n) {
        power <<= 1;
        ++bits;
    }
    return std::max(1, bits);
}
}

AlmostIndependentSpace::AlmostIndependentSpace(
    int variables, int wise, double atom_error)
    : variables_(variables), wise_(wise), coefficient_bits_(0), seed_bits_(0),
      outer_bits_(0), atom_error_(atom_error), size_(0),
      coefficient_modulus_(0), outer_modulus_(0) {
    if (variables < 1 || wise < 1 || wise > variables ||
        !(atom_error > 0.0 && atom_error < 0.5))
        throw std::invalid_argument("invalid almost-independent space parameters");
    coefficient_bits_ = ceil_log2(uint64_t(variables) + 1);
    seed_bits_ = coefficient_bits_ * wise_;
    coefficient_modulus_ = modulus_for(coefficient_bits_);
    // For seed coordinate j, emit LSB(y*x^j). Every nonempty parity of seed
    // coordinates is LSB(y*P(x)) for a nonzero polynomial P of degree < d.
    // It is unbiased whenever P(x) != 0, so the bias is at most (d-1)/2^r.
    // Composing this small-bias seed with a linear exact c-wise independent
    // polynomial generator makes every <= c output pattern epsilon-close
    // to uniform by the Fourier inversion bound.
    outer_bits_ = ceil_log2(static_cast<uint64_t>(
        std::ceil(double(std::max(1, seed_bits_ - 1)) / atom_error_)));
    outer_modulus_ = modulus_for(outer_bits_);
    size_ = uint64_t(1) << (2 * outer_bits_);
    const int words = (seed_bits_ + 63) / 64;
    influence_.assign(variables_, std::vector<uint64_t>(words));
    for (int i = 0; i < variables_; ++i) {
        uint64_t at = uint64_t(i) + 1;
        uint64_t power = 1;
        for (int coefficient = 0; coefficient < wise_; ++coefficient) {
            for (int bit = 0; bit < coefficient_bits_; ++bit) {
                if (multiply(uint64_t(1) << bit, power,
                             coefficient_modulus_) & 1) {
                    int seed_bit = coefficient * coefficient_bits_ + bit;
                    influence_[i][seed_bit / 64] |=
                        uint64_t(1) << (seed_bit % 64);
                }
            }
            power = multiply(power, at, coefficient_modulus_);
        }
    }
}
uint64_t AlmostIndependentSpace::size() const { return size_; }
int AlmostIndependentSpace::variables() const { return variables_; }
int AlmostIndependentSpace::wise() const { return wise_; }
double AlmostIndependentSpace::atom_error() const { return atom_error_; }

std::vector<unsigned char> AlmostIndependentSpace::sample(uint64_t index) const {
    if (index >= size_) throw std::out_of_range("sample index out of range");
    // A fixed Feistel permutation changes enumeration order but preserves
    // the uniform distribution over the complete sample space. It exposes
    // useful dilutions early on ordinary inputs.
    const uint64_t mask = (uint64_t(1) << outer_bits_) - 1;
    uint64_t left = index & mask;
    uint64_t right = (index >> outer_bits_) & mask;
    for (uint64_t round = 0; round < 6; ++round) {
        uint64_t next = left ^ (avalanche(right +
            0x9e3779b97f4a7c15ULL * (round + 1)) & mask);
        left = right;
        right = next;
    }
    const uint64_t x = left;
    const uint64_t y = right;
    std::vector<uint64_t> seed((seed_bits_ + 63) / 64);
    uint64_t power = 1;
    for (int bit = 0; bit < seed_bits_; ++bit) {
        uint64_t value = multiply(y, power, outer_modulus_);
        seed[bit / 64] |= (value & 1) << (bit % 64);
        power = multiply(power, x, outer_modulus_);
    }
    std::vector<unsigned char> result(variables_);
    for (int i = 0; i < variables_; ++i) {
        int parity = 0;
        for (size_t word = 0; word < seed.size(); ++word)
            parity ^= __builtin_parityll(seed[word] & influence_[i][word]);
        result[i] = parity;
    }
    return result;
}
