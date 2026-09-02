#pragma once

#if defined(__C7604__)

#include <cmath>
#include <cstdint>
#include <cstring>
#include <iosfwd>
#include <ostream>

#include "floating_point_utils.h"

namespace floating_point {

    struct alignas(2) bfloat16_t {
        static constexpr uint16_t SIGN_MASK = 0x8000U;
        static constexpr uint16_t EXPONENT_MASK = 0x7F80U;
        static constexpr uint16_t MANTISSA_MASK = 0x007FU;
        static constexpr uint16_t NON_SIGN_BITS = 0x7FFFU;
        static constexpr uint16_t NAN_BITS = 0x7FC0U;
        static constexpr uint16_t POS_INFINITY_BITS = 0x7F80U;
        static constexpr uint16_t NEG_INFINITY_BITS = 0xFF80U;

        uint16_t x;

        bfloat16_t() = default;

        struct from_bits_t {};

        static constexpr from_bits_t from_bits() {
            return from_bits_t();
        }

        constexpr bfloat16_t(uint16_t bits, from_bits_t)
            : x(bits) {}

        inline bfloat16_t(float value);
        inline operator float() const;

        // Special value detection
        inline bool is_nan() const {
            return ((x & EXPONENT_MASK) == EXPONENT_MASK) && ((x & MANTISSA_MASK) != 0);
        }

        inline bool is_infinity() const {
            return (x & NON_SIGN_BITS) == POS_INFINITY_BITS;
        }

        inline bool is_zero() const {
            return (x & NON_SIGN_BITS) == 0;
        }

        inline bool is_finite() const {
            return (x & EXPONENT_MASK) != EXPONENT_MASK;
        }

        inline bool is_negative() const {
            return (x & SIGN_MASK) != 0;
        }

        // Static factory methods for special values
        inline static bfloat16_t zero() {
            return bfloat16_t(0x0000, bfloat16_t::from_bits());
        }

        inline static bfloat16_t negative_zero() {
            return bfloat16_t(SIGN_MASK, bfloat16_t::from_bits());
        }

        inline static bfloat16_t positive_infinity() {
            return bfloat16_t(POS_INFINITY_BITS, bfloat16_t::from_bits());
        }

        inline static bfloat16_t negative_infinity() {
            return bfloat16_t(NEG_INFINITY_BITS, bfloat16_t::from_bits());
        }

        inline static bfloat16_t nan() {
            return bfloat16_t(NAN_BITS, bfloat16_t::from_bits());
        }
    };

    inline std::ostream& operator<<(std::ostream& out, const bfloat16_t& value) {
        out << (float)value;
        return out;
    }

    namespace bit_ops {
        inline float f32_from_bits(uint16_t src) {
            float res = 0;
            uint32_t tmp = src;
            tmp <<= 16;

            std::memcpy(&res, &tmp, sizeof(tmp));

            return res;
        }

        inline uint16_t bits_from_f32(float src) {
            uint32_t res = 0;
            std::memcpy(&res, &src, sizeof(res));
            res >>= 16;
            return res;
        }

        // Helper functions for bit manipulation
        inline uint16_t get_sign_bit(uint16_t bits) {
            return (bits & bfloat16_t::SIGN_MASK) >> 15;
        }

        inline uint16_t get_exponent(uint16_t bits) {
            return (bits & bfloat16_t::EXPONENT_MASK) >> 7;
        }

        inline uint16_t get_mantissa(uint16_t bits) {
            return bits & bfloat16_t::MANTISSA_MASK;
        }

        inline uint16_t round_to_nearest_even(float src) {
            const uint32_t U32 = floating_point::bit_cast<uint32_t>(src);
            const uint32_t sign = U32 & 0x80000000U;
            const uint32_t exponent = U32 & 0x7F800000U;
            const uint32_t mantissa = U32 & 0x007FFFFFU;

            // Preserve sign bit and encode as NaN
            if ((exponent == 0x7F800000U) && (mantissa != 0)) {
                return static_cast<uint16_t>((sign >> 16) | 0x7FC0U);
            }

            // Preserve sign bit for infinity
            if (exponent == 0x7F800000U) {
                return static_cast<uint16_t>((sign >> 16) | 0x7F80U);
            }

            // Preserve sign bit for signed zero; subnormals (flush to zero)
            if (exponent < 0x00800000U) { // Exponent < 1 (includes subnormals and zero)
                return static_cast<uint16_t>(sign >> 16);
            }

            // Handle overflow (exponent too large for bfloat16_t)
            // bfloat16_t has 8 exponent bits, so max exponent is 2^8-1 = 255
            // After removing bias 127, that's 2^128
            // Return signed infinity
            if (exponent > 0x7F000000U) { // Exponent bits exceed 254 (0xFE)
                return static_cast<uint16_t>((sign >> 16) | 0x7F80U);
            }

            // Normal case: round to nearest even
            uint32_t rounding_bias = ((U32 >> 16) & 1) + UINT32_C(0x7FFF);
            return static_cast<uint16_t>((U32 + rounding_bias) >> 16);
        }
    } // namespace bit_ops

