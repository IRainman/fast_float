#ifndef FASTFLOAT_CONSTEXPR_FEATURE_DETECT_H
#define FASTFLOAT_CONSTEXPR_FEATURE_DETECT_H

#ifdef __has_include
#if __has_include(<version>)
#include <version>
#endif
#endif

// C++14 constexpr
#if defined(_MSC_VER)
#if defined(_MSVC_LANG) && _MSVC_LANG >= 201402L
#define FASTFLOAT_CONSTEXPR14 constexpr
#else
#define FASTFLOAT_CONSTEXPR14
#endif
#elif defined(__cpp_constexpr) && __cpp_constexpr >= 201304L
#define FASTFLOAT_CONSTEXPR14 constexpr
#elif defined(__cplusplus) && __cplusplus >= 201402L
#define FASTFLOAT_CONSTEXPR14 constexpr
#else
#define FASTFLOAT_CONSTEXPR14
#endif

// C++14 variable templates
#if defined(__cpp_variable_templates) && __cpp_variable_templates >= 201304L
#define FASTFLOAT_HAS_VARIABLE_TEMPLATES 1
#elif defined(__cplusplus) && __cplusplus >= 201402L
#define FASTFLOAT_HAS_VARIABLE_TEMPLATES 1
#elif defined(_MSC_FULL_VER) && _MSC_FULL_VER >= 190023918L &&                 \
    defined(_MSVC_LANG) && _MSVC_LANG >= 201402L
#define FASTFLOAT_HAS_VARIABLE_TEMPLATES 1
#else
#define FASTFLOAT_HAS_VARIABLE_TEMPLATES 0
#endif

// C++17 if constexpr
#if defined(__cpp_if_constexpr) && __cpp_if_constexpr >= 201606L
#define FASTFLOAT_CONSTEXPR17 constexpr
#elif defined(__cpp_constexpr) && __cpp_constexpr >= 201603L
#define FASTFLOAT_CONSTEXPR17 constexpr
#elif defined(__cplusplus) && __cplusplus >= 201703L
#define FASTFLOAT_CONSTEXPR17 constexpr
#elif defined(_MSC_VER) && defined(_MSVC_LANG) && _MSVC_LANG >= 201703L
#define FASTFLOAT_CONSTEXPR17 constexpr
#else
#define FASTFLOAT_CONSTEXPR17
#endif

// C++17 inline variables
#if defined(__cpp_inline_variables) && __cpp_inline_variables >= 201606L
#define FASTFLOAT_INLINE_VARIABLE inline constexpr
#elif defined(__cplusplus) && __cplusplus >= 201703L
#define FASTFLOAT_INLINE_VARIABLE inline constexpr
#elif defined(_MSC_VER) && defined(_MSVC_LANG) && _MSVC_LANG >= 201703L
#define FASTFLOAT_INLINE_VARIABLE inline constexpr
#else
#define FASTFLOAT_INLINE_VARIABLE static constexpr
#endif

#if (defined(__cplusplus) && __cplusplus >= 201703L) ||                        \
    (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L)
#define FASTFLOAT_DETAIL_MUST_DEFINE_CONSTEXPR_VARIABLE 0
#else
#define FASTFLOAT_DETAIL_MUST_DEFINE_CONSTEXPR_VARIABLE 1
#endif

// C++20 std::bit_cast
#if defined(__cpp_lib_bit_cast) && __cpp_lib_bit_cast >= 201806L
#define FASTFLOAT_HAS_BIT_CAST 1
#else
#define FASTFLOAT_HAS_BIT_CAST 0
#endif

// std::is_constant_evaluated()
//
// This is a library facility and must be detected independently from
// consteval. Do not use it to infer the C++ language mode.
#if defined(__cpp_lib_is_constant_evaluated) &&                                \
    __cpp_lib_is_constant_evaluated >= 201811L
#define FASTFLOAT_HAS_IS_CONSTANT_EVALUATED 1
#else
#define FASTFLOAT_HAS_IS_CONSTANT_EVALUATED 0
#endif

// C++20 language mode.
//
// _MSVC_LANG is required for MSVC because __cplusplus historically does not
// report the selected language mode there.
#if defined(_MSC_VER)
#if defined(_MSVC_LANG) && _MSVC_LANG >= 202002L
#define FASTFLOAT_CXX20 1
#else
#define FASTFLOAT_CXX20 0
#endif
#else
#if defined(__cplusplus) && __cplusplus >= 202002L
#define FASTFLOAT_CXX20 1
#else
#define FASTFLOAT_CXX20 0
#endif
#endif

// C++20 consteval.
//
// consteval is a language feature. It must be detected independently from
// std::is_constant_evaluated() and from the C++20 constexpr library.
#if FASTFLOAT_CXX20
#if defined(__apple_build_version__) && __apple_build_version__ < 14000029L
#define FASTFLOAT_USE_CONSTEVAL 0 // consteval is broken in Apple clang < 14.
#elif defined(_MSC_VER) && !defined(__clang__) && _MSC_VER < 1940
#define FASTFLOAT_USE_CONSTEVAL                                                \
  0 // consteval is broken in some MSVC2022 versions.
#elif defined(__cpp_consteval) && __cpp_consteval >= 201811L
#define FASTFLOAT_USE_CONSTEVAL 1
#else
#define FASTFLOAT_USE_CONSTEVAL 0
#endif
#else
#define FASTFLOAT_USE_CONSTEVAL 0
#endif

#if FASTFLOAT_USE_CONSTEVAL
#define FASTFLOAT_CONSTEVAL consteval
#else
#define FASTFLOAT_CONSTEVAL
#endif

// C++20 constexpr.
//
// This is deliberately independent from consteval. The functions using
// FASTFLOAT_CONSTEXPR20 also require the corresponding C++20 constexpr
// library facilities.
#if FASTFLOAT_CXX20 && FASTFLOAT_HAS_IS_CONSTANT_EVALUATED &&                  \
    FASTFLOAT_HAS_BIT_CAST && defined(__cpp_lib_constexpr_algorithms) &&       \
    __cpp_lib_constexpr_algorithms >= 201806L
#define FASTFLOAT_CONSTEXPR20 constexpr
#define FASTFLOAT_IS_CONSTEXPR 1
#else
#define FASTFLOAT_CONSTEXPR20
#define FASTFLOAT_IS_CONSTEXPR 0
#endif

// C++20 std::byteswap
#if defined(__cpp_lib_byteswap) && __cpp_lib_byteswap >= 202110L
#define FASTFLOAT_HAS_BYTESWAP 1
#else
#define FASTFLOAT_HAS_BYTESWAP 0
#endif

#if defined(__has_builtin)
#define FASTFLOAT_HAS_BUILTIN(x) __has_builtin(x)
#else
#define FASTFLOAT_HAS_BUILTIN(x) false
#endif

#if defined(FASTFLOAT_ASSUME)
// user provided solution
#elif defined(__cpp_attrubute_assume)
// For support attribute [[assume]] is declared in P1774
#define FASTFLOAT_ASSUME(expr) [[assume(expr)]]
#else
#define FASTFLOAT_ASSUME(expr)
#endif

#endif // FASTFLOAT_CONSTEXPR_FEATURE_DETECT_H
