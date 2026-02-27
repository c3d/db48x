// ****************************************************************************
//  factor.cc                                                    DB48X project
// ****************************************************************************
//
//   File Description:
//
//      Implementation of prime factorization and primality testing
//
// ****************************************************************************
//   This software is licensed under the terms outlined in LICENSE.txt
// ****************************************************************************

#include "factor.h"

#include "command.h"
#include "list.h"
#include "settings.h"

#include <algorithm>


// ============================================================================
//
//   factor_result methods
//
// ============================================================================

bool factor_result::add(bignum_r p)
// ----------------------------------------------------------------------------
//   Add a prime factor (increments exponent if already present)
// ----------------------------------------------------------------------------
{
    if (!p)
        return false;
    for (size_t i = 0; i < count; i++)
    {
        if (bignum::compare(factors[i].prime, p) == 0)
        {
            factors[i].exponent++;
            return true;
        }
    }
    if (count >= MAX_FACTORS)
        return false;
    factors[count].prime    = p;
    factors[count].exponent = 1;
    count++;
    return true;
}


// ============================================================================
//
//   Small primes table
//
// ============================================================================

const uint16_t small_primes[] =
{
   2,  3,    5,    7,   11,   13,   17,   19,   23,   29,   31,
      37,   41,   43,   47,   53,   59,   61,   67,   71,   73,
      79,   83,   89,   97,  101,  103,  107,  109,  113,  127,
     131,  137,  139,  149,  151,  157,  163,  167,  173,  179,
     181,  191,  193,  197,  199,  211,  223,  227,  229,  233,
     239,  241,  251,  257,  263,  269,  271,  277,  281,  283,
     293,  307,  311,  313,  317,  331,  337,  347,  349,  353,
     359,  367,  373,  379,  383,  389,  397,  401,  409,  419,
     421,  431,  433,  439,  443,  449,  457,  461,  463,  467,
     479,  487,  491,  499,  503,  509,  521,  523,  541,  547,
     557,  563,  569,  571,  577,  587,  593,  599,  601,  607,
     613,  617,  619,  631,  641,  643,  647,  653,  659,  661,
     673,  677,  683,  691,  701,  709,  719,  727,  733,  739,
     743,  751,  757,  761,  769,  773,  787,  797,  809,  811,
     821,  823,  827,  829,  839,  853,  857,  859,  863,  877,
     881,  883,  887,  907,  911,  919,  929,  937,  941,  947,
     953,  967,  971,  977,  983,  991,  997, 1009, 1013, 1019,
    1021, 1031, 1033, 1039, 1049, 1051, 1061, 1063, 1069, 1087,
    1091, 1093, 1097, 1103, 1109, 1117, 1123, 1129, 1151, 1153,
    1163, 1171, 1181, 1187, 1193, 1201, 1213, 1217, 1223, 1229,
    1231, 1237, 1249, 1259, 1277, 1279, 1283, 1289, 1291, 1297,
    1301, 1303, 1307, 1319, 1321, 1327, 1361, 1367, 1373, 1381,
    1399, 1409, 1423, 1427, 1429, 1433, 1439, 1447, 1451, 1453,
    1459, 1471, 1481, 1483, 1487, 1489, 1493, 1499, 1511, 1523,
    1531, 1543, 1549, 1553, 1559, 1567, 1571, 1579, 1583, 1597,
    1601, 1607, 1609, 1613, 1619, 1621, 1627, 1637, 1657, 1663,
    1667, 1669, 1693, 1697, 1699, 1709, 1721, 1723, 1733, 1741,
    1747, 1753, 1759, 1777, 1783, 1787, 1789, 1801, 1811, 1823,
    1831, 1847, 1861, 1867, 1871, 1873, 1877, 1879, 1889, 1901,
    1907, 1913, 1931, 1933, 1949, 1951, 1973, 1979, 1987, 1993,
    1997, 1999, 2003, 2011, 2017, 2027, 2029, 2039, 2053, 2063,
    2069, 2081, 2083, 2087, 2089, 2099, 2111, 2113, 2129, 2131,
    2137, 2141, 2143, 2153, 2161, 2179, 2203, 2207, 2213, 2221,
    2237, 2239, 2243, 2251, 2267, 2269, 2273, 2281, 2287, 2293,
    2297, 2309, 2311, 2333, 2339, 2341, 2347, 2351, 2357, 2371,
    2377, 2381, 2383, 2389, 2393, 2399, 2411, 2417, 2423, 2437,
    2441, 2447, 2459, 2467, 2473, 2477, 2503, 2521, 2531, 2539,
    2543, 2549, 2551, 2557, 2579, 2591, 2593, 2609, 2617, 2621,
    2633, 2647, 2657, 2659, 2663, 2671, 2677, 2683, 2687, 2689,
    2693, 2699, 2707, 2711, 2713, 2719, 2729, 2731, 2741, 2749,
    2753, 2767, 2777, 2789, 2791, 2797, 2801, 2803, 2819, 2833,
    2837, 2843, 2851, 2857, 2861, 2879, 2887, 2897, 2903, 2909,
    2917, 2927, 2939, 2953, 2957, 2963, 2969, 2971, 2999, 3001,
    3011, 3019, 3023, 3037, 3041, 3049, 3061, 3067, 3079, 3083,
    3089, 3109, 3119, 3121, 3137, 3163, 3167, 3169, 3181, 3187,
    3191, 3203, 3209, 3217, 3221, 3229, 3251, 3253, 3257, 3259,
    3271, 3299, 3301, 3307, 3313, 3319, 3323, 3329, 3331, 3343,
    3347, 3359, 3361, 3371, 3373, 3389, 3391, 3407, 3413, 3433,
    3449, 3457, 3461, 3463, 3467, 3469, 3491, 3499, 3511, 3517,
    3527, 3529, 3533, 3539, 3541, 3547, 3557, 3559, 3571, 3581,
    3583, 3593, 3607, 3613, 3617, 3623, 3631, 3637, 3643, 3659,
    3671, 3673, 3677, 3691, 3697, 3701, 3709, 3719, 3727, 3733,
    3739, 3761, 3767, 3769, 3779, 3793, 3797, 3803, 3821, 3823,
    3833, 3847, 3851, 3853, 3863, 3877, 3881, 3889, 3907, 3911,
    3917, 3919, 3923, 3929, 3931, 3943, 3947, 3967, 3989, 4001,
    4003, 4007, 4013, 4019, 4021, 4027, 4049, 4051, 4057, 4073,
    4079, 4091, 4093, 4099, 4111, 4127, 4129, 4133, 4139, 4153,
    4157, 4159, 4177, 4201, 4211, 4217, 4219, 4229, 4231, 4241,
    4243, 4253, 4259, 4261, 4271, 4273, 4283, 4289, 4297, 4327,
    4337, 4339, 4349, 4357, 4363, 4373, 4391, 4397, 4409, 4421,
    4423, 4441, 4447, 4451, 4457, 4463, 4481, 4483, 4493, 4507,
    4513, 4517, 4519, 4523, 4547, 4549, 4561, 4567, 4583, 4591,
    4597, 4603, 4621, 4637, 4639, 4643, 4649, 4651, 4657, 4663,
    4673, 4679, 4691, 4703, 4721, 4723, 4729, 4733, 4751, 4759,
    4783, 4787, 4789, 4793, 4799, 4801, 4813, 4817, 4831, 4861,
    4871, 4877, 4889, 4903, 4909, 4919, 4931, 4933, 4937, 4943,
    4951, 4957, 4967, 4969, 4973, 4987, 4993, 4999, 5003, 5009,
    5011, 5021, 5023, 5039, 5051, 5059, 5077, 5081, 5087, 5099,
    5101, 5107, 5113, 5119, 5147, 5153, 5167, 5171, 5179, 5189,
    5197, 5209, 5227, 5231, 5233, 5237, 5261, 5273, 5279, 5281,
    5297, 5303, 5309, 5323, 5333, 5347, 5351, 5381, 5387, 5393,
    5399, 5407, 5413, 5417, 5419, 5431, 5437, 5441, 5443, 5449,
    5471, 5477, 5479, 5483, 5501, 5503, 5507, 5519, 5521, 5527,
    5531, 5557, 5563, 5569, 5573, 5581, 5591, 5623, 5639, 5641,
    5647, 5651, 5653, 5657, 5659, 5669, 5683, 5689, 5693, 5701,
    5711, 5717, 5737, 5741, 5743, 5749, 5779, 5783, 5791, 5801,
    5807, 5813, 5821, 5827, 5839, 5843, 5849, 5851, 5857, 5861,
    5867, 5869, 5879, 5881, 5897, 5903, 5923, 5927, 5939, 5953,
    5981, 5987, 6007, 6011, 6029, 6037, 6043, 6047, 6053, 6067,
    6073, 6079, 6089, 6091, 6101, 6113, 6121, 6131, 6133, 6143,
    6151, 6163, 6173, 6197, 6199, 6203, 6211, 6217, 6221, 6229,
    6247, 6257, 6263, 6269, 6271, 6277, 6287, 6299, 6301, 6311,
    6317, 6323, 6329, 6337, 6343, 6353, 6359, 6361, 6367, 6373,
    6379, 6389, 6397, 6421, 6427, 6449, 6451, 6469, 6473, 6481,
    6491, 6521, 6529, 6547, 6551, 6553, 6563, 6569, 6571, 6577,
    6581, 6599, 6607, 6619, 6637, 6653, 6659, 6661, 6673, 6679,
    6689, 6691, 6701, 6703, 6709, 6719, 6733, 6737, 6761, 6763,
    6779, 6781, 6791, 6793, 6803, 6823, 6827, 6829, 6833, 6841,
    6857, 6863, 6869, 6871, 6883, 6899, 6907, 6911, 6917, 6947,
    6949, 6959, 6961, 6967, 6971, 6977, 6983, 6991, 6997, 7001,
    7013, 7019, 7027, 7039, 7043, 7057, 7069, 7079, 7103, 7109,
    7121, 7127, 7129, 7151, 7159, 7177, 7187, 7193, 7207, 7211,
    7213, 7219, 7229, 7237, 7243, 7247, 7253, 7283, 7297, 7307,
    7309, 7321, 7331, 7333, 7349, 7351, 7369, 7393, 7411, 7417,
    7433, 7451, 7457, 7459, 7477, 7481, 7487, 7489, 7499, 7507,
    7517, 7523, 7529, 7537, 7541, 7547, 7549, 7559, 7561, 7573,
    7577, 7583, 7589, 7591, 7603, 7607, 7621, 7639, 7643, 7649,
    7669, 7673, 7681, 7687, 7691, 7699, 7703, 7717, 7723, 7727,
    7741, 7753, 7757, 7759, 7789, 7793, 7817, 7823, 7829, 7841,
    7853, 7867, 7873, 7877, 7879, 7883, 7901, 7907, 7919, 7927,
    7933, 7937, 7949, 7951, 7963, 7993, 8009, 8011, 8017, 8039,
    8053, 8059, 8069, 8081, 8087, 8089, 8093, 8101, 8111, 8117,
    8123, 8147, 8161, 8167, 8171, 8179, 8191, 8209, 8219, 8221,
    8231, 8233, 8237, 8243, 8263, 8269, 8273, 8287, 8291, 8293,
    8297, 8311, 8317, 8329, 8353, 8363, 8369, 8377, 8387, 8389,
    8419, 8423, 8429, 8431, 8443, 8447, 8461, 8467, 8501, 8513,
    8521, 8527, 8537, 8539, 8543, 8563, 8573, 8581, 8597, 8599,
    8609, 8623, 8627, 8629, 8641, 8647, 8663, 8669, 8677, 8681,
    8689, 8693, 8699, 8707, 8713, 8719, 8731, 8737, 8741, 8747,
    8753, 8761, 8779, 8783, 8803, 8807, 8819, 8821, 8831, 8837,
    8839, 8849, 8861, 8863, 8867, 8887, 8893, 8923, 8929, 8933,
    8941, 8951, 8963, 8969, 8971, 8999, 9001, 9007, 9011, 9013,
    9029, 9041, 9043, 9049, 9059, 9067, 9091, 9103, 9109, 9127,
    9133, 9137, 9151, 9157, 9161, 9173, 9181, 9187, 9199, 9203,
    9209, 9221, 9227, 9239, 9241, 9257, 9277, 9281, 9283, 9293,
    9311, 9319, 9323, 9337, 9341, 9343, 9349, 9371, 9377, 9391,
    9397, 9403, 9413, 9419, 9421, 9431, 9433, 9437, 9439, 9461,
    9463, 9467, 9473, 9479, 9491, 9497, 9511, 9521, 9533, 9539,
    9547, 9551, 9587, 9601, 9613, 9619, 9623, 9629, 9631, 9643,
    9649, 9661, 9677, 9679, 9689, 9697, 9719, 9721, 9733, 9739,
    9743, 9749, 9767, 9769, 9781, 9787, 9791, 9803, 9811, 9817,
    9829, 9833, 9839, 9851, 9857, 9859, 9871, 9883, 9887, 9901,
    9907, 9923, 9929, 9931, 9941, 9949, 9967, 9973,
};