    //constructors
    inline bfloat16_t::bfloat16_t(float value)
        : x(bit_ops::round_to_nearest_even(value)) {}
    // {
    //     /* Use VSPBF hardware instruction (round-to-nearest-even).
    //      * Avoids the TI C7604 compiler bug where bit_cast<T>+memcpy aliases
    //      * the float argument stack slot with the local dst, zeroing the input
    //      * before it is read.  __float_to_low_bf16 emits a single VSPBF insn
    //      * and the from_bits constructor stores the raw uint16 directly. */
    //     __bf16_ bval  = __float_to_low_bf16(value).lo();
    //     uint16_t bits = 0;
    //     *((__bf16_*)&bits) = bval;
    //     x = bits;
    // }


    //Implicit conversion
    bfloat16_t::operator float() const {
        return bit_ops::f32_from_bits(x);
    }

    // floating_point::bf16_mma — MMA-accumulator model. Every operator here
    // promotes both operands to float32, computes, and rounds back once.
    // This matches the MMA unit's real fp32-accumulation hardware semantics
    // (see tidl_innerProduct_ref.c / tidl_deconv2d_ref.c consumers).
    namespace bf16_mma {

        inline bfloat16_t operator+(const bfloat16_t& a, const bfloat16_t& b) {
            return static_cast<float>(a) + static_cast<float>(b);
        }

        inline bfloat16_t operator-(const bfloat16_t& a, const bfloat16_t& b) {
            return static_cast<float>(a) - static_cast<float>(b);
        }

        inline bfloat16_t operator*(const bfloat16_t& a, const bfloat16_t& b) {
            return static_cast<float>(a) * static_cast<float>(b);
        }

        inline bfloat16_t operator/(const bfloat16_t& a, const bfloat16_t& b) {
            return static_cast<float>(a) / static_cast<float>(b);
        }

        inline bfloat16_t operator-(const bfloat16_t& a) {
            return -static_cast<float>(a);
        }

        inline bfloat16_t& operator+=(bfloat16_t& a, const bfloat16_t& b) {
            a = a + b;
            return a;
        }

        inline bfloat16_t& operator-=(bfloat16_t& a, const bfloat16_t& b) {
            a = a - b;
            return a;
        }

        inline bfloat16_t& operator*=(bfloat16_t& a, const bfloat16_t& b) {
            a = a * b;
            return a;
        }

        inline bfloat16_t& operator/=(bfloat16_t& a, const bfloat16_t& b) {
            a = a / b;
            return a;
        }

        inline bool operator==(const bfloat16_t& lhs, const bfloat16_t& rhs) {
            return static_cast<float>(lhs) == static_cast<float>(rhs);
        }

        inline bool operator!=(const bfloat16_t& lhs, const bfloat16_t& rhs) {
            return static_cast<float>(lhs) != static_cast<float>(rhs);
        }

        inline bool operator>(const bfloat16_t& lhs, const bfloat16_t& rhs) {
            return static_cast<float>(lhs) > static_cast<float>(rhs);
        }

        inline bool operator<(const bfloat16_t& lhs, const bfloat16_t& rhs) {
            return static_cast<float>(lhs) < static_cast<float>(rhs);
        }

        inline bool operator>=(const bfloat16_t& lhs, const bfloat16_t& rhs) {
            return static_cast<float>(lhs) >= static_cast<float>(rhs);
        }

        inline bool operator<=(const bfloat16_t& lhs, const bfloat16_t& rhs) {
            return static_cast<float>(lhs) <= static_cast<float>(rhs);
        }

        /// Arithmetic with floats

        inline float operator+(bfloat16_t a, float b) {
            return static_cast<float>(a) + b;
        }
        inline float operator-(bfloat16_t a, float b) {
            return static_cast<float>(a) - b;
        }
        inline float operator*(bfloat16_t a, float b) {
            return static_cast<float>(a) * b;
        }
        inline float operator/(bfloat16_t a, float b) {
            return static_cast<float>(a) / b;
        }

        inline float operator+(float a, bfloat16_t b) {
            return a + static_cast<float>(b);
        }
        inline float operator-(float a, bfloat16_t b) {
            return a - static_cast<float>(b);
        }
        inline float operator*(float a, bfloat16_t b) {
            return a * static_cast<float>(b);
        }
        inline float operator/(float a, bfloat16_t b) {
            return a / static_cast<float>(b);
        }

        inline float& operator+=(float& a, const bfloat16_t& b) {
            return a += static_cast<float>(b);
        }
        inline float& operator-=(float& a, const bfloat16_t& b) {
            return a -= static_cast<float>(b);
        }
        inline float& operator*=(float& a, const bfloat16_t& b) {
            return a *= static_cast<float>(b);
        }
        inline float& operator/=(float& a, const bfloat16_t& b) {
            return a /= static_cast<float>(b);
        }

