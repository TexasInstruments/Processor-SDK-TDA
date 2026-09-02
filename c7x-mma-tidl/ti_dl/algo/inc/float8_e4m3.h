#pragma once

/// Defines the float8_e4m3_t type (8-bit floating-point) including conversions
/// to standard C types and basic arithmetic operations. Note that arithmetic
/// operations are implemented by converting to floating point and
/// performing the operation in float32.
/// Binary configuration:
/// s eeee mmm
/// 1 sign bit
/// 4 exponent bits
/// 3 mantissa bits
/// bias = 7
///
/// Implementation based on the paper https://arxiv.org/pdf/2209.05433.pdf
/// and inspired by Half implementation from pytorch/fp8/util/Half.h


#include <cmath>
#include <cstdint>
#include <climits>
#include <iostream>
#include "floating_point_utils.h"

namespace floating_point {
    
    struct alignas(1) float8_e4m3_t {
        uint8_t x;
        
        float8_e4m3_t() = default;
        
        struct from_bits_t {};
        static constexpr from_bits_t from_bits() {
            return from_bits_t();
        }
        
        constexpr float8_e4m3_t(uint8_t bits, from_bits_t)
        : x(bits) {}
        inline float8_e4m3_t(float value);
        inline operator float() const;
        inline bool is_nan() const;

        inline float8_e4m3_t& operator=(const float8_e4m3_t& other) {
            x = (other.x == 0xFF) ? 0x7F : other.x;
            return *this;
        }
    };
    
    inline std::ostream& operator<<(std::ostream& out, const float8_e4m3_t& value) {
        out << (float)value;
        return out;
    }
    
    namespace bit_ops {
        
        /*
        * Convert a 8-bit floating-point number in fp8 E4M3FN format, in bit
        * representation, to a 32-bit floating-point number in IEEE single-precision
        * format, in bit representation.
        *
        * @note The implementation doesn't use any floating-point operations.
        */
        inline float fp8e4m3fn_to_fp32_value(uint8_t input) {
            /*
            * Extend the fp8 E4M3FN number to 32 bits and shift to the
            * upper part of the 32-bit word:
            *      +---+----+---+-----------------------------+
            *      | S |EEEE|MMM|0000 0000 0000 0000 0000 0000|
            *      +---+----+---+-----------------------------+
            * Bits  31 27-30 24-26          0-23
            *
            * S - sign bit, E - bits of the biased exponent, M - bits of the mantissa, 0
            * - zero bits.
            */
            const uint32_t w = (uint32_t)input << 24;
            /*
            * Extract the sign of the input number into the high bit of the 32-bit word:
            *
            *      +---+----------------------------------+
            *      | S |0000000 00000000 00000000 00000000|
            *      +---+----------------------------------+
            * Bits  31                 0-31
            */
            const uint32_t sign = w & UINT32_C(0x80000000);
            /*
            * Extract mantissa and biased exponent of the input number into the bits 0-30
            * of the 32-bit word:
            *
            *      +---+----+---+-----------------------------+
            *      | S |EEEE|MMM|0000 0000 0000 0000 0000 0000|
            *      +---+----+---+-----------------------------+
            * Bits  31  27-30 24-26      0-23
            */
            const uint32_t nonsign = w & UINT32_C(0x7FFFFFFF);
            /*
            * Renorm shift is the number of bits to shift mantissa left to make the
            * half-precision number normalized. If the initial number is normalized, some
            * of its high 5 bits (sign == 0 and 4-bit exponent) equals one. In this case
            * renorm_shift == 0. If the number is denormalize, renorm_shift > 0. Note
            * that if we shift denormalized nonsign by renorm_shift, the unit bit of
            * mantissa will shift into exponent, turning the biased exponent into 1, and
            * making mantissa normalized (i.e. without leading 1).
            */
            // Note: zero is not a supported input into `__builtin_clz`
            uint32_t renorm_shift =
            nonsign != 0 ? __builtin_clz(nonsign) : sizeof(uint32_t) * CHAR_BIT;
            renorm_shift = renorm_shift > 4 ? renorm_shift - 4 : 0;
            /*
            * Iff fp8e4m3fn number has all exponent and mantissa bits set to 1,
            * the addition overflows it into bit 31, and the subsequent shift turns the
            * high 9 bits into 1. Thus inf_nan_mask == 0x7F800000 if the fp8e4m3fn number
            * is Nan, 0x00000000 otherwise
            */
            const int32_t inf_nan_mask =
            ((int32_t)(nonsign + 0x01000000) >> 8) & INT32_C(0x7F800000);
            /*
            * Iff nonsign is 0, it overflows into 0xFFFFFFFF, turning bit 31
            * into 1. Otherwise, bit 31 remains 0. The signed shift right by 31
            * broadcasts bit 31 into all bits of the zero_mask. Thus zero_mask ==
            * 0xFFFFFFFF if the half-precision number was zero (+0.0h or -0.0h)
            * 0x00000000 otherwise
            */
            const int32_t zero_mask = (int32_t)(nonsign - 1) >> 31;
            /*
            * 1. Shift nonsign left by renorm_shift to normalize it (if the input
            * was denormal)
            * 2. Shift nonsign right by 4 so the exponent (4 bits originally)
            * becomes an 8-bit field and 3-bit mantissa shifts into the 3 high
            * bits of the 23-bit mantissa of IEEE single-precision number.
            * 3. Add 0x78 to the exponent (starting at bit 23) to compensate the
            * different in exponent bias (0x7F for single-precision number less 0x07
            * for fp8e4m3fn number).
            * 4. Subtract renorm_shift from the exponent (starting at bit 23) to
            * account for renormalization. As renorm_shift is less than 0x78, this
            * can be combined with step 3.
            * 5. Binary OR with inf_nan_mask to turn the exponent into 0xFF if the
            * input was NaN or infinity.
            * 6. Binary ANDNOT with zero_mask to turn the mantissa and exponent
            * into zero if the input was zero.
            * 7. Combine with the sign of the input number.
            */
            uint32_t result = sign |
            ((((nonsign << renorm_shift >> 4) + ((0x78 - renorm_shift) << 23)) |
            inf_nan_mask) &
            ~zero_mask);
            return fp32_from_bits(result);
        }
        
