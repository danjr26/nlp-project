#ifndef NGRAM_HPP
#define NGRAM_HPP

#include "alphabet.hpp"

#include <memory>
#include <unordered_map>
#include <vector>

class NGramModel {
public:
  using State = std::vector<uint32_t>;

public:
  NGramModel(size_t n, const Alphabet<char32_t, uint32_t> &alphabet);

  State start();
  void observe(State state, char32_t c);
  State step(State state, char32_t c);
  std::unordered_map<char32_t, double> probs(State state);

private:
  struct Mappee {
    size_t count;
    size_t next;
  };

  using Map = std::unordered_map<uint32_t, Mappee>;

private:
  static size_t map_total(const Map &map);

private:
  Alphabet<char32_t, uint32_t> alphabet;
  size_t n;
  std::vector<Map> maps;
};

#endif