import random
import math

# ----------------------------------------------------------------------
# Utility routines for polynomials over GF(2), represented as Python ints.
# Bit i of the int corresponds to the coefficient of t^i.
# ----------------------------------------------------------------------

def gf2_degree(p: int) -> int:
    """Return the degree of the polynomial p (as an int) over GF(2)."""
    return p.bit_length() - 1

def gf2_mul(a: int, b: int) -> int:
    """
    Multiply two GF(2) polynomials a and b.  Both are ints whose bits
    are the coefficients.  Return the product (no reduction).
    """
    #could use FFT
    result = 0
    while b:
        if (b & 1) == 1:
            result ^= a
        a <<= 1
        b >>= 1
    return result

def gf2_mod(poly: int, mod_poly: int) -> int:
    """
    Compute (poly mod mod_poly) in GF(2)[x].  Both arguments are ints
    whose bits represent polynomials.  Returns the remainder (deg < deg(mod_poly)).
    """
    deg_mod = gf2_degree(mod_poly)
    while gf2_degree(poly) >= deg_mod:
        shift = gf2_degree(poly) - deg_mod
        poly ^= (mod_poly << shift)
    return poly

def gf2_gcd(a: int, b: int) -> int:
    """
    Compute gcd(a, b) for GF(2) polynomials.  Uses the Euclidean algorithm,
    where "division" is implemented by gf2_mod.
    """
    while b:
        a, b = b, gf2_mod(a, b)
    return a

def gf2_pow_mod(base: int, exponent: int, mod_poly: int) -> int:
    """
    Compute (base^exponent) mod mod_poly in GF(2)[x], via repeated squaring.
    - base: int (polynomial)
    - exponent: nonnegative int
    - mod_poly: irreducible polynomial (int)
    Returns an int representing the remainder in GF(2)[x].
    """
    result = 1
    base = gf2_mod(base, mod_poly)
    while exponent > 0:
        if (exponent & 1) == 1:
            result = gf2_mod(gf2_mul(result, base), mod_poly)
        base = gf2_mod(gf2_mul(base, base), mod_poly)
        exponent >>= 1
    return result

# ----------------------------------------------------------------------
# Irreducibility test (Rabin's test) over GF(2)[x], for a candidate of degree m.
# ----------------------------------------------------------------------

