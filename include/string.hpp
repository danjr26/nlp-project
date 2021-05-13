#ifndef STRING_HPP
#define STRING_HPP

#include <sstream>
#include <string>

using char8_t = char;

using string8_t = std::basic_string<char8_t>;
using string16_t = std::basic_string<char16_t>;
using string32_t = std::basic_string<char32_t>;

using istream8_t = std::basic_istream<char8_t>;
using istream16_t = std::basic_istream<char16_t>;
using istream32_t = std::basic_istream<char32_t>;

using ostream8_t = std::basic_ostream<char8_t>;
using ostream16_t = std::basic_ostream<char16_t>;
using ostream32_t = std::basic_ostream<char32_t>;

using iostream8_t = std::basic_iostream<char8_t>;
using iostream16_t = std::basic_iostream<char16_t>;
using iostream32_t = std::basic_iostream<char32_t>;

using ifstream8_t = std::basic_ifstream<char8_t>;
using ifstream16_t = std::basic_ifstream<char16_t>;
using ifstream32_t = std::basic_ifstream<char32_t>;

using ofstream8_t = std::basic_ofstream<char8_t>;
using ofstream16_t = std::basic_ofstream<char16_t>;
using ofstream32_t = std::basic_ofstream<char32_t>;

using fstream8_t = std::basic_fstream<char8_t>;
using fstream16_t = std::basic_fstream<char16_t>;
using fstream32_t = std::basic_fstream<char32_t>;

using sstream8_t = std::basic_stringstream<char8_t>;
using sstream16_t = std::basic_stringstream<char16_t>;
using sstream32_t = std::basic_stringstream<char32_t>;

