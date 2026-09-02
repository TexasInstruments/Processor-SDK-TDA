
#ifndef TIDL_CUDA_BFLOAT16_H_
#define TIDL_CUDA_BFLOAT16_H_

#ifdef BUILD_WITH_CUDA

#ifndef TIDL_BFLOAT16_H
#define TIDL_BFLOAT16_H

#include <stdint.h>
#include <limits>

#ifdef __cplusplus

// -------------------------------------------------------------------------
// Self-contained BFloat16 implementation for CUDA.
// Uses __float_as_uint / __uint_as_float CUDA intrinsics (available as
// __host__ __device__ since CUDA 10) instead of memcpy / bit_cast, removing
// any dependency on floating_point_utils.h or other host-only headers.
// -------------------------------------------------------------------------

namespace floating_point {

struct alignas(2) bfloat16_t {
    static constexpr uint16_t SIGN_MASK         = 0x8000U;
    static constexpr uint16_t EXPONENT_MASK     = 0x7F80U;
    static constexpr uint16_t MANTISSA_MASK     = 0x007FU;
    static constexpr uint16_t NON_SIGN_BITS     = 0x7FFFU;
    static constexpr uint16_t NAN_BITS          = 0x7FC0U;
    static constexpr uint16_t POS_INFINITY_BITS = 0x7F80U;
    static constexpr uint16_t NEG_INFINITY_BITS = 0xFF80U;

    uint16_t x;

    bfloat16_t() = default;

    struct from_bits_t {};
    __host__ __device__ static constexpr from_bits_t from_bits() { return from_bits_t(); }
    __host__ __device__ constexpr bfloat16_t(uint16_t bits, from_bits_t) : x(bits) {}

    __host__ __device__ inline bfloat16_t(float value);
    __host__ __device__ inline operator float() const;

    __host__ __device__ inline bool is_nan()      const { return ((x & EXPONENT_MASK) == EXPONENT_MASK) && ((x & MANTISSA_MASK) != 0); }
    __host__ __device__ inline bool is_infinity() const { return (x & NON_SIGN_BITS) == POS_INFINITY_BITS; }
    __host__ __device__ inline bool is_zero()     const { return (x & NON_SIGN_BITS) == 0; }
    __host__ __device__ inline bool is_finite()   const { return (x & EXPONENT_MASK) != EXPONENT_MASK; }
    __host__ __device__ inline bool is_negative() const { return (x & SIGN_MASK) != 0; }

    __host__ __device__ inline static bfloat16_t zero()              { return bfloat16_t(0x0000,             from_bits()); }
    __host__ __device__ inline static bfloat16_t negative_zero()     { return bfloat16_t(SIGN_MASK,          from_bits()); }
    __host__ __device__ inline static bfloat16_t positive_infinity() { return bfloat16_t(POS_INFINITY_BITS,  from_bits()); }
    __host__ __device__ inline static bfloat16_t negative_infinity() { return bfloat16_t(NEG_INFINITY_BITS,  from_bits()); }
    __host__ __device__ inline static bfloat16_t nan()               { return bfloat16_t(NAN_BITS,           from_bits()); }
};

namespace bit_ops {
    // Portable bit-reinterpret helpers (memcpy is valid in both host and device code;
    // the compiler/NVCC will optimize this to a register move in device code).
    __host__ __device__ inline uint32_t f32_to_u32(float f) {
        uint32_t u; __builtin_memcpy(&u, &f, sizeof(u)); return u;
    }
    __host__ __device__ inline float u32_to_f32(uint32_t u) {
        float f; __builtin_memcpy(&f, &u, sizeof(f)); return f;
    }