        /// Arithmetic with doubles

        inline double operator+(bfloat16_t a, double b) {
            return static_cast<double>(a) + b;
        }
        inline double operator-(bfloat16_t a, double b) {
            return static_cast<double>(a) - b;
        }
        inline double operator*(bfloat16_t a, double b) {
            return static_cast<double>(a) * b;
        }
        inline double operator/(bfloat16_t a, double b) {
            return static_cast<double>(a) / b;
        }

        inline double operator+(double a, bfloat16_t b) {
            return a + static_cast<double>(b);
        }
        inline double operator-(double a, bfloat16_t b) {
            return a - static_cast<double>(b);
        }
        inline double operator*(double a, bfloat16_t b) {
            return a * static_cast<double>(b);
        }
        inline double operator/(double a, bfloat16_t b) {
            return a / static_cast<double>(b);
        }

        /// Arithmetic with ints

        inline bfloat16_t operator+(bfloat16_t a, int b) {
            return a + static_cast<bfloat16_t>(b);
        }
        inline bfloat16_t operator-(bfloat16_t a, int b) {
            return a - static_cast<bfloat16_t>(b);
        }
        inline bfloat16_t operator*(bfloat16_t a, int b) {
            return a * static_cast<bfloat16_t>(b);
        }
        inline bfloat16_t operator/(bfloat16_t a, int b) {
            return a / static_cast<bfloat16_t>(b);
        }

        inline bfloat16_t operator+(int a, bfloat16_t b) {
            return static_cast<bfloat16_t>(a) + b;
        }
        inline bfloat16_t operator-(int a, bfloat16_t b) {
            return static_cast<bfloat16_t>(a) - b;
        }
        inline bfloat16_t operator*(int a, bfloat16_t b) {
            return static_cast<bfloat16_t>(a) * b;
        }
        inline bfloat16_t operator/(int a, bfloat16_t b) {
            return static_cast<bfloat16_t>(a) / b;
        }

        //// Arithmetic with int64_t

        inline bfloat16_t operator+(bfloat16_t a, int64_t b) {
            return a + static_cast<bfloat16_t>(b);
        }
        inline bfloat16_t operator-(bfloat16_t a, int64_t b) {
            return a - static_cast<bfloat16_t>(b);
        }
        inline bfloat16_t operator*(bfloat16_t a, int64_t b) {
            return a * static_cast<bfloat16_t>(b);
        }
        inline bfloat16_t operator/(bfloat16_t a, int64_t b) {
            return a / static_cast<bfloat16_t>(b);
        }

        inline bfloat16_t operator+(int64_t a, bfloat16_t b) {
            return static_cast<bfloat16_t>(a) + b;
        }
        inline bfloat16_t operator-(int64_t a, bfloat16_t b) {
            return static_cast<bfloat16_t>(a) - b;
        }
        inline bfloat16_t operator*(int64_t a, bfloat16_t b) {
            return static_cast<bfloat16_t>(a) * b;
        }
        inline bfloat16_t operator/(int64_t a, bfloat16_t b) {
            return static_cast<bfloat16_t>(a) / b;
        }
    } // namespace bf16_mma

    // floating_point::bf16_c7x — bit-accurate native BF16 ALU model (VADDBF/
    // VMPYBF/VRCPBF-style): every operator here stays in bf16 working
    // precision throughout (unpack -> align -> compute -> normalize -> round
    // once) and never promotes through float32. Use this namespace for
    // elementwise/scalar-ALU ops; use bf16_mma above for MMA-accumulator ops.
    namespace bf16_c7x {

        // -----------------------------------------------------------------------
        // NaN canonicalization helpers (match C7604 hardware behaviour)
        // -----------------------------------------------------------------------
        // ADD/SUB: any NaN input → canonical +qNaN (0x7FC0), sign always cleared.
        // MUL/DIV/ABS/MIN/MAX:
        //   sNaN (quiet bit == 0) → +qNaN (0x7FC0), sign cleared
        //   qNaN (quiet bit == 1) → qNaN with sign preserved, payload cleared
        inline bfloat16_t canonicalize_nan(const bfloat16_t& a) {
            bool is_quiet = ((a.x >> 6) & 1u) != 0u;
            if (is_quiet) {
                // qNaN: preserve sign, clear payload
                return bfloat16_t(static_cast<uint16_t>((a.x & bfloat16_t::SIGN_MASK) | bfloat16_t::NAN_BITS),
                                  bfloat16_t::from_bits());
            }
            // sNaN: always returns canonical +qNaN
            return bfloat16_t::nan();
        }

        // C7604 has no subnormal support: any input with a zero exponent field
        // (subnormal or already zero) is flushed to a signed zero before any
        // arithmetic runs. Idempotent on exact zero, leaves NaN/Inf untouched.
        inline bfloat16_t flush_subnormal(const bfloat16_t& a) {
            if ((a.x & bfloat16_t::EXPONENT_MASK) == 0) {
                return bfloat16_t(static_cast<uint16_t>(a.x & bfloat16_t::SIGN_MASK), bfloat16_t::from_bits());
            }
            return a;
        }