const size_t NUM_SMALL_PRIMES = sizeof(small_primes) / sizeof(small_primes[0]);



// ============================================================================
//
//   Miller-Rabin primality test
//
// ============================================================================

static bool miller_rabin_witness(bignum_r a, bignum_r d, unsigned r, bignum_r n)
// ----------------------------------------------------------------------------
//   Test a single Miller-Rabin witness 'a' against n, where n-1 = d * 2^r
// ----------------------------------------------------------------------------
{
    bignum_g one  = bignum::make(1);
    bignum_g n_m1 = n - one;
    if (!one || !n_m1)
        return false;

    bignum_g x = bignum::powmod(a, d, n);
    if (!x)
        return false;

    if (bignum::compare(x, one) == 0 || bignum::compare(x, n_m1) == 0)
        return true;

    for (unsigned i = 0; i < r - 1; i++)
    {
        x = bignum::mulmod(x, x, n);
        if (!x)
            return false;

        if (bignum::compare(x, n_m1) == 0)
            return true;

        if (bignum::compare(x, one) == 0)
            return false;
    }
    return false;
}


bool is_probably_prime(bignum_r n)
// ----------------------------------------------------------------------------
//   Miller-Rabin primality test
// ----------------------------------------------------------------------------
//   Deterministic for n < 3.317e24 (~82 bits) with witnesses below.
//   Probabilistic with error < 4^{-12} ~ 6e-8 for larger n.
{
    bignum_g one = bignum::make(1);
    bignum_g two = bignum::make(2);
    if (!one || !two || !n)
        return false;

    // n <= 1 → not prime
    if (n->is_zero() || n->is_one())
        return false;

    // n == 2 or n == 3 → prime
    if (bignum::compare(n, two) == 0)
        return true;
    bignum_g three = bignum::make(3);
    if (bignum::compare(n, three) == 0)
        return true;

    // Even → composite
    {
        size_t sz = 0;
        byte_p raw = n->value(&sz);
        if (sz > 0 && !(raw[0] & 1))
            return false;
    }

    // Write n - 1 = d * 2^r
    bignum_g d = n - one;
    if (!d)
        return false;
    unsigned r = 0;
    while (true)
    {
        size_t sz = 0;
        byte_p raw = d->value(&sz);
        if (sz == 0)
            break;
        if (raw[0] & 1)
            break;
        d = d >> 1u;
        if (!d)
            return false;
        r++;
    }

    static const unsigned witnesses[] = {
        2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37
    };
    static const size_t num_witnesses =
        sizeof(witnesses) / sizeof(witnesses[0]);

    for (size_t i = 0; i < num_witnesses; i++)
    {
        bignum_g a = bignum::make(witnesses[i]);
        if (!a)
            return false;
        if (bignum::compare(a, n) >= 0)
            continue;
        if (!miller_rabin_witness(a, d, r, n))
            return false;
    }

    return true;
}