    // BF16 bits → float: shift the 16-bit pattern into the upper half of a float32.
    __host__ __device__ inline float f32_from_bits(uint16_t src) {
        return u32_to_f32(static_cast<uint32_t>(src) << 16);
    }
    // float → BF16 bits: take the upper 16 bits of the float32 representation.
    __host__ __device__ inline uint16_t bits_from_f32(float src) {
        return static_cast<uint16_t>(f32_to_u32(src) >> 16);
    }
    __host__ __device__ inline uint16_t get_sign_bit(uint16_t bits) { return (bits & bfloat16_t::SIGN_MASK)     >> 15; }
    __host__ __device__ inline uint16_t get_exponent(uint16_t bits) { return (bits & bfloat16_t::EXPONENT_MASK) >>  7; }
    __host__ __device__ inline uint16_t get_mantissa(uint16_t bits) { return  bits & bfloat16_t::MANTISSA_MASK;        }

    // Round float to nearest-even BF16.
    __host__ __device__ inline uint16_t round_to_nearest_even(float src) {
        const uint32_t U32      = f32_to_u32(src);
        const uint32_t sign     = U32 & 0x80000000U;
        const uint32_t exponent = U32 & 0x7F800000U;
        const uint32_t mantissa = U32 & 0x007FFFFFU;
        if ((exponent == 0x7F800000U) && (mantissa != 0))
            return static_cast<uint16_t>((sign >> 16) | 0x7FC0U);
        if (exponent == 0x7F800000U)
            return static_cast<uint16_t>((sign >> 16) | 0x7F80U);
        if (exponent < 0x00800000U)
            return static_cast<uint16_t>(sign >> 16);
        if (exponent > 0x7F000000U)
            return static_cast<uint16_t>((sign >> 16) | 0x7F80U);
        const uint32_t rounding_bias = ((U32 >> 16) & 1U) + 0x7FFFU;
        return static_cast<uint16_t>((U32 + rounding_bias) >> 16);
    }
} // namespace bit_ops

// Constructor and float conversion operator
__host__ __device__ inline bfloat16_t::bfloat16_t(float value)
    : x(bit_ops::round_to_nearest_even(value)) {}

__host__ __device__ inline bfloat16_t::operator float() const {
    return bit_ops::f32_from_bits(x);
}

namespace bf16_c7x {

    // -----------------------------------------------------------------------
    // NaN canonicalization helpers (match C7604 hardware behaviour)
    // -----------------------------------------------------------------------
    // ADD/SUB: any NaN input → canonical +qNaN (0x7FC0), sign always cleared.
    // MUL/DIV/ABS/MIN/MAX:
    //   sNaN (quiet bit == 0) → +qNaN (0x7FC0), sign cleared
    //   qNaN (quiet bit == 1) → qNaN with sign preserved, payload cleared
    __host__ __device__ inline bfloat16_t canonicalize_nan(const bfloat16_t& a) {
        bool is_quiet = ((a.x >> 6) & 1u) != 0u;
        if (is_quiet) {
            // qNaN: preserve sign, clear payload
            return bfloat16_t(static_cast<uint16_t>((a.x & bfloat16_t::SIGN_MASK) | bfloat16_t::NAN_BITS),
                              bfloat16_t::from_bits());
        }
        // sNaN: always returns canonical +qNaN
        return bfloat16_t::nan();
    }

    struct bf16_parts { uint16_t sign, exp; uint32_t sig; };

    __host__ __device__ inline bf16_parts bf16_unpack(const bfloat16_t& v) {
        return { bit_ops::get_sign_bit(v.x),
                 bit_ops::get_exponent(v.x),
                 static_cast<uint32_t>(0x80u | bit_ops::get_mantissa(v.x)) << 7 };
    }

    // Plain struct replaces std::pair — not reliably usable in device code.
    struct bf16_align_t { uint32_t sig; bool sticky; };

    __host__ __device__ inline bf16_align_t bf16_align_right(uint32_t sig, uint16_t shift) {
        if (shift ==  0) return {sig, false};
        if (shift >= 15) return {0u, sig != 0u};
        bool sticky = (sig & ((1u << shift) - 1u)) != 0u;
        return {sig >> shift, sticky};
    }

    __host__ __device__ inline uint32_t bf16_round_rne(uint32_t sig, bool extra_sticky) {
        bool guard  = (sig >> 6) & 1U;
        bool sticky = extra_sticky || ((sig & 0x3FU) != 0U);
        bool lsb    = (sig >> 7) & 1U;
        if (guard && (sticky || lsb)) sig += 0x80U;
        return sig;
    }