        // -----------------------------------------------------------------------
        // Internal helpers
        // -----------------------------------------------------------------------

        // Working-precision representation.
        // sig = (implicit_1 | 7-bit mantissa) << 7, stored in uint32_t.
        // This places the 8-bit significand at bits [13:7], leaving bits [6:0] = 0
        // as guard/sticky headroom for alignment and rounding.
        //
        //  bit 14   : carry headroom (set only after addition overflow)
        //  bit 13   : implicit 1
        //  bits 12:7: mantissa bits
        //  bit 6    : guard bit   — (sig >> 6) & 1
        //  bits 5:0 : sticky bits — (sig & 0x3F) != 0
        struct bf16_parts
        {
            uint16_t sign, exp;
            uint32_t sig;
        };

        inline bf16_parts bf16_unpack(const bfloat16_t& v)
        {
            return
            {
                bit_ops::get_sign_bit(v.x),
                bit_ops::get_exponent(v.x),
                static_cast<uint32_t>(0x80u | bit_ops::get_mantissa(v.x)) << 7
            };
        }

        // Right-shift sig by shift positions; return {shifted_sig, sticky}.
        // sticky = true if any set bit was shifted off.
        inline std::pair<uint32_t, bool> bf16_align_right(uint32_t sig, uint16_t shift)
        {
            if (shift == 0)
            {
                return {sig, false};
            }
            if (shift >= 15)
            {
                return {0u, sig != 0u};
            }
            bool sticky = (sig & ((1u << shift) - 1u)) != 0u;
            return { sig >> shift, sticky };
        }

        // Round-to-nearest-even using guard (bit 6) and sticky (bits 5:0 | extra_sticky).
        // Adds 1 ULP at bit 7 when round-up is required; returns rounded sig.
        inline uint32_t bf16_round_rne(uint32_t sig, bool extra_sticky)
        {
            bool guard  = (sig >> 6) & 1U;
            bool sticky = extra_sticky || ((sig & 0x3FU) != 0U);
            bool lsb    = (sig >> 7) & 1U;
            if (guard && (sticky || lsb))
            {
                sig += 0x80U;
            }
            return sig;
        }

        // Pack sign + biased exponent + working-precision sig into bfloat16_t.
        inline bfloat16_t bf16_pack(uint16_t sign, uint16_t exp, uint32_t sig)
        {
            uint16_t bits = static_cast<uint16_t>(sign << 15)
                          | static_cast<uint16_t>(exp  <<  7)
                          | static_cast<uint16_t>((sig >> 7) & 0x7FU);
            return bfloat16_t(bits, bfloat16_t::from_bits());
        }

        // -----------------------------------------------------------------------
        // operator+
        // Pipeline: special-cases → unpack → order → align → add/sub →
        //           normalize → round → overflow check → pack
        // -----------------------------------------------------------------------
        inline bfloat16_t operator+(const bfloat16_t& a_in, const bfloat16_t& b_in) {

            // Step 0 — Special cases
            bfloat16_t a = flush_subnormal(a_in);
            bfloat16_t b = flush_subnormal(b_in);
            // ADD: any NaN input → canonical +qNaN (0x7FC0), sign always cleared
            if (a.is_nan() || b.is_nan()) return bfloat16_t::nan();
            if (a.is_zero())
            {
                if (b.is_zero())
                {
                    return (a.is_negative() && b.is_negative()) ? bfloat16_t::negative_zero() : bfloat16_t::zero();
                }
                return b;
            }
            if (b.is_zero())
            {
                return a;
            }
            if (a.is_infinity() || b.is_infinity())
            {
                if (a.is_infinity() && b.is_infinity())
                {
                    bool same_sign = (a.x & bfloat16_t::SIGN_MASK) == (b.x & bfloat16_t::SIGN_MASK);
                    return same_sign ? a : bfloat16_t::nan();
                }
                return a.is_infinity() ? a : b;
            }

            // Step 1 — Unpack into working precision
            bf16_parts pa = bf16_unpack(a);
            bf16_parts pb = bf16_unpack(b);

            // Step 2 — Order so pa.exp >= pb.exp (ensures pa.sig >= aligned pb.sig)
            if (pa.exp < pb.exp)
            {
                bf16_parts tmp = pa;
                pa = pb;
                pb = tmp;
            }
            // When exponents are equal and signs differ (effective subtraction),
            // also ensure pa has the larger significand so pa.sig - sig_b >= 0.
            else if (pa.exp == pb.exp && pa.sign != pb.sign && pa.sig < pb.sig)
            {
                bf16_parts tmp = pa;
                pa = pb;
                pb = tmp;
            }

            // Step 3 — Align pb's significand to pa's exponent
            auto aligned   = bf16_align_right(pb.sig, static_cast<uint16_t>(pa.exp - pb.exp));
            uint32_t sig_b = aligned.first;
            bool   sticky  = aligned.second;

            // Step 4 — Add or subtract significands based on effective sign
            uint32_t m_res;
            uint16_t s_res;

            if (pa.sign == pb.sign)
            {
                m_res = pa.sig + sig_b;
                s_res = pa.sign;
            }
            else
            {
                m_res = pa.sig - sig_b;   // safe: pa.sig >= sig_b after Step 2
                s_res = pa.sign;
                if (m_res == 0)
                {
                    return bfloat16_t::zero();
                }
            }

            // Step 5 — Normalize
            uint16_t exp_res = pa.exp;
            if (pa.sign == pb.sign && m_res >= 0x8000u)
            {
                // Addition carry-out: implicit-1 shifted to bit 14
                sticky    |= (m_res & 1u) != 0u;
                m_res    >>= 1;
                exp_res++;
            }
            else if (pa.sign != pb.sign)
            {
                // Subtraction: shift left until implicit-1 is back at bit 13
                while (m_res < 0x4000u && exp_res > 0)
                {
                    m_res <<= 1;
                    exp_res--;
                }
                if (m_res < 0x4000u)
                {
                    return s_res ? bfloat16_t::negative_zero() : bfloat16_t::zero();
                }
            }

            // Step 6+7 — Round to nearest even; handle post-round carry
            m_res = bf16_round_rne(m_res, sticky);
            if (m_res >= 0x8000u)
            {
                m_res >>= 1;
                exp_res++;
            }

            // Step 8 — Overflow → infinity
            if (exp_res >= 0xFFu)
            {
                return s_res ? bfloat16_t::negative_infinity() : bfloat16_t::positive_infinity();
            }

            // Step 9 — Pack and return
            return bf16_pack(s_res, exp_res, m_res);
        }