        /*
        * Convert a 32-bit floating-point number in IEEE single-precision format to a
        * 8-bit floating-point number in fp8 E4M3FN format, in bit representation.
        */
        inline uint8_t fp8e4m3fn_from_fp32_value(float f) {
            /*
            * A mask for converting fp32 numbers lower than fp8e4m3fn normal range
            * into denorm representation
            * magic number: ((127 - 7) + (23 - 3) + 1)
            */
            constexpr uint32_t denorm_mask = UINT32_C(141) << 23;

            uint32_t f_bits = fp32_to_bits(f);

            /*
            * Rule (b): any NaN input produces +NaN (0x7F) — sign canonicalized.
            * IEEE NaN: exponent = 0xFF and mantissa != 0, i.e. abs(bits) > 0x7F800000.
            */
            if ((f_bits & UINT32_C(0x7FFFFFFF)) > UINT32_C(0x7F800000)) {
                return 0x7F;
            }

            /*
            * Extract the sign of the input number into the high bit of the 32-bit word:
            *
            *      +---+----------------------------------+
            *      | S |0000000 00000000 00000000 00000000|
            *      +---+----------------------------------+
            * Bits  31                 0-31
            */
            const uint32_t sign = f_bits & UINT32_C(0x80000000);

            /*
            * Set sign bit to 0 — work on absolute value from here.
            */
            f_bits ^= sign;

            /*
            * Rule (a): SAT mode — any finite value > 448.0f saturates to ±448 (0x7E).
            * 448.0f in fp32 bits = 0x43E00000.
            * OVF mode (PyTorch default) would map [464, inf) to NaN — hardware does NOT do this.
            */
            constexpr uint32_t fp8_sat = UINT32_C(0x43E00000);  /* 448.0f */
            if (f_bits > fp8_sat) {
                return static_cast<uint8_t>(sign >> 24) | 0x7E;  /* ±448 */
            }

            uint8_t result = 0u;

            if (f_bits < (UINT32_C(121) << 23)) {
                // Input number is smaller than 2^(-6), which is the smallest
                // fp8e4m3fn normal number — handle subnormal range via denorm trick
                f_bits =
                fp32_to_bits(fp32_from_bits(f_bits) + fp32_from_bits(denorm_mask));
                result = static_cast<uint8_t>(f_bits - denorm_mask);
            } else {
                // resulting mantissa is odd
                uint8_t mant_odd = (f_bits >> 20) & 1;

                // update exponent, rounding bias part 1
                f_bits += ((uint32_t)(7 - 127) << 23) + 0x7FFFF;

                // rounding bias part 2
                f_bits += mant_odd;

                // take the bits!
                result = static_cast<uint8_t>(f_bits >> 20);
            }

            /*
            * Rule (b): -0 is forced to +0 — do not apply sign to a zero result.
            */
            if (result == 0x00) {
                return 0x00;
            }

            result |= static_cast<uint8_t>(sign >> 24);
            return result;
        }
        
    } // namespace bit_ops
    
    /// Constructors
    
    inline float8_e4m3_t::float8_e4m3_t(float value)
    : x(bit_ops::fp8e4m3fn_from_fp32_value(value)) {}
    
    /// Implicit conversions
    
    inline float8_e4m3_t::operator float() const {
        return bit_ops::fp8e4m3fn_to_fp32_value(x);
    }
    