    __host__ __device__ inline bfloat16_t bf16_pack(uint16_t sign, uint16_t exp, uint32_t sig) {
        uint16_t bits = static_cast<uint16_t>(sign << 15)
                      | static_cast<uint16_t>(exp  <<  7)
                      | static_cast<uint16_t>((sig >> 7) & 0x7FU);
        return bfloat16_t(bits, bfloat16_t::from_bits());
    }

    __host__ __device__ inline bfloat16_t operator+(const bfloat16_t& a, const bfloat16_t& b) {
        // ADD: any NaN input → canonical +qNaN (0x7FC0), sign always cleared
        if (a.is_nan() || b.is_nan()) return bfloat16_t::nan();
        if (a.is_zero()) {
            if (b.is_zero())
                return (a.is_negative() && b.is_negative()) ? bfloat16_t::negative_zero() : bfloat16_t::zero();
            return b;
        }
        if (b.is_zero()) return a;
        if (a.is_infinity() || b.is_infinity()) {
            if (a.is_infinity() && b.is_infinity()) {
                bool same_sign = (a.x & bfloat16_t::SIGN_MASK) == (b.x & bfloat16_t::SIGN_MASK);
                return same_sign ? a : bfloat16_t::nan();
            }
            return a.is_infinity() ? a : b;
        }

        bf16_parts pa = bf16_unpack(a);
        bf16_parts pb = bf16_unpack(b);

        // Order so pa holds the larger-magnitude operand.
        // When exponents differ, ordering by exponent guarantees pa.sig >= aligned pb.sig.
        // When exponents are equal and signs differ (effective subtraction), also compare
        // significands to prevent unsigned underflow in pa.sig - sig_b.
        if (pa.exp < pb.exp) {
            bf16_parts tmp = pa; pa = pb; pb = tmp;
        } else if (pa.exp == pb.exp && pa.sign != pb.sign && pa.sig < pb.sig) {
            bf16_parts tmp = pa; pa = pb; pb = tmp;
        }

        bf16_align_t aligned = bf16_align_right(pb.sig, static_cast<uint16_t>(pa.exp - pb.exp));
        uint32_t sig_b  = aligned.sig;
        bool     sticky = aligned.sticky;

        uint32_t m_res;
        uint16_t s_res;
        if (pa.sign == pb.sign) {
            m_res = pa.sig + sig_b;
            s_res = pa.sign;
        } else {
            m_res = pa.sig - sig_b;   // safe: pa.sig >= sig_b after ordering above
            s_res = pa.sign;
            if (m_res == 0) return bfloat16_t::zero();
        }

        uint16_t exp_res = pa.exp;
        if (pa.sign == pb.sign && m_res >= 0x8000u) {
            sticky   |= (m_res & 1u) != 0u;
            m_res   >>= 1;
            exp_res++;
        } else if (pa.sign != pb.sign) {
            while (m_res < 0x4000u && exp_res > 0) { m_res <<= 1; exp_res--; }
            if (m_res < 0x4000u) return s_res ? bfloat16_t::negative_zero() : bfloat16_t::zero();
        }

        m_res = bf16_round_rne(m_res, sticky);
        if (m_res >= 0x8000u) { m_res >>= 1; exp_res++; }
        if (exp_res >= 0xFFu)
            return s_res ? bfloat16_t::negative_infinity() : bfloat16_t::positive_infinity();
        return bf16_pack(s_res, exp_res, m_res);
    }

    __host__ __device__ inline bfloat16_t operator-(const bfloat16_t& a, const bfloat16_t& b) {
        bfloat16_t neg_b(static_cast<uint16_t>(b.x ^ bfloat16_t::SIGN_MASK), bfloat16_t::from_bits());
        return operator+(a, neg_b);
    }

