#include "ngram.hpp"

NGramModel::NGramModel(size_t n, const Alphabet<char32_t, uint32_t> &alphabet)
    : n(n), maps({std::unordered_map<uint32_t, Mappee>()}), alphabet(alphabet) {
}

typename NGramModel::State NGramModel::start() {
  return State(n - 1, alphabet.serialize(utf::BEG_STRING));
}

void NGramModel::observe(State state, char32_t c) {
  state.push_back(alphabet.serialize(c));
  size_t i = 0;
  for (size_t j = 0; j < state.size(); j++) {
    uint32_t sc = state[j];
    auto result = maps[i].insert(std::make_pair(sc, Mappee{0, 0}));
    result.first->second.count++;
    i = result.first->second.next;
    if (!i && j != state.size() - 1) {
      i = result.first->second.next = maps.size();
      maps.push_back(Map());
    }
  }
}

typename NGramModel::State NGramModel::step(State state, char32_t c) {
  state.push_back(alphabet.serialize(c));
  state.erase(state.begin());
  return state;
}

std::unordered_map<char32_t, double> NGramModel::probs(State state) {
  std::vector<double> probs(alphabet.size(), 1.0 / alphabet.size());
  for (size_t i = 0; i < n; i++) {
    Map *map = &maps[0];
    for (size_t j = i; j < n - 1; j++) {
      auto result = map->find(state[j]);
      if (result == map->end()) {
        map = nullptr;
        break;
      } else {
        map = &maps[result->second.next];
      }
    }
    if (map) {
      size_t total = map_total(*map);
      for (uint32_t j = 0; j < alphabet.size(); j++) {
        auto result = map->find(j);
        if (result == map->end()) {
          probs[j] = 0.01 / ((double)total + 0.01 * alphabet.size());
        } else {
          probs[j] = ((double)result->second.count + 0.01) /
                     ((double)total + 0.01 * alphabet.size());
        }
      }
      break;
    }
  }

  std::unordered_map<char32_t, double> out;
  for (uint32_t i = 0; i < probs.size(); i++) {
    out.insert_or_assign(alphabet.deserialize(i), probs[i]);
  }
  return out;
}

size_t NGramModel::map_total(const typename NGramModel::Map &map) {
  size_t out = 0;
  for (auto it = map.begin(); it != map.end(); it++) {
    out += it->second.count;
  }
  return out;
}