        // Subtraction: negate b and reuse operator+
        inline bfloat16_t operator-(const bfloat16_t& a, const bfloat16_t& b) {
            bfloat16_t neg_b(static_cast<uint16_t>(b.x ^ bfloat16_t::SIGN_MASK),
                             bfloat16_t::from_bits());
            return operator+(a, neg_b);
        }

        // Multiplication: result sign = XOR of signs; result exponent = ea + eb - 127;
        // result significand = product of 8-bit significands normalized to working precision.
        inline bfloat16_t operator*(const bfloat16_t& a_in, const bfloat16_t& b_in) {
            bfloat16_t a = flush_subnormal(a_in);
            bfloat16_t b = flush_subnormal(b_in);
            // MUL: sNaN → +qNaN; qNaN → preserve sign (hardware VMPYBF rule)
            if (a.is_nan()) return canonicalize_nan(a);
            if (b.is_nan()) return canonicalize_nan(b);
            uint16_t s_res = static_cast<uint16_t>(bit_ops::get_sign_bit(a.x) ^ bit_ops::get_sign_bit(b.x));
            if (a.is_infinity() || b.is_infinity())
            {
                if (a.is_zero() || b.is_zero())
                {
                    // Inf * 0 → NaN whose sign = sign(a) XOR sign(b)
                    return bfloat16_t(static_cast<uint16_t>(bfloat16_t::NAN_BITS | (s_res << 15u)),
                                      bfloat16_t::from_bits());
                }
                return s_res ? bfloat16_t::negative_infinity() : bfloat16_t::positive_infinity();
            }
            if (a.is_zero() || b.is_zero())
            {
                return bfloat16_t(static_cast<uint16_t>(s_res << 15), bfloat16_t::from_bits());
            }

            uint32_t ma = 0x80u | bit_ops::get_mantissa(a.x);
            uint32_t mb = 0x80u | bit_ops::get_mantissa(b.x);
            int32_t  exp_res = static_cast<int32_t>(bit_ops::get_exponent(a.x)) + static_cast<int32_t>(bit_ops::get_exponent(b.x)) - 127;

            // Product of two 8-bit significands is in [0x4000, 0xFE01] (14–16 bits).
            // Implicit 1 lands at bit 14 (no overflow) or bit 15 (overflow).
            uint32_t prod = ma * mb;
            bool sticky_mul = false;
            if (prod >= 0x8000u)
            {
                // Overflow: shift right to bring implicit 1 to bit 14
                sticky_mul = (prod & 1u) != 0u;
                prod >>= 1;
                exp_res++;
            }
            // prod is now in working-precision format: implicit 1 at bit 14

            prod = bf16_round_rne(prod, sticky_mul);
            if (prod >= 0x8000u)
            {
                prod >>= 1;
                exp_res++;
            }  // post-round carry

            if (exp_res >= 0xFF)
            {
                return s_res ? bfloat16_t::negative_infinity() : bfloat16_t::positive_infinity();
            }
            if (exp_res <= 0)
            {
                return bfloat16_t(static_cast<uint16_t>(s_res << 15), bfloat16_t::from_bits());
            }

            return bf16_pack(s_res, static_cast<uint16_t>(exp_res), prod);
        }