    __host__ __device__ inline bfloat16_t operator*(const bfloat16_t& a, const bfloat16_t& b) {
        // MUL: sNaN → +qNaN; qNaN → preserve sign (hardware VMPYBF rule)
        if (a.is_nan()) return canonicalize_nan(a);
        if (b.is_nan()) return canonicalize_nan(b);
        uint16_t s_res = static_cast<uint16_t>(bit_ops::get_sign_bit(a.x) ^ bit_ops::get_sign_bit(b.x));
        if (a.is_infinity() || b.is_infinity()) {
            // Inf * 0 → NaN whose sign = sign(a) XOR sign(b)
            if (a.is_zero() || b.is_zero())
                return bfloat16_t(static_cast<uint16_t>(bfloat16_t::NAN_BITS | (s_res << 15u)),
                                  bfloat16_t::from_bits());
            return s_res ? bfloat16_t::negative_infinity() : bfloat16_t::positive_infinity();
        }
        if (a.is_zero() || b.is_zero())
            return bfloat16_t(static_cast<uint16_t>(s_res << 15), bfloat16_t::from_bits());
        uint32_t ma = 0x80u | bit_ops::get_mantissa(a.x);
        uint32_t mb = 0x80u | bit_ops::get_mantissa(b.x);
        int32_t  exp_res = static_cast<int32_t>(bit_ops::get_exponent(a.x))
                         + static_cast<int32_t>(bit_ops::get_exponent(b.x)) - 127;
        uint32_t prod = ma * mb;
        bool sticky_mul = false;
        if (prod >= 0x8000u) { sticky_mul = (prod & 1u) != 0u; prod >>= 1; exp_res++; }
        prod = bf16_round_rne(prod, sticky_mul);
        if (prod >= 0x8000u) { prod >>= 1; exp_res++; }
        if (exp_res >= 0xFF) return s_res ? bfloat16_t::negative_infinity() : bfloat16_t::positive_infinity();
        if (exp_res <= 0)    return bfloat16_t(static_cast<uint16_t>(s_res << 15), bfloat16_t::from_bits());
        return bf16_pack(s_res, static_cast<uint16_t>(exp_res), prod);
    }

    __host__ __device__ inline bfloat16_t operator/(const bfloat16_t& a, const bfloat16_t& b) {
        // DIV: sNaN → +qNaN; qNaN → preserve sign
        if (a.is_nan()) return canonicalize_nan(a);
        if (b.is_nan()) return canonicalize_nan(b);
        uint16_t s_res = static_cast<uint16_t>(bit_ops::get_sign_bit(a.x) ^ bit_ops::get_sign_bit(b.x));
        if (a.is_infinity() && b.is_infinity()) return bfloat16_t::nan();
        if (a.is_zero()     && b.is_zero())     return bfloat16_t::nan();
        if (b.is_zero())             return s_res ? bfloat16_t::negative_infinity() : bfloat16_t::positive_infinity();
        if (a.is_zero() || b.is_infinity()) return bfloat16_t(static_cast<uint16_t>(s_res << 15), bfloat16_t::from_bits());
        if (a.is_infinity()) return s_res ? bfloat16_t::negative_infinity() : bfloat16_t::positive_infinity();
        uint32_t ma = 0x80u | bit_ops::get_mantissa(a.x);
        uint32_t mb = 0x80u | bit_ops::get_mantissa(b.x);
        int32_t  exp_res = static_cast<int32_t>(bit_ops::get_exponent(a.x))
                         - static_cast<int32_t>(bit_ops::get_exponent(b.x)) + 127;
        uint32_t numer = ma << 14;
        uint32_t quot  = numer / mb;
        bool     sticky = (numer % mb) != 0u;
        if (quot < 0x4000u) { quot <<= 1; exp_res--; }
        quot = bf16_round_rne(quot, sticky);
        if (quot >= 0x8000u) { quot >>= 1; exp_res++; }
        if (exp_res >= 0xFF) return s_res ? bfloat16_t::negative_infinity() : bfloat16_t::positive_infinity();
        if (exp_res <= 0)    return bfloat16_t(static_cast<uint16_t>(s_res << 15), bfloat16_t::from_bits());
        return bf16_pack(s_res, static_cast<uint16_t>(exp_res), quot);
    }