// ============================================================================
//
//   Pollard's Rho (Brent's variant with GCD batching)
//
// ============================================================================

bignum_p pollard_rho_brent(bignum_r n)
// ----------------------------------------------------------------------------
//   Find a non-trivial factor of n
// ----------------------------------------------------------------------------
{
    static const unsigned BATCH_SIZE = 128;

    bignum_g one = bignum::make(1);
    bignum_g two = bignum::make(2);
    if (!one || !two)
        return nullptr;

    for (unsigned c_val = 1; c_val <= 20; c_val++)
    {
        bignum_g c = bignum::make(c_val);
        if (!c)
            return nullptr;

        bignum_g x = two;
        bignum_g y = two;
        bignum_g d = one;

        unsigned lam   = 1;
        unsigned iters = 0;

        while (bignum::compare(d, one) == 0)
        {
            x = y;
            for (unsigned i = 0; i < lam; i++)
            {
                y = bignum::mulmod(y, y, n);
                if (!y)
                    return nullptr;
                y = bignum::addmod(y, c, n);
                if (!y)
                    return nullptr;
            }

            unsigned k = 0;
            while (k < lam && bignum::compare(d, one) == 0)
            {
                bignum_g ys    = y;
                bignum_g accum = one;

                unsigned batch = std::min(BATCH_SIZE, lam - k);
                for (unsigned j = 0; j < batch; j++)
                {
                    y = bignum::mulmod(y, y, n);
                    if (!y)
                        return nullptr;
                    y = bignum::addmod(y, c, n);
                    if (!y)
                        return nullptr;

                    bignum_g diff = bignum::abs_diff(x, y);
                    if (!diff || diff->is_zero())
                        continue;
                    accum = bignum::mulmod(accum, diff, n);
                    if (!accum)
                        return nullptr;
                }

                d = bignum::gcd(accum, n);
                if (!d)
                    return nullptr;

                k += batch;
                iters += batch;

                if (bignum::compare(d, n) == 0)
                {
                    d = one;
                    y = ys;
                    for (unsigned j = 0; j < batch; j++)
                    {
                        y = bignum::mulmod(y, y, n);
                        if (!y)
                            return nullptr;
                        y = bignum::addmod(y, c, n);
                        if (!y)
                            return nullptr;

                        bignum_g diff = bignum::abs_diff(x, y);
                        if (!diff || diff->is_zero())
                            continue;
                        d = bignum::gcd(diff, n);
                        if (!d)
                            return nullptr;
                        if (bignum::compare(d, one) != 0)
                            break;
                    }
                    if (bignum::compare(d, n) == 0)
                    {
                        d = one;
                        break;
                    }
                }
            }

            if (iters > Settings.MaxFactorIterations())
                break;

            lam *= 2;
        }

        if (bignum::compare(d, one) != 0 && bignum::compare(d, n) != 0)
            return d;
    }

    return nullptr;
}