        // -----------------------------------------------------------------------
        // bf16_recip_bitmatch: bit-accurate SW model of the C7604 VRCPBF instruction.
        //
        // 128-entry LUT indexed by the 7 mantissa bits of the BF16 input.
        // Each entry is the BF16 bit-pattern of recip(1 + m/128), m = 0..127.
        // Generated by enumerating __recip(__bf16_) on the C7604 host-emulation library.
        //
        // Exponent inversion:
        //   m == 0 (exact power-of-2) → new_biased_exp = 254 - old_biased_exp
        //   m  > 0                    → new_biased_exp = 253 - old_biased_exp
        //
        // Special cases (confirmed against C7604 hardware):
        //   NaN           → canonical +qNaN (0x7FC0), sign cleared
        //   ±inf          → ±zero
        //   zero/subnorm  → ±inf
        // -----------------------------------------------------------------------
        inline bfloat16_t bf16_recip_bitmatch(const bfloat16_t& b)
        {
            static const uint16_t vrcpbf_lut[128] = {
                /* m=  0 */ 0x3F80, 0x3F7E, 0x3F7C, 0x3F7A,
                /* m=  4 */ 0x3F78, 0x3F76, 0x3F74, 0x3F72,
                /* m=  8 */ 0x3F71, 0x3F6F, 0x3F6D, 0x3F6B,
                /* m= 12 */ 0x3F6A, 0x3F68, 0x3F67, 0x3F65,
                /* m= 16 */ 0x3F63, 0x3F62, 0x3F60, 0x3F5F,
                /* m= 20 */ 0x3F5D, 0x3F5C, 0x3F5A, 0x3F59,
                /* m= 24 */ 0x3F57, 0x3F56, 0x3F55, 0x3F53,
                /* m= 28 */ 0x3F52, 0x3F50, 0x3F4F, 0x3F4E,
                /* m= 32 */ 0x3F4D, 0x3F4B, 0x3F4A, 0x3F49,
                /* m= 36 */ 0x3F48, 0x3F46, 0x3F45, 0x3F44,
                /* m= 40 */ 0x3F43, 0x3F42, 0x3F41, 0x3F3F,
                /* m= 44 */ 0x3F3E, 0x3F3D, 0x3F3C, 0x3F3B,
                /* m= 48 */ 0x3F3A, 0x3F39, 0x3F38, 0x3F37,
                /* m= 52 */ 0x3F36, 0x3F35, 0x3F34, 0x3F33,
                /* m= 56 */ 0x3F32, 0x3F31, 0x3F30, 0x3F2F,
                /* m= 60 */ 0x3F2E, 0x3F2D, 0x3F2C, 0x3F2B,
                /* m= 64 */ 0x3F2A, 0x3F2A, 0x3F29, 0x3F28,
                /* m= 68 */ 0x3F27, 0x3F26, 0x3F25, 0x3F24,
                /* m= 72 */ 0x3F24, 0x3F23, 0x3F22, 0x3F21,
                /* m= 76 */ 0x3F20, 0x3F20, 0x3F1F, 0x3F1E,
                /* m= 80 */ 0x3F1D, 0x3F1D, 0x3F1C, 0x3F1B,
                /* m= 84 */ 0x3F1A, 0x3F1A, 0x3F19, 0x3F18,
                /* m= 88 */ 0x3F17, 0x3F17, 0x3F16, 0x3F15,
                /* m= 92 */ 0x3F15, 0x3F14, 0x3F13, 0x3F13,
                /* m= 96 */ 0x3F12, 0x3F11, 0x3F11, 0x3F10,
                /* m=100 */ 0x3F0F, 0x3F0F, 0x3F0E, 0x3F0E,
                /* m=104 */ 0x3F0D, 0x3F0C, 0x3F0C, 0x3F0B,
                /* m=108 */ 0x3F0B, 0x3F0A, 0x3F09, 0x3F09,
                /* m=112 */ 0x3F08, 0x3F08, 0x3F07, 0x3F07,
                /* m=116 */ 0x3F06, 0x3F05, 0x3F05, 0x3F04,
                /* m=120 */ 0x3F04, 0x3F03, 0x3F03, 0x3F02,
                /* m=124 */ 0x3F02, 0x3F01, 0x3F01, 0x3F00
            };

            bool     sign = (b.x & 0x8000u) != 0u;
            uint16_t exp  = (b.x & 0x7F80u) >> 7;
            uint16_t man  =  b.x & 0x007Fu;

            // NaN (exp=0xFF, man!=0) → canonical +qNaN (0x7FC0), sign cleared
            if (exp == 0xFFu && man != 0u)
                return bfloat16_t::nan();

            // ±inf → ±zero
            if (exp == 0xFFu)
                return bfloat16_t(static_cast<uint16_t>(sign ? 0x8000u : 0x0000u), bfloat16_t::from_bits());

            // zero / subnormal (exp=0) → ±inf
            if (exp == 0u)
                return bfloat16_t(static_cast<uint16_t>(sign ? 0xFF80u : 0x7F80u), bfloat16_t::from_bits());

            // Normal: look up reciprocal significand for input significand in [1, 2)
            uint16_t lut_man = vrcpbf_lut[man] & 0x007Fu;
            int      new_exp = (man == 0u) ? (254 - (int)exp) : (253 - (int)exp);

            if (new_exp >= 0xFF)
                return bfloat16_t(static_cast<uint16_t>(sign ? 0xFF80u : 0x7F80u), bfloat16_t::from_bits());
            if (new_exp <= 0)
                return bfloat16_t(static_cast<uint16_t>(sign ? 0x8000u : 0x0000u), bfloat16_t::from_bits());

            uint16_t result = static_cast<uint16_t>(sign ? 0x8000u : 0x0000u)
                            | static_cast<uint16_t>((new_exp & 0xFF) << 7)
                            | lut_man;
            return bfloat16_t(result, bfloat16_t::from_bits());
        }