def is_irreducible(poly: int, m: int) -> bool:
    """
    Rabin test for checking that `poly` is irreducible over GF(2) of degree exactly m.

    Steps:
      1) Check that deg(poly) == m and constant term (bit 0) = 1  (monic).
      2) Let x(t) = t (represented by int 0b10).  Check x^(2^m) ≡ x (mod poly).
      3) For each proper divisor d of m, check gcd(poly, x^(2^(m/d)) - x) == 1.

    Returns True iff `poly` passes all checks.
    """
    # 1) Must be monic of degree m (highest bit = 1) and have constant bit = 1
    if gf2_degree(poly) != m or (poly & 1) == 0:
        return False

    # Represent x(t) = t as integer 0b10
    x_poly = 0b10

    # 2) Check x^(2^m) mod poly == x
    if gf2_pow_mod(x_poly, 1 << m, poly) != x_poly:
        return False

    # 3) Check GCD conditions for each proper divisor of m
    #    Collect divisors of m
    divisors = []
    d = 1
    while d * d <= m:
        if m % d == 0:
            divisors.append(d)
            if d != m // d:
                divisors.append(m // d)
        d += 1

    for d in divisors:
        if 1 < d < m:
            xp = gf2_pow_mod(x_poly, 1 << (m // d), poly)
            # Compute gcd(poly, xp - x).  In GF(2), subtraction = XOR
            if gf2_gcd(poly, xp ^ x_poly) != 1:
                return False

    return True

def find_irreducible(m: int) -> int:
    """
    Finds and returns a monic irreducible polynomial of degree m over GF(2),
    represented as an int.  Searches all candidates of the form x^m + ... + 1.

    Raises ValueError if none is found (theoretically impossible for m >= 1).
    """
    start = (1 << m) | 1          # The polynomial x^m + 1  (binary: 1 followed by m−1 zeros, then 1)
    end = 1 << (m + 1)            # All polynomials from 2^m .. 2^(m+1)-1
    for poly in range(start, end, 2):  # step by 2 to ensure constant term = 1 (odd ints)
        if is_irreducible(poly, m):
            return poly

    raise ValueError(f"No irreducible polynomial found for degree {m}")

# ----------------------------------------------------------------------
# A small wrapper class for elements of GF(2^m) using a fixed irreducible.
# ----------------------------------------------------------------------

class GF2m:
    """
    Represents an element of GF(2^m) by storing an integer `value` (0 <= value < 2^m),
    along with the irreducible polynomial `mod_poly` of degree m.  Arithmetic is
    automatically done mod `mod_poly` in GF(2)[x].
    """

    def __init__(self, value: int, mod_poly: int, m: int):
        self.mod_poly = mod_poly
        self.m = m
        self.value = gf2_mod(value, mod_poly)  # Always keep the value reduced

    def __add__(self, other: "GF2m") -> "GF2m":
        return GF2m(self.value ^ other.value, self.mod_poly, self.m)

    def __mul__(self, other: "GF2m") -> "GF2m":
        return GF2m(gf2_mul(self.value, other.value), self.mod_poly, self.m)

    def __pow__(self, exponent: int) -> "GF2m":
        return GF2m(gf2_pow_mod(self.value, exponent, self.mod_poly), self.mod_poly, self.m)

    def to_bits(self) -> [int]:
        """
        Return a list of m bits (length = m), representing the polynomial coefficients
        of self.value in descending order (highest‐degree bit first).
        """
        return [(self.value >> i) & 1 for i in range(self.m - 1, -1, -1)]

# ----------------------------------------------------------------------
# Construction 3: Build an n‐bit ε‐biased sample.
# ----------------------------------------------------------------------

def generate_biased_sample(n: int, epsilon: float) -> [int]:
    """
    Generates an n‐bit string using Construction 3 (powering in GF(2^m)) with bias ≤ ε.

    Steps:
      1) Compute m = ceil(log2(n/ε)), so 2^m ≥ n/ε.
      2) Find a monic irreducible polynomial f(t) of degree m over GF(2).
      3) Define the field GF(2^m) via reduction mod f(t).
      4) Pick random x ≠ 0 and random y in GF(2^m).
      5) For each i=0..n-1, set
           r_i = 〈 bin(x^i), bin(y) 〉_2,
         where “bin(·)” is the m‐bit representation and “〈·,·〉_2” is the dot‐product mod 2.
    Returns:
      A list of n bits (0 or 1).
    """
    # 1) Choose m so that 2^m >= n/ε
    target = math.ceil(n / epsilon)
    m = target.bit_length()  # smallest integer with 2^m > target
    # (we know 2^(m-1) <= target < 2^m → so 2^m >= target ≥ n/ε)

    # 2) Find an irreducible polynomial f of degree m
    f_poly = find_irreducible(m)

    # 3) Field is set up by the GF2m class which reduces mod f_poly

    # 4) Pick random x (nonzero) and y (any) in GF(2^m)
    max_val = (1 << m) - 1
    x_val = random.randint(1, max_val)  # avoid zero for mixing
    y_val = random.randint(0, max_val)
    x = GF2m(x_val, f_poly, m)
    y = GF2m(y_val, f_poly, m)

    # 5) Build the n‐bit output array
    result = []
    current = GF2m(1, f_poly, m)  # x^0 = 1 in the field
    for _ in range(n):
        bits_xi = current.to_bits()  # m‐bit list for x^i
        bits_y = y.to_bits()         # m‐bit list for y
        # Dot‐product mod 2:
        dot = 0
        for bx, by in zip(bits_xi, bits_y):
            dot ^= (bx & by)
        result.append(dot)

        # Move on to the next power: x^(i+1) = (x^i) * x
        current = current * x

    return result

# ----------------------------------------------------------------------
# Example usage:
# ----------------------------------------------------------------------
if __name__ == "__main__":
    n = 16
    epsilon = 0.1
    sample = generate_biased_sample(n, epsilon)
    print("Generated sample of length", n, ":", sample)