// ============================================================================
//
//   Core factorization
//
// ============================================================================

static bool extract_small_factor(bignum_g &n, bignum_r p,
                                 factor_result &result)
// ----------------------------------------------------------------------------
//   Divide out all copies of prime p from n, record in result
// ----------------------------------------------------------------------------
{
    bignum_g r = n % p;
    if (!r)
        return false;

    if (!r->is_zero())
        return true;

    if (!result.add(p))
        return false;

    n = n / p;
    if (!n)
        return false;

    while (true)
    {
        r = n % p;
        if (!r)
            return false;
        if (!r->is_zero())
            break;
        result.factors[result.count - 1].exponent++;
        n = n / p;
        if (!n)
            return false;
    }

    return true;
}


static bool factorize_recursive(bignum_r n, factor_result &result)
// ----------------------------------------------------------------------------
//   Recursively factorize n, adding prime factors to result
// ----------------------------------------------------------------------------
{
    if (!n || n->is_zero() || n->is_one())
        return true;

    if (is_probably_prime(n))
        return result.add(n);

    bignum_g factor = pollard_rho_brent(n);
    if (!factor)
        return false;

    bignum_g cofactor = n / factor;
    if (!cofactor)
        return false;

    if (!factorize_recursive(factor, result))
        return false;
    return factorize_recursive(cofactor, result);
}