        // Division: a / b = a * recip(b) using the C7x __recip hardware intrinsic.
        // Division operator overload defined in tidl_bfloat16.h

        // Unary negation: canonicalize NaN first, then flip sign.
        // sNaN(any sign) → +qNaN(0x7FC0) → -qNaN(0xFFC0)
        // +qNaN(0x7FC0) → -qNaN(0xFFC0);  -qNaN(0xFFC0) → +qNaN(0x7FC0)
        inline bfloat16_t operator-(const bfloat16_t& a) {
            if (a.is_nan()) {
                bfloat16_t canon = canonicalize_nan(a);
                return bfloat16_t(static_cast<uint16_t>(canon.x ^ bfloat16_t::SIGN_MASK),
                                  bfloat16_t::from_bits());
            }
            return bfloat16_t(static_cast<uint16_t>(a.x ^ bfloat16_t::SIGN_MASK), bfloat16_t::from_bits());
        }

        // Compound assignment: delegate to the binary operators above
        inline bfloat16_t& operator+=(bfloat16_t& a, const bfloat16_t& b) {
            a = operator+(a, b);
            return a;
        }
        inline bfloat16_t& operator-=(bfloat16_t& a, const bfloat16_t& b) {
            a = operator-(a, b);
            return a;
        }
        inline bfloat16_t& operator*=(bfloat16_t& a, const bfloat16_t& b) {
            a = operator*(a, b);
            return a;
        }
        // operator/ and operator/= defined in tidl_bfloat16.h (requires __recip intrinsic)

        // Comparisons — implemented entirely on bit patterns, no float promotion.
        // Rules: NaN comparisons always return false (except !=); +0 == -0.
        // operator< and operator== are the primitives; all others derive from them.

        inline bool operator<(const bfloat16_t& a, const bfloat16_t& b) {
            if (a.is_nan() || b.is_nan())
            {
                return false;
            }
            if ((a.x & 0x7FFFu) == 0u && (b.x & 0x7FFFu) == 0u)
            {
                return false;  // ±0 == ±0
            }
            uint16_t sa = a.x >> 15, sb = b.x >> 15;
            if (sa != sb)
            {
                return sa > sb;          // negative (sa=1) < positive (sa=0)
            }
            return (sa == 0u) ? (a.x < b.x)        // both positive: larger bits = larger value
                              : (a.x > b.x);        // both negative: larger bits = smaller value
        }

        inline bool operator==(const bfloat16_t& a, const bfloat16_t& b) {
            if (a.is_nan() || b.is_nan())
            {
                return false;
            }
            if ((a.x & 0x7FFFu) == 0u && (b.x & 0x7FFFu) == 0u)
            {
                return true;   // ±0 == ±0
            }
            return a.x == b.x;
        }

        inline bool operator!=(const bfloat16_t& a, const bfloat16_t& b) {
            return !(operator==(a, b));
        }
        inline bool operator> (const bfloat16_t& a, const bfloat16_t& b) {
            return operator< (b, a);
        }
        inline bool operator<=(const bfloat16_t& a, const bfloat16_t& b) {
            if (a.is_nan() || b.is_nan()) return false;
            return !(operator< (b, a));
        }
        inline bool operator>=(const bfloat16_t& a, const bfloat16_t& b) {
            if (a.is_nan() || b.is_nan()) return false;
            return !(operator< (a, b));
        }

    } // namespace bf16_c7x

    // Expose the bit-exact hardware model as the default bfloat16_t op bfloat16_t
    // arithmetic/comparison, so ADL finds it from plain `a + b` call sites (ADL only
    // searches floating_point, not its nested bf16_mma/bf16_c7x sub-namespaces).
    // Call sites that model an MMA-accumulated pipeline (which accumulates in float)
    // should explicitly qualify with bf16_mma::operator+/-/* /... instead.
    using namespace bf16_c7x;

    inline bfloat16_t& operator|(bfloat16_t& a, const bfloat16_t& b) {
        a.x = a.x | b.x;
        return a;
    }

