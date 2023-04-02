#ifndef FASTFLOAT_ASCII_NUMBER_H
#define FASTFLOAT_ASCII_NUMBER_H

#include <cctype>
#include <cstdint>
#include <cstring>
#include <iterator>

#include "float_common.h"

namespace fast_float {

// Next function can be micro-optimized, but compilers are entirely
// able to optimize it well.
template <typename TCH>
fastfloat_really_inline constexpr bool is_integer(TCH c) noexcept {
  return !(c > TCH('9') || c < TCH('0'));
}

fastfloat_really_inline constexpr uint64_t byteswap(uint64_t val) {
  return (val & 0xFF00000000000000) >> 56
    | (val & 0x00FF000000000000) >> 40
    | (val & 0x0000FF0000000000) >> 24
    | (val & 0x000000FF00000000) >> 8
    | (val & 0x00000000FF000000) << 8
    | (val & 0x0000000000FF0000) << 24
    | (val & 0x000000000000FF00) << 40
    | (val & 0x00000000000000FF) << 56;
}
template <typename TCH>
fastfloat_really_inline FASTFLOAT_CONSTEXPR20
uint64_t read_u64(TCH const * chars) {
  if (cpp20_and_in_constexpr() || sizeof(TCH) > 1) {
    uint64_t val{};
    for(int i = 0; i < 8; ++i) {
      val |= uint64_t(char(*chars)) << (i * 8);
      ++chars;
    }
    return val;
  }
  uint64_t val;
  ::memcpy(&val, chars, sizeof(uint64_t));
#if FASTFLOAT_IS_BIG_ENDIAN == 1
  // Need to read as-if the number was in little-endian order.
  val = byteswap(val);
#endif
  return val;
}

fastfloat_really_inline FASTFLOAT_CONSTEXPR20
void write_u64(uint8_t *chars, uint64_t val) {
  if (cpp20_and_in_constexpr()) {
    for(int i = 0; i < 8; ++i) {
      *chars = uint8_t(val);
      val >>= 8;
      ++chars;
    }
    return;
  }
#if FASTFLOAT_IS_BIG_ENDIAN == 1
  // Need to read as-if the number was in little-endian order.
  val = byteswap(val);
#endif
  ::memcpy(chars, &val, sizeof(uint64_t));
}

// credit  @aqrit
fastfloat_really_inline FASTFLOAT_CONSTEXPR14
uint32_t parse_eight_digits_unrolled(uint64_t val) {
  const uint64_t mask = 0x000000FF000000FF;
  const uint64_t mul1 = 0x000F424000000064; // 100 + (1000000ULL << 32)
  const uint64_t mul2 = 0x0000271000000001; // 1 + (10000ULL << 32)
  val -= 0x3030303030303030;
  val = (val * 10) + (val >> 8); // val = (val * 2561) >> 8;
  val = (((val & mask) * mul1) + (((val >> 16) & mask) * mul2)) >> 32;
  return uint32_t(val);
}
template <typename TCH>
fastfloat_really_inline FASTFLOAT_CONSTEXPR20
uint32_t parse_eight_digits_unrolled(TCH const * chars)  noexcept  {
  return parse_eight_digits_unrolled(read_u64(chars));
}

// credit @aqrit
fastfloat_really_inline constexpr bool is_made_of_eight_digits_fast(uint64_t val)  noexcept  {
  return !((((val + 0x4646464646464646) | (val - 0x3030303030303030)) &
     0x8080808080808080));
}

template <typename TCH>
fastfloat_really_inline FASTFLOAT_CONSTEXPR20
bool is_made_of_eight_digits_fast(TCH const * chars)  noexcept  {
  return is_made_of_eight_digits_fast(read_u64(chars));
}

template <typename TCH>
struct parsed_number_string_t {
  int64_t exponent{0};
  uint64_t mantissa{0};
  TCH const * lastmatch{nullptr};
  bool negative{false};
  bool valid{false};
  bool too_many_digits{false};
  // contains the range of the significant digits
  span<TCH> integer{};  // non-nullable
  span<TCH> fraction{}; // nullable
};
using byte_span = span<char>;
//using parsed_number_string = parsed_number_string_t<char>;
// Assuming that you use no more than 19 digits, this will
// parse an ASCII string.
template <typename TCH>
fastfloat_really_inline FASTFLOAT_CONSTEXPR20
parsed_number_string_t<TCH> parse_number_string(TCH const *p, TCH const * pend, parse_options_t<TCH> options) noexcept {
  chars_format const fmt = options.format;
  TCH const decimal_point = options.decimal_point;

  parsed_number_string_t<TCH> answer;
  answer.valid = false;
  answer.too_many_digits = false;
  answer.negative = (*p == TCH('-'));
#if FASTFLOAT_ALLOWS_LEADING_PLUS // disabled by default
  if ((*p == TCH('-')) || (*p == TCH('+'))) {
#else
  if (*p == TCH('-')) { // C++17 20.19.3.(7.1) explicitly forbids '+' sign here
#endif
    ++p;
    if (p == pend) {
      return answer;
    }
    if (!is_integer(*p) && (*p != decimal_point)) { // a sign must be followed by an integer or the dot
      return answer;
    }
  }
  TCH const * const start_digits = p;

  uint64_t i = 0; // an unsigned int avoids signed overflows (which are bad)

  while ((p != pend) && is_integer(*p)) {
    // a multiplication by 10 is cheaper than an arbitrary integer
    // multiplication
    i = 10 * i +
        uint64_t(*p - TCH('0')); // might overflow, we will handle the overflow later
    ++p;
  }
  TCH const * const end_of_integer_part = p;
  int64_t digit_count = int64_t(end_of_integer_part - start_digits);
  answer.integer = span<TCH>(start_digits, size_t(digit_count));
  int64_t exponent = 0;
  if ((p != pend) && (*p == decimal_point)) {
    ++p;
    TCH const * before = p;
    // can occur at most twice without overflowing, but let it occur more, since
    // for integers with many digits, digit parsing is the primary bottleneck.
    while ((std::distance(p, pend) >= 8) && is_made_of_eight_digits_fast(p)) {
      i = i * 100000000 + parse_eight_digits_unrolled(p); // in rare cases, this will overflow, but that's ok
      p += 8;
    }
    while ((p != pend) && is_integer(*p)) {
      uint8_t digit = uint8_t(*p - TCH('0'));
      ++p;
      i = i * 10 + digit; // in rare cases, this will overflow, but that's ok
    }
    exponent = before - p;
    answer.fraction = span<TCH>(before, size_t(p - before));
    digit_count -= exponent;
  }
  // we must have encountered at least one integer!
  if (digit_count == 0) {
    return answer;
  }
  int64_t exp_number = 0;            // explicit exponential part
  if ((fmt & chars_format::scientific) && (p != pend) && ((TCH('e') == *p) || (TCH('E') == *p))) {
    TCH const * location_of_e = p;
    ++p;
    bool neg_exp = false;
    if ((p != pend) && (TCH('-') == *p)) {
      neg_exp = true;
      ++p;
    } else if ((p != pend) && (TCH('+') == *p)) { // '+' on exponent is allowed by C++17 20.19.3.(7.1)
      ++p;
    }
    if ((p == pend) || !is_integer(*p)) {
      if(!(fmt & chars_format::fixed)) {
        // We are in error.
        return answer;
      }
      // Otherwise, we will be ignoring the 'e'.
      p = location_of_e;
    } else {
      while ((p != pend) && is_integer(*p)) {
        uint8_t digit = uint8_t(*p - TCH('0'));
        if (exp_number < 0x10000000) {
          exp_number = 10 * exp_number + digit;
        }
        ++p;
      }
      if(neg_exp) { exp_number = - exp_number; }
      exponent += exp_number;
    }
  } else {
    // If it scientific and not fixed, we have to bail out.
    if((fmt & chars_format::scientific) && !(fmt & chars_format::fixed)) { return answer; }
  }
  answer.lastmatch = p;
  answer.valid = true;

  // If we frequently had to deal with long strings of digits,
  // we could extend our code by using a 128-bit integer instead
  // of a 64-bit integer. However, this is uncommon.
  //
  // We can deal with up to 19 digits.
  if (digit_count > 19) { // this is uncommon
    // It is possible that the integer had an overflow.
    // We have to handle the case where we have 0.0000somenumber.
    // We need to be mindful of the case where we only have zeroes...
    // E.g., 0.000000000...000.
    TCH const * start = start_digits;
    while ((start != pend) && (*start == TCH('0') || *start == decimal_point)) {
      if(*start == TCH('0')) { digit_count --; }
      start++;
    }
    if (digit_count > 19) {
      answer.too_many_digits = true;
      // Let us start again, this time, avoiding overflows.
      // We don't need to check if is_integer, since we use the
      // pre-tokenized spans from above.
      i = 0;
      p = answer.integer.ptr;
      TCH const * int_end = p + answer.integer.len();
      const uint64_t minimal_nineteen_digit_integer{1000000000000000000};
      while((i < minimal_nineteen_digit_integer) && (p != int_end)) {
        i = i * 10 + uint64_t(*p - TCH('0'));
        ++p;
      }
      if (i >= minimal_nineteen_digit_integer) { // We have a big integers
        exponent = end_of_integer_part - p + exp_number;
      } else { // We have a value with a fractional component.
          p = answer.fraction.ptr;
          TCH const * frac_end = p + answer.fraction.len();
          while((i < minimal_nineteen_digit_integer) && (p != frac_end)) {
            i = i * 10 + uint64_t(*p - TCH('0'));
            ++p;
          }
          exponent = answer.fraction.ptr - p + exp_number;
      }
      // We have now corrected both exponent and i, to a truncated value
    }
  }
  answer.exponent = exponent;
  answer.mantissa = i;
  return answer;
}

} // namespace fast_float

#endif
