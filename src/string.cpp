#include "string.hpp"

#include <fcntl.h>
#include <io.h>

namespace utf {
constexpr bool is_utf8_1byte(char8_t c) {
  return (c & UTF8_1BYTE_SIGNAL_MASK) == UTF8_1BYTE_SIGNAL;
}

constexpr bool is_utf8_2byte(char8_t c) {
  return (c & UTF8_2BYTE_SIGNAL_MASK) == UTF8_2BYTE_SIGNAL;
}

constexpr bool is_utf8_3byte(char8_t c) {
  return (c & UTF8_3BYTE_SIGNAL_MASK) == UTF8_3BYTE_SIGNAL;
}

constexpr bool is_utf8_4byte(char8_t c) {
  return (c & UTF8_4BYTE_SIGNAL_MASK) == UTF8_4BYTE_SIGNAL;
}

constexpr bool is_utf8_xbyte(char8_t c) {
  return (c & UTF8_XBYTE_SIGNAL_MASK) == UTF8_XBYTE_SIGNAL;
}

constexpr char8_t get_utf8_1byte_data(char8_t c) {
  return c & UTF8_1BYTE_DATA_MASK;
}

constexpr char8_t get_utf8_2byte_data(char8_t c) {
  return c & UTF8_2BYTE_DATA_MASK;
}

constexpr char8_t get_utf8_3byte_data(char8_t c) {
  return c & UTF8_3BYTE_DATA_MASK;
}

constexpr char8_t get_utf8_4byte_data(char8_t c) {
  return c & UTF8_4BYTE_DATA_MASK;
}

constexpr char8_t get_utf8_xbyte_data(char8_t c) {
  return c & UTF8_XBYTE_DATA_MASK;
}

constexpr char8_t get_utf8_byte_arity(char8_t c) {
  if (is_utf8_1byte(c)) {
    return 1;
  }
  if (is_utf8_2byte(c)) {
    return 2;
  }
  if (is_utf8_3byte(c)) {
    return 3;
  }
  if (is_utf8_4byte(c)) {
    return 4;
  }
  return INVALID;
}

constexpr char32_t append_utf8_xbyte_data(char32_t codePoint, char8_t c) {
  return (codePoint << UTF8_XBYTE_DATA_LENGTH) |
         (char32_t)get_utf8_xbyte_data(c);
}

constexpr bool is_utf16_hiword(char16_t c) {
  return (c & UTF16_HIWORD_SIGNAL_MASK) == UTF16_HIWORD_SIGNAL;
}

constexpr bool is_utf16_loword(char16_t c) {
  return (c & UTF16_LOWORD_SIGNAL_MASK) == UTF16_LOWORD_SIGNAL;
}

constexpr char16_t get_utf16_hiword_data(char16_t c) {
  return c & UTF16_HIWORD_DATA_MASK;
}

constexpr char16_t get_utf16_loword_data(char16_t c) {
  return c & UTF16_LOWORD_DATA_MASK;
}

constexpr char32_t concat_utf16_data(char16_t c1, char16_t c2) {
  return (((char32_t)c1 << UTF16_LOWORD_DATA_LENGTH) | (char32_t)c2) + 0x10000;
}

char32_t read_ascii(istream8_t &ifs) {
  char8_t c = ifs.get();
  return ifs.eof() ? END_STREAM : char32_t(c);
}

char32_t read_utf8(istream8_t &ifs) {
  char32_t codePoint = (char32_t)ifs.get();
  if (ifs.eof()) {
    return END_STREAM;
  }

  char8_t arity = get_utf8_byte_arity((char8_t)codePoint);
  switch (arity) {
  case 1:
    codePoint &= UTF8_1BYTE_DATA_MASK;
    break;
  case 2:
    codePoint &= UTF8_2BYTE_DATA_MASK;
    break;
  case 3:
    codePoint &= UTF8_3BYTE_DATA_MASK;
    break;
  case 4:
    codePoint &= UTF8_4BYTE_DATA_MASK;
    break;
  default:
    return INVALID;
  }

  for (char8_t i = 0; i < arity - 1; i++) {
    char8_t c = ifs.get();
    if (ifs.eof() || !is_utf8_xbyte(c)) {
      return INVALID;
    }
    codePoint = append_utf8_xbyte_data(codePoint, get_utf8_xbyte_data(c));
  }

  return codePoint;
}

char32_t read_utf16(istream16_t &ifs) {
  char16_t c1 = ifs.get();
  if (ifs.eof()) {
    return END_STREAM;
  }

  if (is_utf16_hiword(c1)) {
    char16_t c2 = ifs.get();
    if (ifs.eof() || !is_utf16_loword(c2)) {
      return INVALID;
    }
    return concat_utf16_data(c1, c2);
  }
  if (is_utf16_loword(c1)) {
    return INVALID;
  }
  return (char32_t)c1;
}

char32_t read_utf32(istream32_t &ifs) {
  char32_t c = ifs.get();
  return ifs.eof() ? END_STREAM : c;
}

void write_ascii(uint32_t codePoint, ostream8_t &ofs) {
  if (codePoint > INVALID) {
    codePoint = INVALID;
  }
  ofs.put((char8_t)codePoint);
}

void write_utf8(char32_t codePoint, ostream8_t &ofs) {
  if (codePoint > UTF8_4BYTE_MAX) {
    ofs.put(INVALID);
  } else if (codePoint > UTF8_3BYTE_MAX) {
    ofs.put(encode_chunk<char8_t>(codePoint, 3 * UTF8_XBYTE_DATA_LENGTH,
                                  UTF8_4BYTE_DATA_MASK, UTF8_4BYTE_SIGNAL));
    ofs.put(encode_chunk<char8_t>(codePoint, 2 * UTF8_XBYTE_DATA_LENGTH,
                                  UTF8_XBYTE_DATA_MASK, UTF8_XBYTE_SIGNAL));
    ofs.put(encode_chunk<char8_t>(codePoint, 1 * UTF8_XBYTE_DATA_LENGTH,
                                  UTF8_XBYTE_DATA_MASK, UTF8_XBYTE_SIGNAL));
    ofs.put(encode_chunk<char8_t>(codePoint, 0 * UTF8_XBYTE_DATA_LENGTH,
                                  UTF8_XBYTE_DATA_MASK, UTF8_XBYTE_SIGNAL));
  } else if (codePoint > UTF8_2BYTE_MAX) {
    ofs.put(encode_chunk<char8_t>(codePoint, 2 * UTF8_XBYTE_DATA_LENGTH,
                                  UTF8_3BYTE_DATA_MASK, UTF8_3BYTE_SIGNAL));
    ofs.put(encode_chunk<char8_t>(codePoint, 1 * UTF8_XBYTE_DATA_LENGTH,
                                  UTF8_XBYTE_DATA_MASK, UTF8_XBYTE_SIGNAL));
    ofs.put(encode_chunk<char8_t>(codePoint, 0 * UTF8_XBYTE_DATA_LENGTH,
                                  UTF8_XBYTE_DATA_MASK, UTF8_XBYTE_SIGNAL));
  } else if (codePoint > UTF8_1BYTE_MAX) {
    ofs.put(encode_chunk<char8_t>(codePoint, 1 * UTF8_XBYTE_DATA_LENGTH,
                                  UTF8_2BYTE_DATA_MASK, UTF8_2BYTE_SIGNAL));
    ofs.put(encode_chunk<char8_t>(codePoint, 0 * UTF8_XBYTE_DATA_LENGTH,
                                  UTF8_XBYTE_DATA_MASK, UTF8_XBYTE_SIGNAL));
  } else {
    ofs.put(char8_t(codePoint));
  }
}

void write_utf16(char32_t codePoint, ostream16_t &ofs) {
  if (codePoint > UTF16_2WORD_MAX) {
    ofs.put(INVALID);
  } else if (codePoint > UTF16_1WORD_MAX) {
    ofs.put(encode_chunk<char16_t>(codePoint, UTF16_LOWORD_DATA_LENGTH,
                                   UTF16_HIWORD_DATA_MASK,
                                   UTF16_HIWORD_SIGNAL));
    ofs.put(encode_chunk<char16_t>(codePoint, 0, UTF16_LOWORD_DATA_MASK,
                                   UTF16_LOWORD_SIGNAL));
  } else {
    ofs.put(codePoint);
  }
}

void write_utf32(char32_t codePoint, ostream32_t &ofs) { ofs.put(codePoint); }
} // namespace utf