    __host__ __device__ inline bfloat16_t operator-(const bfloat16_t& a) {
        // NEG: canonicalize NaN first (sNaN→+qNaN, qNaN→clear payload/preserve sign),
        // then flip sign.  e.g. sNaN(any sign) → +qNaN(0x7FC0) → -qNaN(0xFFC0).
        if (a.is_nan()) {
            bfloat16_t canon = canonicalize_nan(a);
            return bfloat16_t(static_cast<uint16_t>(canon.x ^ bfloat16_t::SIGN_MASK),
                              bfloat16_t::from_bits());
        }
        return bfloat16_t(static_cast<uint16_t>(a.x ^ bfloat16_t::SIGN_MASK), bfloat16_t::from_bits());
    }

    __host__ __device__ inline bfloat16_t& operator+=(bfloat16_t& a, const bfloat16_t& b) { a = operator+(a, b); return a; }
    __host__ __device__ inline bfloat16_t& operator-=(bfloat16_t& a, const bfloat16_t& b) { a = operator-(a, b); return a; }
    __host__ __device__ inline bfloat16_t& operator*=(bfloat16_t& a, const bfloat16_t& b) { a = operator*(a, b); return a; }
    __host__ __device__ inline bfloat16_t& operator/=(bfloat16_t& a, const bfloat16_t& b) { a = operator/(a, b); return a; }

    __host__ __device__ inline bool operator<(const bfloat16_t& a, const bfloat16_t& b) {
        if (a.is_nan() || b.is_nan()) return false;
        if ((a.x & 0x7FFFu) == 0u && (b.x & 0x7FFFu) == 0u) return false;
        uint16_t sa = a.x >> 15, sb = b.x >> 15;
        if (sa != sb) return sa > sb;
        return (sa == 0u) ? (a.x < b.x) : (a.x > b.x);
    }
    __host__ __device__ inline bool operator==(const bfloat16_t& a, const bfloat16_t& b) {
        if (a.is_nan() || b.is_nan()) return false;
        if ((a.x & 0x7FFFu) == 0u && (b.x & 0x7FFFu) == 0u) return true;
        return a.x == b.x;
    }
    __host__ __device__ inline bool operator!=(const bfloat16_t& a, const bfloat16_t& b) { return !(operator==(a, b)); }
    __host__ __device__ inline bool operator> (const bfloat16_t& a, const bfloat16_t& b) { return  operator< (b, a); }
    __host__ __device__ inline bool operator<=(const bfloat16_t& a, const bfloat16_t& b) { return !operator< (b, a); }
    __host__ __device__ inline bool operator>=(const bfloat16_t& a, const bfloat16_t& b) { return !operator< (a, b); }

} // namespace bf16_c7x
namespace bf16_mma {

    __host__ __device__ inline bfloat16_t operator+(const bfloat16_t& a, const bfloat16_t& b) {
        return static_cast<float>(a) + static_cast<float>(b);
    }

    __host__ __device__ inline bfloat16_t operator-(const bfloat16_t& a, const bfloat16_t& b) {
        return static_cast<float>(a) - static_cast<float>(b);
    }

    __host__ __device__ inline bfloat16_t operator*(const bfloat16_t& a, const bfloat16_t& b) {
        return static_cast<float>(a) * static_cast<float>(b);
    }

    __host__ __device__ inline bfloat16_t operator/(const bfloat16_t& a, const bfloat16_t& b) {
        return static_cast<float>(a) / static_cast<float>(b);
    }

    __host__ __device__ inline bfloat16_t operator-(const bfloat16_t& a) {
        return -static_cast<float>(a);
    }

    __host__ __device__ inline bfloat16_t& operator+=(bfloat16_t& a, const bfloat16_t& b) {
        a = a + b;
        return a;
    }

    __host__ __device__ inline bfloat16_t& operator-=(bfloat16_t& a, const bfloat16_t& b) {
        a = a - b;
        return a;
    }

    __host__ __device__ inline bfloat16_t& operator*=(bfloat16_t& a, const bfloat16_t& b) {
        a = a * b;
        return a;
    }