    /// Special values helper
    
    inline bool float8_e4m3_t::is_nan() const {
        return (x & 0b01111111) == 0b01111111;
    }
    
    /// Arithmetic
    
    inline float8_e4m3_t operator+(const float8_e4m3_t& a, const float8_e4m3_t& b) {
        return static_cast<float>(a) + static_cast<float>(b);
    }
    
    inline float8_e4m3_t operator-(const float8_e4m3_t& a, const float8_e4m3_t& b) {
        return static_cast<float>(a) - static_cast<float>(b);
    }
    
    inline float8_e4m3_t operator*(const float8_e4m3_t& a, const float8_e4m3_t& b) {
        return static_cast<float>(a) * static_cast<float>(b);
    }
    
    inline float8_e4m3_t operator/(const float8_e4m3_t& a, const float8_e4m3_t& b) {
        float b_float = static_cast<float>(b);
        if (b_float == 0.0f) {
            return float8_e4m3_t(0x7F, float8_e4m3_t::from_bits());  /* ÷0 → +NaN */
        }
        return static_cast<float>(a) / b_float;
    }
        
    inline float8_e4m3_t operator-(const float8_e4m3_t& a) {
        return -static_cast<float>(a);
    }
    
    inline float8_e4m3_t& operator+=(float8_e4m3_t& a, const float8_e4m3_t& b) {
        a = a + b;
        return a;
    }
        
    inline float8_e4m3_t& operator-=(float8_e4m3_t& a, const float8_e4m3_t& b) {
        a = a - b;
        return a;
    }
        
    inline float8_e4m3_t& operator*=(float8_e4m3_t& a, const float8_e4m3_t& b) {
        a = a * b;
        return a;
    }
        
    inline float8_e4m3_t& operator/=(float8_e4m3_t& a, const float8_e4m3_t& b) {
        a = a / b;
        return a;
    }
                    
    /// Arithmetic with floats
    
    inline float operator+(float8_e4m3_t a, float b) {
        return static_cast<float>(a) + b;
    }
    inline float operator-(float8_e4m3_t a, float b) {
        return static_cast<float>(a) - b;
    }
    inline float operator*(float8_e4m3_t a, float b) {
        return static_cast<float>(a) * b;
    }
    inline float operator/(float8_e4m3_t a, float b) {
        return static_cast<float>(a) / b;
    }
                    
    inline float operator+(float a, float8_e4m3_t b) {
        return a + static_cast<float>(b);
    }
    inline float operator-(float a, float8_e4m3_t b) {
        return a - static_cast<float>(b);
    }
    inline float operator*(float a, float8_e4m3_t b) {
        return a * static_cast<float>(b);
    }
    inline float operator/(float a, float8_e4m3_t b) {
        return a / static_cast<float>(b);
    }
    
    inline float& operator+=(float& a, const float8_e4m3_t& b) {
        return a += static_cast<float>(b);
    }
    inline float& operator-=(float& a, const float8_e4m3_t& b) {
        return a -= static_cast<float>(b);
    }
    inline float& operator*=(float& a, const float8_e4m3_t& b) {
        return a *= static_cast<float>(b);
    }
    inline float& operator/=(float& a, const float8_e4m3_t& b) {
        return a /= static_cast<float>(b);
    }
                    
    /// Arithmetic with doubles
    
    inline double operator+(float8_e4m3_t a, double b) {
        return static_cast<double>(a) + b;
    }
    inline double operator-(float8_e4m3_t a, double b) {
        return static_cast<double>(a) - b;
    }
    inline double operator*(float8_e4m3_t a, double b) {
        return static_cast<double>(a) * b;
    }
    inline double operator/(float8_e4m3_t a, double b) {
        return static_cast<double>(a) / b;
    }
    
    inline double operator+(double a, float8_e4m3_t b) {
        return a + static_cast<double>(b);
    }
    inline double operator-(double a, float8_e4m3_t b) {
        return a - static_cast<double>(b);
    }
    inline double operator*(double a, float8_e4m3_t b) {
        return a * static_cast<double>(b);
    }
    inline double operator/(double a, float8_e4m3_t b) {
        return a / static_cast<double>(b);
    }
                    
    /// Arithmetic with ints
    
    inline float8_e4m3_t operator+(float8_e4m3_t a, int b) {
        return a + static_cast<float8_e4m3_t>(b);
    }
    inline float8_e4m3_t operator-(float8_e4m3_t a, int b) {
        return a - static_cast<float8_e4m3_t>(b);
    }
    inline float8_e4m3_t operator*(float8_e4m3_t a, int b) {
        return a * static_cast<float8_e4m3_t>(b);
    }
    inline float8_e4m3_t operator/(float8_e4m3_t a, int b) {
        return a / static_cast<float8_e4m3_t>(b);
    }
                    