bool factorize(bignum_g n, factor_result &result)
// ----------------------------------------------------------------------------
//   Full factorization of a bignum n into prime factors
// ----------------------------------------------------------------------------
{
    if (!n || n->is_zero())
        return false;

    if (n->is_one())
        return true;

    // Phase 1: trial division by small primes
    for (size_t i = 0; i < NUM_SMALL_PRIMES; i++)
    {
        bignum_g p = bignum::make(small_primes[i]);
        if (!p)
            return false;

        if (!extract_small_factor(n, p, result))
            return false;

        if (n->is_one())
            return true;

        bignum_g p2 = p * p;
        if (p2 && bignum::compare(p2, n) > 0)
            return result.add(n);
    }

    // Phase 2: Miller-Rabin + Pollard Rho
    return factorize_recursive(n, result);
}



// ============================================================================
//
//   High-level: IsPrime
//
// ============================================================================

int isprime(bignum_r n)
// ----------------------------------------------------------------------------
//   Test if n is prime
// ----------------------------------------------------------------------------
//   Returns:  1 = prime,  0 = composite,  -1 = error
//
//   Optimized path: checks small primes by trial division first,
//   which avoids the cost of Miller-Rabin for the common case of
//   numbers with small factors.
{
    if (!n)
        return -1;

    // 0 and 1 are not prime
    if (n->is_zero() || n->is_one())
        return 0;

    // Quick check: is n itself a small prime or divisible by one?
    bignum_g two = bignum::make(2);
    if (!two)
        return -1;
    if (bignum::compare(n, two) == 0)
        return 1;

    // Even → composite
    {
        size_t sz = 0;
        byte_p raw = n->value(&sz);
        if (sz > 0 && !(raw[0] & 1))
            return 0;
    }

    // Trial division by small odd primes
    for (size_t i = 0; i < NUM_SMALL_PRIMES; i++)
    {
        bignum_g p = bignum::make(small_primes[i]);
        if (!p)
            return -1;

        // If n == p, it's prime
        if (bignum::compare(n, p) == 0)
            return 1;

        // If p^2 > n and we haven't found a divisor, n is prime
        bignum_g p2 = p * p;
        if (p2 && bignum::compare(p2, n) > 0)
            return 1;

        // If p divides n, composite
        bignum_g r = n % p;
        if (!r)
            return -1;
        if (r->is_zero())
            return 0;
    }

    // Fall back to Miller-Rabin for large n
    return is_probably_prime(n) ? 1 : 0;
}



