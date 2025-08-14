#pragma once

#if __cplusplus >= 202002L || (defined(__cpp_concepts) && defined(__cpp_lib_concepts))
#include <concepts>
#endif

namespace fumen::details::math {

#if __cplusplus >= 202002L || (defined(__cpp_concepts) && defined(__cpp_lib_concepts))
template <std::integral T>
#else
template <typename T>
#endif
constexpr T powi(T _base, T _exp) {
    T _result = 1;

    while (_exp) {
        if (_exp & 1) _result = _result * _base;
        _base = _base * _base;
        _exp >>= 1;
    }

    return _result;
}

}