    inline float8_e4m3_t operator+(int a, float8_e4m3_t b) {
        return static_cast<float8_e4m3_t>(a) + b;
    }
    inline float8_e4m3_t operator-(int a, float8_e4m3_t b) {
        return static_cast<float8_e4m3_t>(a) - b;
    }
    inline float8_e4m3_t operator*(int a, float8_e4m3_t b) {
        return static_cast<float8_e4m3_t>(a) * b;
    }
    inline float8_e4m3_t operator/(int a, float8_e4m3_t b) {
        return static_cast<float8_e4m3_t>(a) / b;
    }
                    
    //// Arithmetic with int64_t
    
    inline float8_e4m3_t operator+(float8_e4m3_t a, int64_t b) {
        return a + static_cast<float8_e4m3_t>(b);
    }
    inline float8_e4m3_t operator-(float8_e4m3_t a, int64_t b) {
        return a - static_cast<float8_e4m3_t>(b);
    }
    inline float8_e4m3_t operator*(float8_e4m3_t a, int64_t b) {
        return a * static_cast<float8_e4m3_t>(b);
    }
    inline float8_e4m3_t operator/(float8_e4m3_t a, int64_t b) {
        return a / static_cast<float8_e4m3_t>(b);
    }
    
    inline float8_e4m3_t operator+(int64_t a, float8_e4m3_t b) {
        return static_cast<float8_e4m3_t>(a) + b;
    }
    inline float8_e4m3_t operator-(int64_t a, float8_e4m3_t b) {
        return static_cast<float8_e4m3_t>(a) - b;
    }
    inline float8_e4m3_t operator*(int64_t a, float8_e4m3_t b) {
        return static_cast<float8_e4m3_t>(a) * b;
    }
    inline float8_e4m3_t operator/(int64_t a, float8_e4m3_t b) {
        return static_cast<float8_e4m3_t>(a) / b;
    }

    // The above overloaded operators can be templatised for both operations with other floats and int types.
    // Inital operator overloaded test with float, double, int and int64 only.
                    
    /// NOTE: we do not define comparisons directly and instead rely on the implicit
    /// conversion from floating_point::float8_e4m3_t to float.
    
    // fp8_CLANG_DIAGNOSTIC_POP()
    
} // namespace floating_point
                
                
namespace std {
    
    template <>
    class numeric_limits<floating_point::float8_e4m3_t> {
        public:
        static constexpr bool is_specialized = true;
        static constexpr bool is_signed = true;
        static constexpr bool is_integer = false;
        static constexpr bool is_exact = false;
        static constexpr bool has_infinity = false;
        static constexpr bool has_quiet_nan = true;
        static constexpr bool has_signaling_nan = false;
        static constexpr auto has_denorm = true;
        static constexpr auto has_denorm_loss = true;
        static constexpr auto round_style = numeric_limits<float>::round_style;
        static constexpr bool is_iec559 = false;
        static constexpr bool is_bounded = true;
        static constexpr bool is_modulo = false;
        static constexpr int digits = 4;
        static constexpr int digits10 = 0;
        static constexpr int max_digits10 = 3;
        static constexpr int radix = 2;
        static constexpr int min_exponent = -5;
        static constexpr int min_exponent10 = -1;
        static constexpr int max_exponent = 8;
        static constexpr int max_exponent10 = 2;
        static constexpr auto traps = numeric_limits<float>::traps;
        static constexpr auto tinyness_before = false;
        
        static constexpr floating_point::float8_e4m3_t min() {
            return floating_point::float8_e4m3_t(0x08, floating_point::float8_e4m3_t::from_bits());
        }
        static constexpr floating_point::float8_e4m3_t lowest() {
            return floating_point::float8_e4m3_t(0xFE, floating_point::float8_e4m3_t::from_bits());
        }
        static constexpr floating_point::float8_e4m3_t max() {
            return floating_point::float8_e4m3_t(0x7E, floating_point::float8_e4m3_t::from_bits());
        }
        static constexpr floating_point::float8_e4m3_t epsilon() {
            return floating_point::float8_e4m3_t(0x20, floating_point::float8_e4m3_t::from_bits());
        }
        static constexpr floating_point::float8_e4m3_t round_error() {
            return floating_point::float8_e4m3_t(0x30, floating_point::float8_e4m3_t::from_bits());
        }
        static constexpr floating_point::float8_e4m3_t quiet_nan() {
            return floating_point::float8_e4m3_t(0x7F, floating_point::float8_e4m3_t::from_bits());
        }
        static constexpr floating_point::float8_e4m3_t denorm_min() {
            return floating_point::float8_e4m3_t(0x01, floating_point::float8_e4m3_t::from_bits());
        }
    };
    
} // namespace std