// ============================================================================
//
//   NextPrime / PreviousPrime
//
// ============================================================================

static bignum_p adjacent_prime(bignum_r n, bool next)
// ----------------------------------------------------------------------------
//   Find the next (next=true) or previous (next=false) prime relative to n
// ----------------------------------------------------------------------------
{
    if (!n)
        return nullptr;
    bignum_g one = bignum::make(1);
    bignum_g two = bignum::make(2);
    if (!one || !two)
        return nullptr;

    // candidate = n + 1  or  n - 1
    bignum_g cand = next ? (n + one) : (n - one);
    if (!cand)
        return nullptr;

    // No prime <= 1
    if (!next && (cand->is_zero() || cand->is_one()))
        return nullptr;

    // Handle the only even prime: 2
    if (next)
    {
        // If candidate == 2, it's prime
        if (bignum::compare(cand, two) == 0)
            return cand;

        // If candidate is even, advance to the next odd number
        {
            size_t sz  = 0;
            byte_p raw = cand->value(&sz);
            if (sz > 0 && !(raw[0] & 1))
            {
                cand = cand + one;
                if (!cand)
                    return nullptr;
            }
        }
    }
    else
    {
        // Going backward: if candidate == 2 it's prime, return it
        if (bignum::compare(cand, two) == 0)
            return cand;

        // If candidate is even, step back to the previous odd number
        {
            size_t sz  = 0;
            byte_p raw = cand->value(&sz);
            if (sz > 0 && !(raw[0] & 1))
            {
                cand = cand - one;
                if (!cand || cand->is_one())
                    return nullptr;
                // cand == 2 after stepping back?
                if (bignum::compare(cand, two) == 0)
                    return cand;
            }
        }
    }

    // Walk through odd numbers until we find a prime
    static const unsigned MAX_SEARCH = 1 << 20;
    for (unsigned iter = 0; iter < MAX_SEARCH; iter++)
    {
        if (isprime(cand) == 1)
            return cand;

        // Step by 2 (stay in odd numbers)
        cand = next ? (cand + two) : (cand - two);
        if (!cand)
            return nullptr;

        // Going backward: stop if we've gone below 2
        if (!next && (cand->is_zero() || cand->is_one()))
            return nullptr;
    }

    return nullptr; // search limit reached
}


