#ifndef FASTFLOAT_PARSE_NUMBER_H
#define FASTFLOAT_PARSE_NUMBER_H

#include "ascii_number.h"
#include "decimal_to_binary.h"
#include "digit_comparison.h"
#include "float_common.h"

#include <cmath>
#include <cstring>
#include <limits>
#include <system_error>

namespace fast_float {

namespace detail {
#ifndef FASTFLOAT_ONLY_POSITIVE_C_NUMBER_WO_INF_NAN
/**
 * Special case +inf, -inf, nan, infinity, -infinity.
 * The case comparisons could be made much faster given that we know that the
 * strings a null-free and fixed.
 **/
template <typename T, typename UC>
from_chars_result_t<UC>
    FASTFLOAT_CONSTEXPR14 parse_infnan(UC const *first, UC const *last,
                                       T &value,
                                       const chars_format fmt) noexcept {
  from_chars_result_t<UC> answer{};
  answer.ptr = first;
  answer.ec = std::errc();        // be optimistic
  FASTFLOAT_ASSUME(first < last); // so dereference without checks

  bool const minusSign = (*first == UC('-'));
  // C++17 20.19.3.(7.1) explicitly forbids '+' sign here
  if ((*first == UC('-')) ||
      ((chars_format_t(fmt & chars_format::allow_leading_plus)) &&
       (*first == UC('+')))) {
    ++first;
  }

  if (last - first >= 3) {
    if (fastfloat_strncasecmp(first, str_const_nan<UC>(), 3)) {
      answer.ptr = (first += 3);
      value = minusSign ? -std::numeric_limits<T>::quiet_NaN()
                        : std::numeric_limits<T>::quiet_NaN();
      // Check for possible nan(n-char-seq-opt), C++17 20.19.3.7,
      // C11 7.20.1.3.3. At least MSVC produces nan(ind) and nan(snan).
      if (first != last && *first == UC('(')) {
        for (UC const *ptr = first + 1; ptr != last; ++ptr) {
          if (*ptr == UC(')')) {
            answer.ptr = ptr + 1; // valid nan(n-char-seq-opt)
            break;
          } else if (!((UC('a') <= *ptr && *ptr <= UC('z')) ||
                       (UC('A') <= *ptr && *ptr <= UC('Z')) ||
                       (UC('0') <= *ptr && *ptr <= UC('9')) || *ptr == UC('_')))
            break; // forbidden char, not nan(n-char-seq-opt)
        }
      }
      return answer;
    }
    if (fastfloat_strncasecmp(first, str_const_inf<UC>(), 3)) {
      if ((last - first >= 8) &&
          fastfloat_strncasecmp(first + 3, str_const_inf<UC>() + 3, 5)) {
        answer.ptr = first + 8;
      } else {
        answer.ptr = first + 3;
      }
      value = minusSign ? -std::numeric_limits<T>::infinity()
                        : std::numeric_limits<T>::infinity();
      return answer;
    }
  }
  answer.ec = std::errc::invalid_argument;
  return answer;
}
#endif

#ifndef FASTFLOAT_ONLY_ROUNDS_TO_NEAREST_SUPPORTED
/**
 * Returns true if the floating-pointing rounding mode is to 'nearest'.
 * It is the default on most system. This function is meant to be inexpensive.
 * Credit : @mwalcott3
 */
fastfloat_really_inline bool rounds_to_nearest() noexcept {
  // https://lemire.me/blog/2020/06/26/gcc-not-nearest/
#if (FLT_EVAL_METHOD != 1) && (FLT_EVAL_METHOD != 0)
  return false;
#endif
  // See
  // A fast function to check your floating-point rounding mode
  // https://lemire.me/blog/2022/11/16/a-fast-function-to-check-your-floating-point-rounding-mode/
  //
  // This function is meant to be equivalent to :
  // prior: #include <cfenv>
  //  return fegetround() == FE_TONEAREST;
  // However, it is expected to be much faster than the fegetround()
  // function call.
  //
  // The volatile keyword prevents the compiler from computing the function
  // at compile-time.
  // There might be other ways to prevent compile-time optimizations (e.g.,
  // asm). The value does not need to be std::numeric_limits<float>::min(), any
  // small value so that 1 + x should round to 1 would do (after accounting for
  // excess precision, as in 387 instructions).
  static float volatile fmin = std::numeric_limits<float>::min();
  float fmini = fmin; // we copy it so that it gets loaded at most once.
//
// Explanation:
// Only when fegetround() == FE_TONEAREST do we have that
// fmin + 1.0f == 1.0f - fmin.
//
// FE_UPWARD:
//  fmin + 1.0f > 1
//  1.0f - fmin == 1
//
// FE_DOWNWARD or  FE_TOWARDZERO:
//  fmin + 1.0f == 1
//  1.0f - fmin < 1
//
// Note: This may fail to be accurate if fast-math has been
// enabled, as rounding conventions may not apply.
#ifdef FASTFLOAT_VISUAL_STUDIO
#pragma warning(push)
//  todo: is there a VS warning?
//  see
//  https://stackoverflow.com/questions/46079446/is-there-a-warning-for-floating-point-equality-checking-in-visual-studio-2013
#elif defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfloat-equal"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
#endif
  return (fmini + 1.0f == 1.0f - fmini);
#ifdef FASTFLOAT_VISUAL_STUDIO
#pragma warning(pop)
#elif defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
}
#endif

} // namespace detail

template <typename T> struct from_chars_caller {
  template <typename UC>
  FASTFLOAT_CONSTEXPR20 static from_chars_result_t<UC>
  call(UC const *first, UC const *last, T &value,
       parse_options_t<UC> const &options) noexcept {
    return from_chars_advanced(first, last, value, options);
  }
};

#ifdef __STDCPP_FLOAT32_T__
template <> struct from_chars_caller<std::float32_t> {
  template <typename UC>
  FASTFLOAT_CONSTEXPR20 static from_chars_result_t<UC>
  call(UC const *first, UC const *last, std::float32_t &value,
       parse_options_t<UC> const options) noexcept {
    // if std::float32_t is defined, and we are in C++23 mode; macro set for
    // float32; set value to float due to equivalence between float and
    // float32_t
    float val;
    auto ret = from_chars_advanced(first, last, val, options);
    value = val;
    return ret;
  }
};
#endif

#ifdef __STDCPP_FLOAT64_T__
template <> struct from_chars_caller<std::float64_t> {
  template <typename UC>
  FASTFLOAT_CONSTEXPR20 static from_chars_result_t<UC>
  call(UC const *first, UC const *last, std::float64_t &value,
       parse_options_t<UC> const options) noexcept {
    // if std::float64_t is defined, and we are in C++23 mode; macro set for
    // float64; set value as double due to equivalence between double and
    // float64_t
    double val;
    auto ret = from_chars_advanced(first, last, val, options);
    value = val;
    return ret;
  }
};
#endif

template <typename T, typename UC, typename>
FASTFLOAT_CONSTEXPR20 from_chars_result_t<UC>
from_chars(UC const *first, UC const *last, T &value,
           chars_format const fmt /*= chars_format::general*/) noexcept {
  return from_chars_caller<T>::call(first, last, value,
                                    parse_options_t<UC>(fmt));
}

/**
 * This function overload takes parsed_number_string_t structure that is created
 * and populated either by from_chars_advanced function taking chars range and
 * parsing options or other parsing custom function implemented by user.
 */
template <typename T, typename UC>
FASTFLOAT_CONSTEXPR20 from_chars_result_t<UC>
from_chars_advanced(parsed_number_string_t<UC> const &pns, T &value) noexcept {

  static_assert(is_supported_float_type<T>::value,
                "only some floating-point types are supported");
  static_assert(is_supported_char_type<UC>::value,
                "only char, wchar_t, char16_t and char32_t are supported");

  from_chars_result_t<UC> answer;

  answer.ec = std::errc(); // be optimistic
  answer.ptr = pns.lastmatch;
  // The implementation of the Clinger's fast path is convoluted because
  // we want round-to-nearest in all cases, irrespective of the rounding mode
  // selected on the thread.
  // We proceed optimistically, assuming that detail::rounds_to_nearest()
  // returns true.
  if (binary_format<T>::min_exponent_fast_path() <= pns.exponent &&
      pns.exponent <= binary_format<T>::max_exponent_fast_path() &&
      !pns.too_many_digits) {
    // Unfortunately, the conventional Clinger's fast path is only possible
    // when the system rounds to the nearest float.
    //
    // We expect the next branch to almost always be selected.
    // We could check it first (before the previous branch), but
    // there might be performance advantages at having the check
    // be last.
#ifndef FASTFLOAT_ONLY_ROUNDS_TO_NEAREST_SUPPORTED
    if (!cpp20_and_in_constexpr() && detail::rounds_to_nearest()) {
#endif
      // We have that fegetround() == FE_TONEAREST.
      // Next is Clinger's fast path.
      if (pns.mantissa <= binary_format<T>::max_mantissa_fast_path()) {
        value = T(pns.mantissa);
        if (pns.exponent < 0) {
          value = value / binary_format<T>::exact_power_of_ten(-pns.exponent);
        } else {
          value = value * binary_format<T>::exact_power_of_ten(pns.exponent);
        }
#ifndef FASTFLOAT_ONLY_POSITIVE_C_NUMBER_WO_INF_NAN
        if (pns.negative) {
          value = -value;
        }
#endif
        return answer;
      }
#ifndef FASTFLOAT_ONLY_ROUNDS_TO_NEAREST_SUPPORTED
    } else {
      // We do not have that fegetround() == FE_TONEAREST.
      // Next is a modified Clinger's fast path, inspired by Jakub Jelínek's
      // proposal
      if (pns.exponent >= 0 &&
          pns.mantissa <=
              binary_format<T>::max_mantissa_fast_path(pns.exponent)) {
#if defined(__clang__) || defined(FASTFLOAT_32BIT)
        // Clang may map 0 to -0.0 when fegetround() == FE_DOWNWARD
        if (pns.mantissa == 0) {
          value =
#ifndef FASTFLOAT_ONLY_POSITIVE_C_NUMBER_WO_INF_NAN
              pns.negative ? T(-0.) :
#endif
                           T(0.);
          return answer;
        }
#endif
        value = T(pns.mantissa) *
                binary_format<T>::exact_power_of_ten(pns.exponent);
#ifndef FASTFLOAT_ONLY_POSITIVE_C_NUMBER_WO_INF_NAN
        if (pns.negative) {
          value = -value;
        }
#endif
        return answer;
      }
    }
#endif
  }
  adjusted_mantissa am =
      compute_float<binary_format<T>>(pns.exponent, pns.mantissa);
  if (pns.too_many_digits && am.power2 >= 0) {
    if (am != compute_float<binary_format<T>>(pns.exponent, pns.mantissa + 1)) {
      am = compute_error<binary_format<T>>(pns.exponent, pns.mantissa);
    }
  }
  // If we called compute_float<binary_format<T>>(pns.exponent, pns.mantissa)
  // and we have an invalid power (am.power2 < 0), then we need to go the long
  // way around again. This is very uncommon.
  if (am.power2 < 0) {
    am = digit_comp<T>(pns, am);
  }
  to_float(
#ifndef FASTFLOAT_ONLY_POSITIVE_C_NUMBER_WO_INF_NAN
      pns.negative,
#endif
      am, value);
  // Test for over/underflow.
  if ((pns.mantissa != 0 && am.mantissa == 0 && am.power2 == 0) ||
      am.power2 == binary_format<T>::infinite_power()) {
    answer.ec = std::errc::result_out_of_range;
  }
  return answer;
}

template <typename T, typename UC>
FASTFLOAT_CONSTEXPR20 from_chars_result_t<UC>
from_chars_float_advanced(UC const *first, UC const *last, T &value,
                          parse_options_t<UC> const &options) noexcept {

  static_assert(is_supported_float_type<T>::value,
                "only some floating-point types are supported");
  static_assert(is_supported_char_type<UC>::value,
                "only char, wchar_t, char16_t and char32_t are supported");

  from_chars_result_t<UC> answer;
#ifndef FASTFLOAT_ONLY_POSITIVE_C_NUMBER_WO_INF_NAN
  if (chars_format_t(options.format & chars_format::skip_white_space)) {
    while ((first != last) && fast_float::is_space(*first)) {
      ++first;
    }
  }
  if (first == last) {
    answer.ec = std::errc::invalid_argument;
    answer.ptr = first;
    return answer;
  }
#else
  // We are in parser code with external loop that checks bounds.
  FASTFLOAT_ASSUME(first < last);
#endif
  parsed_number_string_t<UC> const pns =
#ifndef FASTFLOAT_ONLY_POSITIVE_C_NUMBER_WO_INF_NAN
      (chars_format_t(options.format & detail::basic_json_fmt))
          ? parse_number_string<true, UC>(first, last, options)
          :
#endif
          parse_number_string<false, UC>(first, last, options);
  if (pns.invalid) {
#ifndef FASTFLOAT_ONLY_POSITIVE_C_NUMBER_WO_INF_NAN
    if (chars_format_t(options.format & chars_format::no_infnan)) {
#endif
      answer.ec = std::errc::invalid_argument;
      answer.ptr = first;
      return answer;
#ifndef FASTFLOAT_ONLY_POSITIVE_C_NUMBER_WO_INF_NAN
    } else {
      return detail::parse_infnan(first, last, value, options.format);
    }
#endif
  }

  // call overload that takes parsed_number_string_t directly.
  return from_chars_advanced(pns, value);
}

template <typename T, typename UC, typename>
FASTFLOAT_CONSTEXPR20 from_chars_result_t<UC>
from_chars(UC const *first, UC const *last, T &value, int const base) noexcept {

  static_assert(is_supported_integer_type<T>::value,
                "only integer types are supported");
  static_assert(is_supported_char_type<UC>::value,
                "only char, wchar_t, char16_t and char32_t are supported");

  parse_options_t<UC> const options(chars_format::general, UC('.'),
                                    static_cast<uint_fast8_t>(base));
  return from_chars_advanced(first, last, value, options);
}

template <typename T, typename UC>
FASTFLOAT_CONSTEXPR20 from_chars_result_t<UC>
from_chars_int_advanced(UC const *first, UC const *last, T &value,
                        parse_options_t<UC> const &options) noexcept {

  static_assert(is_supported_integer_type<T>::value,
                "only integer types are supported");
  static_assert(is_supported_char_type<UC>::value,
                "only char, wchar_t, char16_t and char32_t are supported");

#ifndef FASTFLOAT_ONLY_POSITIVE_C_NUMBER_WO_INF_NAN
  if (chars_format_t(options.format & chars_format::skip_white_space)) {
    while ((first != last) && fast_float::is_space(*first)) {
      ++first;
    }
  }
  if (first == last || options.base < 2 || options.base > 36) {
    from_chars_result_t<UC> answer;
    answer.ec = std::errc::invalid_argument;
    answer.ptr = first;
    return answer;
  }
#else
  // We are in parser code with external loop that checks bounds.
  FASTFLOAT_ASSUME(first < last);
  // base is already checked in the parse_options_t constructor.
#endif

  return parse_int_string(first, last, value, options);
}

template <size_t TypeIx> struct from_chars_advanced_caller {
  static_assert(TypeIx > 0, "unsupported type");
};

template <> struct from_chars_advanced_caller<1> {
  template <typename T, typename UC>
  FASTFLOAT_CONSTEXPR20 static from_chars_result_t<UC>
  call(UC const *first, UC const *last, T &value,
       parse_options_t<UC> const &options) noexcept {
    return from_chars_float_advanced(first, last, value, options);
  }
};

template <> struct from_chars_advanced_caller<2> {
  template <typename T, typename UC>
  FASTFLOAT_CONSTEXPR20 static from_chars_result_t<UC>
  call(UC const *first, UC const *last, T &value,
       parse_options_t<UC> const &options) noexcept {
    return from_chars_int_advanced(first, last, value, options);
  }
};

template <typename T, typename UC>
FASTFLOAT_CONSTEXPR20 from_chars_result_t<UC>
from_chars_advanced(UC const *first, UC const *last, T &value,
                    parse_options_t<UC> const &options) noexcept {
  return from_chars_advanced_caller<
      size_t(is_supported_float_type<T>::value) +
      2 * size_t(is_supported_integer_type<T>::value)>::call(first, last, value,
                                                             options);
}

} // namespace fast_float

#endif