    __host__ __device__ inline bfloat16_t& operator/=(bfloat16_t& a, const bfloat16_t& b) {
        a = a / b;
        return a;
    }

    __host__ __device__ inline bool operator==(const bfloat16_t& lhs, const bfloat16_t& rhs) {
        return static_cast<float>(lhs) == static_cast<float>(rhs);
    }

    __host__ __device__ inline bool operator!=(const bfloat16_t& lhs, const bfloat16_t& rhs) {
        return static_cast<float>(lhs) != static_cast<float>(rhs);
    }

    __host__ __device__ inline bool operator>(const bfloat16_t& lhs, const bfloat16_t& rhs) {
        return static_cast<float>(lhs) > static_cast<float>(rhs);
    }

    __host__ __device__ inline bool operator<(const bfloat16_t& lhs, const bfloat16_t& rhs) {
        return static_cast<float>(lhs) < static_cast<float>(rhs);
    }

    __host__ __device__ inline bool operator>=(const bfloat16_t& lhs, const bfloat16_t& rhs) {
        return static_cast<float>(lhs) >= static_cast<float>(rhs);
    }

    __host__ __device__ inline bool operator<=(const bfloat16_t& lhs, const bfloat16_t& rhs) {
        return static_cast<float>(lhs) <= static_cast<float>(rhs);
    }

    /// Arithmetic with floats

    __host__ __device__ inline float operator+(bfloat16_t a, float b) {
        return static_cast<float>(a) + b;
    }
    __host__ __device__ inline float operator-(bfloat16_t a, float b) {
        return static_cast<float>(a) - b;
    }
    __host__ __device__ inline float operator*(bfloat16_t a, float b) {
        return static_cast<float>(a) * b;
    }
    __host__ __device__ inline float operator/(bfloat16_t a, float b) {
        return static_cast<float>(a) / b;
    }

    __host__ __device__ inline float operator+(float a, bfloat16_t b) {
        return a + static_cast<float>(b);
    }
    __host__ __device__ inline float operator-(float a, bfloat16_t b) {
        return a - static_cast<float>(b);
    }
    __host__ __device__ inline float operator*(float a, bfloat16_t b) {
        return a * static_cast<float>(b);
    }
    __host__ __device__ inline float operator/(float a, bfloat16_t b) {
        return a / static_cast<float>(b);
    }

    __host__ __device__ inline float& operator+=(float& a, const bfloat16_t& b) {
        return a += static_cast<float>(b);
    }
    __host__ __device__ inline float& operator-=(float& a, const bfloat16_t& b) {
        return a -= static_cast<float>(b);
    }
    __host__ __device__ inline float& operator*=(float& a, const bfloat16_t& b) {
        return a *= static_cast<float>(b);
    }
    __host__ __device__ inline float& operator/=(float& a, const bfloat16_t& b) {
        return a /= static_cast<float>(b);
    }

    /// Arithmetic with doubles

    __host__ __device__ inline double operator+(bfloat16_t a, double b) {
        return static_cast<double>(a) + b;
    }
    __host__ __device__ inline double operator-(bfloat16_t a, double b) {
        return static_cast<double>(a) - b;
    }
    __host__ __device__ inline double operator*(bfloat16_t a, double b) {
        return static_cast<double>(a) * b;
    }
    __host__ __device__ inline double operator/(bfloat16_t a, double b) {
        return static_cast<double>(a) / b;
    }

    __host__ __device__ inline double operator+(double a, bfloat16_t b) {
        return a + static_cast<double>(b);
    }
    __host__ __device__ inline double operator-(double a, bfloat16_t b) {
        return a - static_cast<double>(b);
    }
    __host__ __device__ inline double operator*(double a, bfloat16_t b) {
        return a * static_cast<double>(b);
    }
    __host__ __device__ inline double operator/(double a, bfloat16_t b) {
        return a / static_cast<double>(b);
    }

    /// Arithmetic with ints