namespace utf {
const char32_t BEG_STRING = 0x02; // U+0002 = Start of Text
const char32_t END_STRING = 0x03; // U+0003 = End of Text
const char32_t END_STREAM = 0x00; // U+0000 = Null
const char32_t UNKNOWN = 0x1a;    // U+001a = Substitute
const char32_t INVALID = 0x7f;    // U+007F = Delete

const char8_t UTF8_1BYTE_SIGNAL_MASK = 0x80; // 10000000
const char8_t UTF8_1BYTE_SIGNAL = 0x00;      // 00000000
const char8_t UTF8_1BYTE_DATA_MASK = 0x7f;   // 01111111
const char8_t UTF8_1BYTE_DATA_LENGTH = 7;    //
const char32_t UTF8_1BYTE_MAX = UTF8_1BYTE_DATA_MASK;
const char8_t UTF8_2BYTE_SIGNAL_MASK = 0xe0; // 11100000
const char8_t UTF8_2BYTE_SIGNAL = 0xc0;      // 11000000
const char8_t UTF8_2BYTE_DATA_MASK = 0x1f;   // 00011111
const char8_t UTF8_2BYTE_DATA_LENGTH = 5;    //
const char32_t UTF8_2BYTE_MAX =
    (UTF8_1BYTE_MAX << UTF8_2BYTE_DATA_LENGTH) | UTF8_2BYTE_DATA_MASK;
const char8_t UTF8_3BYTE_SIGNAL_MASK = 0xf0; // 11110000
const char8_t UTF8_3BYTE_SIGNAL = 0xe0;      // 11100000
const char8_t UTF8_3BYTE_DATA_MASK = 0x0f;   // 00001111
const char8_t UTF8_3BYTE_DATA_LENGTH = 4;    //
const char32_t UTF8_3BYTE_MAX =
    (UTF8_2BYTE_MAX << UTF8_3BYTE_DATA_LENGTH) | UTF8_3BYTE_DATA_MASK;
const char8_t UTF8_4BYTE_SIGNAL_MASK = 0xf8; // 11111000
const char8_t UTF8_4BYTE_SIGNAL = 0xf0;      // 11110000
const char8_t UTF8_4BYTE_DATA_MASK = 0x07;   // 00000111
const char8_t UTF8_4BYTE_DATA_LENGTH = 3;    //
const char32_t UTF8_4BYTE_MAX =
    (UTF8_3BYTE_MAX << UTF8_4BYTE_DATA_LENGTH) | UTF8_4BYTE_DATA_MASK;
const char8_t UTF8_XBYTE_SIGNAL_MASK = 0xc0; // 11000000
const char8_t UTF8_XBYTE_SIGNAL = 0x80;      // 10000000
const char8_t UTF8_XBYTE_DATA_MASK = 0x3f;   // 00111111
const char8_t UTF8_XBYTE_DATA_LENGTH = 6;    //

const char16_t UTF16_HIWORD_SIGNAL_MASK = 0xfc00; // 1111110000000000
const char16_t UTF16_HIWORD_SIGNAL = 0xd800;      // 1101100000000000
const char16_t UTF16_HIWORD_DATA_MASK = 0x03ff;   // 0000001111111111
const char16_t UTF16_HIWORD_DATA_LENGTH = 10;     //
const char16_t UTF16_LOWORD_SIGNAL_MASK = 0xfc00; // 1111110000000000
const char16_t UTF16_LOWORD_SIGNAL = 0xdc00;      // 1101110000000000
const char16_t UTF16_LOWORD_DATA_MASK = 0x03ff;   // 0000001111111111
const char16_t UTF16_LOWORD_DATA_LENGTH = 10;     //
const char32_t UTF16_1WORD_MAX = 0xffff;          // 1111111111111111
const char32_t UTF16_2WORD_MAX = 0x10ffff;        //

constexpr bool is_utf8_1byte(char8_t c);
constexpr bool is_utf8_2byte(char8_t c);
constexpr bool is_utf8_3byte(char8_t c);
constexpr bool is_utf8_4byte(char8_t c);
constexpr bool is_utf8_xbyte(char8_t c);

constexpr char8_t get_utf8_1byte_data(char8_t c);
constexpr char8_t get_utf8_2byte_data(char8_t c);
constexpr char8_t get_utf8_3byte_data(char8_t c);
constexpr char8_t get_utf8_4byte_data(char8_t c);
constexpr char8_t get_utf8_xbyte_data(char8_t c);

constexpr char8_t get_utf8_byte_arity(char8_t c);
constexpr char8_t get_utf8_code_point_arity(char32_t codePoint);
constexpr char32_t append_utf8_xbyte_data(char32_t codePoint, char8_t c);

constexpr bool is_utf16_hiword(char16_t c);
constexpr bool is_utf16_loword(char16_t c);

constexpr char16_t get_utf16_hiword_data(char16_t c);
constexpr char16_t get_utf16_loword_data(char16_t c);

constexpr char8_t get_utf16_code_point_arity(char32_t codePoint);
constexpr char32_t concat_utf16_data(char16_t c1, char16_t c2);

char32_t read_ascii(istream8_t &ifs);
char32_t read_utf8(istream8_t &ifs);
char32_t read_utf16(istream16_t &ifs);
char32_t read_utf32(istream32_t &ifs);

void write_ascii(char32_t codePoint, ostream8_t &ofs);
void write_utf8(char32_t codePoint, ostream8_t &ofs);
void write_utf16(char32_t codePoint, ostream16_t &ofs);
void write_utf32(char32_t codePoint, ostream32_t &ofs);

template <class char_t>
char_t encode_chunk(char32_t codePoint, uint8_t offset, char_t mask,
                    char_t signal) {
  return ((codePoint >> offset) & mask) | signal;
}
} // namespace utf

template <class char_t>
char_t safe_index(const std::basic_string<char_t> &s, int64_t i,
                  char_t beg = (char_t)utf::BEG_STRING,
                  char_t end = (char_t)utf::END_STRING) {
  if (i < 0) {
    return beg;
  }
  if (i >= s.size()) {
    return end;
  }
  return s[i];
}

template <class char_t>
std::basic_string<char_t> safe_substr(const std::basic_string<char_t> &s,
                                      int64_t i, size_t len,
                                      char_t beg = (char_t)utf::BEG_STRING,
                                      char_t end = (char_t)utf::END_STRING) {
  int64_t clippedI = std::max<int64_t>(i, 0);
  size_t clippedLen = std::min<size_t>(len, s.size() - clippedI);
  return std::basic_string<char_t>(clippedI - i, beg) +
         s.substr(clippedI, clippedLen) +
         std::basic_string<char_t>(
             (int64_t)len - (int64_t)clippedLen - clippedI + i, end);
}

#endif