static bignum_p factorable_value_from_stack()
// ----------------------------------------------------------------------------
//   Check if we have a valid input for prime checking
// ----------------------------------------------------------------------------
{
    object_p xo = object::strip(rt.stack(0));
    if (!xo)
        return nullptr;

    if (!object::is_integer(xo->type()))
    {
        rt.type_error();
        return nullptr;
    }

    bignum_p xi = bignum::promote(xo);
    if (!xi)
        return nullptr;
    if (xi->more_bits_than(Settings.MaxFactorsBits()))
    {
        rt.number_too_big_error();
        return nullptr;
    }
    return xi;
}


static object::result adjacent_prime_command(bool next)
// ----------------------------------------------------------------------------
//   Command for adjacent prime number
// ----------------------------------------------------------------------------
{
    bignum_g xi = factorable_value_from_stack();
    if (xi)
    {
        xi = adjacent_prime(xi, next);
        if (!xi)
            rt.value_error();
        else if (rt.top(+xi))
            return object::OK;
    }
    return object::ERROR;
}


COMMAND_BODY(NextPrime)
// ----------------------------------------------------------------------------
//   Returns the next prime after the given number
// ----------------------------------------------------------------------------
{
    return adjacent_prime_command(true);
}


COMMAND_BODY(PreviousPrime)
// ----------------------------------------------------------------------------
//  Return the previous prime before current number
// ----------------------------------------------------------------------------
{
    return adjacent_prime_command(false);
}


COMMAND_BODY(IsPrime)
// ----------------------------------------------------------------------------
//   Test if a number is prime or not
// ----------------------------------------------------------------------------
{
    bignum_g xi = factorable_value_from_stack();
    if (xi)
    {
        int res = isprime(xi);
        if (res >= 0)
        {
            object_p r = command::static_object(res ? ID_True : ID_False);
            if (r && rt.top(r))
                return OK;
        }
    }
    return ERROR;
}



// ============================================================================
//
//   High-level: Factors
//
// ============================================================================

bool factors(bignum_r n, factor_result &result)
// ----------------------------------------------------------------------------
//   Decompose n into prime factors
// ----------------------------------------------------------------------------
//   For n < 2, returns true with empty result (no prime factors).
//   The caller should handle negative n by passing |n| and tracking sign.
//
//   On success, result.factors[] contains the distinct primes in
//   ascending order (as produced by trial division then Pollard Rho),
//   each with its exponent.
//
//   Usage example:
//     bignum_g n = bignum::make(12345678901234567LL);
//     factor_result res;
//     if (factors(n, res))
//         for (size_t i = 0; i < res.count; i++)
//             printf("%s ^ %u\n", ..., res.factors[i].exponent);
{
    if (!n)
        return false;

    // n < 2: no prime factors
    if (n->is_zero() || n->is_one())
        return true;

    // Use is_probably_prime as a fast path for prime inputs
    // This avoids running the full trial division + Rho machinery
    // on what is already a prime number
    if (is_probably_prime(n))
        return result.add(n);

    return factorize(n, result);
}


COMMAND_BODY(Factors)
// ----------------------------------------------------------------------------
//  Factorize the input
// ----------------------------------------------------------------------------
{
    bignum_g xi = factorable_value_from_stack();
    if (!xi)
        return ERROR;

    factor_result result;
    if (!factors(xi, result))
    {
        return ERROR;
    }

    // Build list of results { p1 e1 p2 e2 ... }
    scribble scr;
    for (size_t i = 0; i < result.count; i++)
    {
        bignum_p p = result.factors[i].prime;
        if (!p || !rt.append(p))
            return ERROR;

        bignum_p e = bignum::make(result.factors[i].exponent);
        if (!e || !rt.append(e))
            return ERROR;
    }

    list_g lst = list::make(scr.scratch(), scr.growth());
    if (!lst || !rt.top(lst))
        return ERROR;

    return OK;
}