    __host__ __device__ inline bfloat16_t operator+(bfloat16_t a, int b) {
        return a + static_cast<bfloat16_t>(b);
    }
    __host__ __device__ inline bfloat16_t operator-(bfloat16_t a, int b) {
        return a - static_cast<bfloat16_t>(b);
    }
    __host__ __device__ inline bfloat16_t operator*(bfloat16_t a, int b) {
        return a * static_cast<bfloat16_t>(b);
    }
    __host__ __device__ inline bfloat16_t operator/(bfloat16_t a, int b) {
        return a / static_cast<bfloat16_t>(b);
    }

    __host__ __device__ inline bfloat16_t operator+(int a, bfloat16_t b) {
        return static_cast<bfloat16_t>(a) + b;
    }
    __host__ __device__ inline bfloat16_t operator-(int a, bfloat16_t b) {
        return static_cast<bfloat16_t>(a) - b;
    }
    __host__ __device__ inline bfloat16_t operator*(int a, bfloat16_t b) {
        return static_cast<bfloat16_t>(a) * b;
    }
    __host__ __device__ inline bfloat16_t operator/(int a, bfloat16_t b) {
        return static_cast<bfloat16_t>(a) / b;
    }

    //// Arithmetic with int64_t

    __host__ __device__ inline bfloat16_t operator+(bfloat16_t a, int64_t b) {
        return a + static_cast<bfloat16_t>(b);
    }
    __host__ __device__ inline bfloat16_t operator-(bfloat16_t a, int64_t b) {
        return a - static_cast<bfloat16_t>(b);
    }
    __host__ __device__ inline bfloat16_t operator*(bfloat16_t a, int64_t b) {
        return a * static_cast<bfloat16_t>(b);
    }
    __host__ __device__ inline bfloat16_t operator/(bfloat16_t a, int64_t b) {
        return a / static_cast<bfloat16_t>(b);
    }

    __host__ __device__ inline bfloat16_t operator+(int64_t a, bfloat16_t b) {
        return static_cast<bfloat16_t>(a) + b;
    }
    __host__ __device__ inline bfloat16_t operator-(int64_t a, bfloat16_t b) {
        return static_cast<bfloat16_t>(a) - b;
    }
    __host__ __device__ inline bfloat16_t operator*(int64_t a, bfloat16_t b) {
        return static_cast<bfloat16_t>(a) * b;
    }
    __host__ __device__ inline bfloat16_t operator/(int64_t a, bfloat16_t b) {
        return static_cast<bfloat16_t>(a) / b;
    }
} // namespace bf16_mma

__host__ __device__ inline bfloat16_t abs(const bfloat16_t& a) {
    // ABS: sNaN → +qNaN; qNaN → preserve sign (then sign cleared by ABS → always +qNaN)
    if (a.is_nan()) return bf16_c7x::canonicalize_nan(a);
    return bfloat16_t(static_cast<uint16_t>(a.x & ~bfloat16_t::SIGN_MASK), bfloat16_t::from_bits());
}
__host__ __device__ inline bfloat16_t min(const bfloat16_t& a, const bfloat16_t& b) {
    // MIN: sNaN → +qNaN; qNaN → preserve sign. Hardware returns b when a == b.
    if (a.is_nan()) return bf16_c7x::canonicalize_nan(a);
    if (b.is_nan()) return bf16_c7x::canonicalize_nan(b);
    return bf16_c7x::operator<(a, b) ? a : b;
}
__host__ __device__ inline bfloat16_t max(const bfloat16_t& a, const bfloat16_t& b) {
    // MAX: sNaN → +qNaN; qNaN → preserve sign. Hardware returns b when a == b.
    if (a.is_nan()) return bf16_c7x::canonicalize_nan(a);
    if (b.is_nan()) return bf16_c7x::canonicalize_nan(b);
    return bf16_c7x::operator<(b, a) ? a : b;
}

} // namespace floating_point

