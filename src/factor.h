#ifndef FACTOR_H
#define FACTOR_H
// ****************************************************************************
//  factor.h                                                     DB48X project
// ****************************************************************************
//
//   File Description:
//
//      Prime factorization and primality testing for bignum integers
//
//      Uses a multi-stage approach:
//        1. Trial division by small primes (up to ~10000)
//        2. Miller-Rabin primality test
//        3. Pollard's Rho with Brent's cycle detection + GCD batching
//
//      Memory usage: O(1) beyond the factor list itself
//      Time complexity: O(n^{1/4}) for each factor via Pollard Rho
//
//      High-level functions:
//        - IsPrime(n)  → bool
//        - Factors(n)  → list of { prime, exponent } pairs
//
// ****************************************************************************
//   This software is licensed under the terms outlined in LICENSE.txt
// ****************************************************************************

#include "bignum.h"


// ============================================================================
//
//   Factor result storage
//
// ============================================================================

struct prime_factor
// ----------------------------------------------------------------------------
//   A single prime factor with its multiplicity
// ----------------------------------------------------------------------------
{
    bignum_g    prime;
    unsigned    exponent;
};


// Maximum number of distinct prime factors we can store.
// For a 256+ bit number, the theoretical max of distinct prime factors
// is ~85 (product of first 85 primes exceeds 2^256), but in practice
// it's far fewer. 128 is very generous.
static const size_t MAX_FACTORS = 128;

struct factor_result
// ----------------------------------------------------------------------------
//   Collect all prime factors
// ----------------------------------------------------------------------------
{
    prime_factor factors[MAX_FACTORS];
    size_t       count;

    factor_result() : count(0) {}

    // Add a prime factor (increments exponent if already present)
    bool add(bignum_r p);
};



// ============================================================================
//
//   Small primes table for trial division
//
// ============================================================================

extern const uint16_t small_primes[];
extern const size_t   NUM_SMALL_PRIMES;


// ============================================================================
//
//   Low-level API
//
// ============================================================================

// Primality test (Miller-Rabin, deterministic for < ~82 bits)
bool     is_probably_prime(bignum_r n);

// Pollard Rho (Brent variant with GCD batching)
bignum_p pollard_rho_brent(bignum_r n);

// Full factorization — fills result with prime factors and exponents
bool     factorize(bignum_g n, factor_result &result);


// ============================================================================
//
//   Calculator commands: IsPrime, Factors, NextPrime, PreviousPrime
//
// ============================================================================

// IsPrime: test if n is a small integer or bignum that is prime.
//   For small integers (integer_p), promotes to bignum internally.
//   Also handles trial division for small values before Miller-Rabin.
//   Returns:  1 = prime,  0 = composite,  -1 = error (invalid input)
int      isprime(bignum_r n);

COMMAND_DECLARE(IsPrime, 1);

// Factors: decompose n into prime factors.
//   Returns true on success, fills result with the factorization.
//   The caller is responsible for converting the result to whatever
//   representation is needed (RPL list, array, stack objects, etc.)
//
//   For n < 2, returns success with an empty result.
//   Negative inputs: the caller should handle the sign and pass |n|.
bool     factors(bignum_r n, factor_result &result);

COMMAND_DECLARE(Factors, 1);

// NextPrime / PreviousPrime: find adjacent primes.
//   Returns the next/previous prime strictly greater/less than n.
//   Returns nullptr on error or if no previous prime exists (n <= 2).
bignum_p next_prime(bignum_r n);
bignum_p prev_prime(bignum_r n);

COMMAND_DECLARE(NextPrime, 1);
COMMAND_DECLARE(PreviousPrime, 1);

#endif // FACTOR_H