    inline bfloat16_t& operator&(bfloat16_t& a, const bfloat16_t& b) {
        a.x = a.x & b.x;
        return a;
    }
    // The above overloaded operators can be templatised for both operations with other floats and int types.
    // Inital operator overloaded test with float, double, int and int64 only.

    // Mathematical utility functions
    inline bfloat16_t abs(const bfloat16_t& a) {
        // ABS: sNaN → +qNaN; qNaN → preserve sign (hardware confirmed)
        if (a.is_nan()) return bf16_c7x::canonicalize_nan(a);
        return bfloat16_t(a.x & ~bfloat16_t::SIGN_MASK, bfloat16_t::from_bits());
    }

    inline bfloat16_t min(const bfloat16_t& a, const bfloat16_t& b) {
        // MIN: sNaN → +qNaN; qNaN → preserve sign. Hardware returns b when a == b.
        if (a.is_nan()) return bf16_c7x::canonicalize_nan(a);
        if (b.is_nan()) return bf16_c7x::canonicalize_nan(b);
        // Special case for zeros with different signs: IEEE says -0 < +0
        if (a.is_zero() && b.is_zero()) {
            return a.is_negative() ? a : b;
        }
        // Explicitly qualified: bf16_c7x::operator< lives in a nested namespace
        // that ADL won't find from here, so an unqualified `a < b` would
        // silently fall back to float-promoted comparison.
        return bf16_c7x::operator<(a, b) ? a : b;
    }

    inline bfloat16_t max(const bfloat16_t& a, const bfloat16_t& b) {
        // MAX: sNaN → +qNaN; qNaN → preserve sign. Hardware returns b when a == b.
        if (a.is_nan()) return bf16_c7x::canonicalize_nan(a);
        if (b.is_nan()) return bf16_c7x::canonicalize_nan(b);
        // Special case for zeros with different signs: IEEE says -0 < +0
        if (a.is_zero() && b.is_zero()) {
            return a.is_negative() ? b : a;
        }
        // See comment in min() above — keep this explicitly qualified too.
        return bf16_c7x::operator>(a, b) ? a : b;
    }
}

namespace std {

    template<>
    class numeric_limits<floating_point::bfloat16_t> {
    public:
        static constexpr bool is_signed = true;
        static constexpr bool is_specialized = true;
        static constexpr bool is_integer = false;
        static constexpr bool is_exact = false;
        static constexpr bool has_infinity = true;
        static constexpr bool has_quiet_nan = true;
        static constexpr bool has_signaling_nan = true;
        static constexpr auto has_denorm = numeric_limits<float>::has_denorm;
        static constexpr bool has_denorm_loss = numeric_limits<float>::has_denorm_loss;
        static constexpr auto round_style = numeric_limits<float>::round_style;
        static constexpr bool is_iec559 = false;
        static constexpr bool is_bounded = true;
        static constexpr bool is_modulo = false;
        static constexpr int digits = 8;
        static constexpr int digits10 = 2;
        static constexpr int max_digits10 = 4;
        static constexpr int radix = 2;
        static constexpr int min_exponent = -125;
        static constexpr int min_exponent10 = -37;
        static constexpr int max_exponent = 128;
        static constexpr int max_exponent10 = 38;
        static constexpr auto traps = numeric_limits<float>::traps;
        static constexpr auto tinyness_before = numeric_limits<float>::tinyness_before;

        static constexpr floating_point::bfloat16_t min() {
            return floating_point::bfloat16_t(0x0080, floating_point::bfloat16_t::from_bits());
        }

        static constexpr floating_point::bfloat16_t lowest() {
            return floating_point::bfloat16_t(0xFF7F, floating_point::bfloat16_t::from_bits());
        }

        static constexpr floating_point::bfloat16_t max() {
            return floating_point::bfloat16_t(0x7F7F, floating_point::bfloat16_t::from_bits());
        }

        static constexpr floating_point::bfloat16_t epsilon() {
            return floating_point::bfloat16_t(0x3C00, floating_point::bfloat16_t::from_bits());
        }

        static constexpr floating_point::bfloat16_t round_error() {
            return floating_point::bfloat16_t(0x3F00, floating_point::bfloat16_t::from_bits());
        }

        static constexpr floating_point::bfloat16_t infinity() {
            return floating_point::bfloat16_t(0x7F80, floating_point::bfloat16_t::from_bits());
        }

        static constexpr floating_point::bfloat16_t quiet_NaN() {
            return floating_point::bfloat16_t(0x7FC0, floating_point::bfloat16_t::from_bits());
        }

        static constexpr floating_point::bfloat16_t signaling_NaN() {
            return floating_point::bfloat16_t(0x7F80, floating_point::bfloat16_t::from_bits());
        }

        static constexpr floating_point::bfloat16_t denorm_min() {
            return floating_point::bfloat16_t(0x0001, floating_point::bfloat16_t::from_bits());
        }
    };

}

#endif // __C7604__