namespace std {
    template<>
    class numeric_limits<floating_point::bfloat16_t> {
    public:
        static constexpr bool is_signed      = true;
        static constexpr bool is_specialized = true;
        static constexpr bool is_integer     = false;
        static constexpr bool is_exact       = false;
        static constexpr bool has_infinity   = true;
        static constexpr bool has_quiet_nan  = true;
        static constexpr bool has_signaling_nan = true;
        static constexpr bool is_iec559      = false;
        static constexpr bool is_bounded     = true;
        static constexpr bool is_modulo      = false;
        static constexpr int  digits         = 8;
        static constexpr int  radix          = 2;
        static constexpr int  max_exponent   = 128;
        static constexpr int  min_exponent   = -125;
        __host__ __device__ static constexpr floating_point::bfloat16_t min()      { return floating_point::bfloat16_t(0x0080, floating_point::bfloat16_t::from_bits()); }
        __host__ __device__ static constexpr floating_point::bfloat16_t lowest()   { return floating_point::bfloat16_t(0xFF7F, floating_point::bfloat16_t::from_bits()); }
        __host__ __device__ static constexpr floating_point::bfloat16_t max()      { return floating_point::bfloat16_t(0x7F7F, floating_point::bfloat16_t::from_bits()); }
        __host__ __device__ static constexpr floating_point::bfloat16_t epsilon()  { return floating_point::bfloat16_t(0x3C00, floating_point::bfloat16_t::from_bits()); }
        __host__ __device__ static constexpr floating_point::bfloat16_t infinity() { return floating_point::bfloat16_t(0x7F80, floating_point::bfloat16_t::from_bits()); }
        __host__ __device__ static constexpr floating_point::bfloat16_t quiet_NaN(){ return floating_point::bfloat16_t(0x7FC0, floating_point::bfloat16_t::from_bits()); }
    };
} // namespace std

/* cuda::std::numeric_limits specialization for device code.
 * NVCC device compilation does not see the host-side <limits> specialization
 * above; it resolves std::numeric_limits through libcu++ (cuda::std).
 * Add an identical specialization in cuda::std so that device functions can
 * call cuda::std::numeric_limits<bfloat16_tidl>::max() / lowest() correctly. */
#ifdef __CUDACC__
#include <cuda/std/limits>
namespace cuda { namespace std {
    template<>
    class numeric_limits<floating_point::bfloat16_t> {
    public:
        static constexpr bool is_signed         = true;
        static constexpr bool is_specialized    = true;
        static constexpr bool is_integer        = false;
        static constexpr bool is_exact          = false;
        static constexpr bool has_infinity      = true;
        static constexpr bool has_quiet_nan     = true;
        static constexpr bool has_signaling_nan = true;
        static constexpr bool is_iec559         = false;
        static constexpr bool is_bounded        = true;
        static constexpr bool is_modulo         = false;
        static constexpr int  digits            = 8;
        static constexpr int  radix             = 2;
        static constexpr int  max_exponent      = 128;
        static constexpr int  min_exponent      = -125;
        __host__ __device__ static constexpr floating_point::bfloat16_t min()       { return floating_point::bfloat16_t(0x0080, floating_point::bfloat16_t::from_bits()); }
        __host__ __device__ static constexpr floating_point::bfloat16_t lowest()    { return floating_point::bfloat16_t(0xFF7F, floating_point::bfloat16_t::from_bits()); }
        __host__ __device__ static constexpr floating_point::bfloat16_t max()       { return floating_point::bfloat16_t(0x7F7F, floating_point::bfloat16_t::from_bits()); }
        __host__ __device__ static constexpr floating_point::bfloat16_t epsilon()   { return floating_point::bfloat16_t(0x3C00, floating_point::bfloat16_t::from_bits()); }
        __host__ __device__ static constexpr floating_point::bfloat16_t infinity()  { return floating_point::bfloat16_t(0x7F80, floating_point::bfloat16_t::from_bits()); }
        __host__ __device__ static constexpr floating_point::bfloat16_t quiet_NaN() { return floating_point::bfloat16_t(0x7FC0, floating_point::bfloat16_t::from_bits()); }
    };
}} // namespace cuda::std
#endif /* __CUDACC__ */

typedef floating_point::bfloat16_t bfloat16_tidl;

#endif /* __cplusplus */

#endif /* TIDL_BFLOAT16_H */

#endif /* BUILD_WITH_CUDA */
#endif /* TIDL_CUDA_BFLOAT16_H_ */
