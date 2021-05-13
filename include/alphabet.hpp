#ifndef ALPHABET_HPP
#define ALPHABET_HPP

#include "string.hpp"

#include <cassert>
#include <unordered_map>
#include <unordered_set>

template <class TrueChar = char32_t, class SerialChar = uint32_t>
class Alphabet {
public:
  Alphabet(const std::unordered_set<TrueChar> &tcs);

  SerialChar serialize(TrueChar tc) const;
  TrueChar deserialize(SerialChar sc) const;
  SerialChar size() const;

private:
  std::unordered_map<TrueChar, SerialChar> trueToSerial;
  std::unordered_map<SerialChar, TrueChar> serialToTrue;
};

template <class TrueChar, class SerialChar>
Alphabet<TrueChar, SerialChar>::Alphabet(
    const std::unordered_set<TrueChar> &tcs)
    : trueToSerial(tcs.size() + 3), serialToTrue(tcs.size() + 3) {
  SerialChar count = 0;

  std::vector<TrueChar> xChars = {utf::BEG_STRING, utf::END_STRING,
                                  utf::UNKNOWN};
  for (TrueChar tc : xChars) {
    assert(!tcs.count(tc));
    SerialChar sc = count++;
    trueToSerial.insert_or_assign(tc, sc);
    serialToTrue.insert_or_assign(sc, tc);
  }

  for (TrueChar tc : tcs) {
    SerialChar sc = count++;
    trueToSerial.insert_or_assign(tc, sc);
    serialToTrue.insert_or_assign(sc, tc);
  }
}

template <class TrueChar, class SerialChar>
SerialChar Alphabet<TrueChar, SerialChar>::serialize(TrueChar tc) const {
  auto result = trueToSerial.find(tc);
  if (result == trueToSerial.end()) {
    return trueToSerial.at(utf::UNKNOWN);
  } else {
    return result->second;
  }
}

template <class TrueChar, class SerialChar>
TrueChar Alphabet<TrueChar, SerialChar>::deserialize(SerialChar sc) const {
  return serialToTrue.at(sc);
}

template <class TrueChar, class SerialChar>
SerialChar Alphabet<TrueChar, SerialChar>::size() const {
  assert(serialToTrue.size() == trueToSerial.size());
  return (SerialChar)serialToTrue.size();
}

#endif