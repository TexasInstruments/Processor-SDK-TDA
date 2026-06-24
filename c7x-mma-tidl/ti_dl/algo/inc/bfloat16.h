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
            return bfloat16_t(0x0000, from_bits());
        }

        inline static bfloat16_t negative_zero() {
            return bfloat16_t(SIGN_MASK, from_bits());
        }

        inline static bfloat16_t positive_infinity() {
            return bfloat16_t(POS_INFINITY_BITS, from_bits());
        }

        inline static bfloat16_t negative_infinity() {
            return bfloat16_t(NEG_INFINITY_BITS, from_bits());
        }

        inline static bfloat16_t nan() {
            return bfloat16_t(NAN_BITS, from_bits());
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

    //Arithmetic
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

    inline bfloat16_t& operator|(bfloat16_t& a, const bfloat16_t& b) {
        a.x = a.x | b.x;
        return a;
    }

    inline bfloat16_t& operator&(bfloat16_t& a, const bfloat16_t& b) {
        a.x = a.x & b.x;
        return a;
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

    // The above overloaded operators can be templatised for both operations with other floats and int types.
    // Inital operator overloaded test with float, double, int and int64 only.

    // Overloading comparison operators.

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


    // Mathematical utility functions
    inline bfloat16_t abs(const bfloat16_t& a) {
        if (a.is_nan()) {
            return a;
        }

        return bfloat16_t(a.x & ~bfloat16_t::SIGN_MASK, bfloat16_t::from_bits());
    }

    inline bfloat16_t min(const bfloat16_t& a, const bfloat16_t& b) {
        if (a.is_nan()) {
            return a;
        }

        if (b.is_nan()) {
            return b;
        }

        // Special case for zeros with different signs: IEEE says -0 < +0
        if (a.is_zero() && b.is_zero()) {
            return a.is_negative() ? a : b;
        }

        return (a < b) ? a : b;
    }

    inline bfloat16_t max(const bfloat16_t& a, const bfloat16_t& b) {
        if (a.is_nan()) {
            return a;
        }

        if (b.is_nan()) {
            return b;
        }

        // Special case for zeros with different signs: IEEE says -0 < +0
        if (a.is_zero() && b.is_zero()) {
            return a.is_negative() ? b : a;
        }

        return (a > b) ? a : b;
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
