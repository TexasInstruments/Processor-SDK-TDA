#pragma once

#include <cstring>
#include <type_traits>
#include <cstdint>

namespace floating_point {
    namespace bitcast {
        // C++20 and above may have std::bit_cast
        #if defined(__cpp_lib_bit_cast) && __cpp_lib_bit_cast >= 201806L
        #include <bit>
        #define C7X_HAVE_STD_BIT_CAST 1
        #else
        #define C7X_HAVE_STD_BIT_CAST 0
        #endif

        #if C7X_HAVE_STD_BIT_CAST
        using std::bit_cast;
        #else
        // Implementation of bit_cast for C++11 compatibility
        //
        // This is a less sketchy version of reinterpret_cast.
        //
        // See https://en.cppreference.com/w/cpp/numeric/bit_cast for more
        // information as well as the source of our implementations.
        template <class To, class From>
        typename std::enable_if<
            sizeof(To) == sizeof(From) &&
            std::is_trivially_copyable<From>::value &&
            std::is_trivially_copyable<To>::value,
            To>::type
        bit_cast(const From& src) noexcept {
            static_assert(std::is_trivially_constructible<To>::value,
                        "This implementation of bit_cast requires the destination type to be trivially constructible");
            To dst;
            std::memcpy(&dst, &src, sizeof(To));
            return dst;
        }
        #endif
        #undef C7X_HAVE_STD_BIT_CAST
    } // namespace bitcast

    using bitcast::bit_cast;

    namespace bit_ops {
        inline float fp32_from_bits(uint32_t w) {
            return bitcast::bit_cast<float>(w);
        }

        inline uint32_t fp32_to_bits(float f) {
            return bitcast::bit_cast<uint32_t>(f);
        }
    } // namespace bit_ops
}
