#include "custom_net.hpp"
#include "ngram.hpp"
#include "progress.hpp"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>

using corpus_t = std::vector<string32_t>;

template <class M> void train(M &model, const string32_t &s) {
  using State = typename M::State;
  string32_t str = s;
  str.append(1, utf::END_STRING);
  State state = model.start();
  for (char32_t c : str) {
    model.observe(state, c);
    state = model.step(state, c);
  }
}

template <class M>
void train(M &model, const corpus_t &trainCorpus, const corpus_t &valCorpus) {
  Progress pbar(trainCorpus.size());
  for (const string32_t s : trainCorpus) {
    train(model, s);
    pbar.add(1);
  }
}

template <class M> double perplexity(M &model, const string32_t &s) {
  using State = typename M::State;
  string32_t str = s;
  str.append(1, utf::END_STRING);
  double logprob = 0.0;
  State state = model.start();
  for (char32_t c : str) {
    auto probs = model.probs(state);
    double prob = probs.count(c) ? probs.at(c) : probs.at(utf::UNKNOWN);
    logprob -= std::log2(prob);
    state = model.step(state, c);
  }
  logprob /= str.size();
  return std::exp2(logprob);
}

template <class M> double perplexity(M &model, const corpus_t &corpus) {
  Progress pbar(corpus.size());
  double avg = 0.0;
  for (string32_t s : corpus) {
    avg += std::log2(perplexity(model, s));
    pbar.add(1);
  }
  avg /= corpus.size();
  return std::exp2(avg);
}

template <class M> string32_t generate_best(M &model, size_t maxLen) {
  using State = typename M::State;
  string32_t out;
  State state = model.start();
  for (size_t i = 0; i < maxLen; i++) {
    auto probs = model.probs(state);
    auto bestIt =
        std::max_element(probs.begin(), probs.end(),
                         [](const typename decltype(probs)::value_type &v1,
                            const typename decltype(probs)::value_type &v2) {
                           return v1.second < v2.second;
                         });
    char32_t c = bestIt->first;
    if (c == utf::END_STRING) {
      break;
    } else {
      out.append(1, c);
      state = model.step(state, c);
    }
  }
  return out;
}

template <class M> string32_t generate_random(M &model, size_t maxLen) {
  using State = typename M::State;
  std::uniform_real_distribution distribution;
  std::default_random_engine generator(
      std::chrono::system_clock::now().time_since_epoch().count());
  string32_t out;
  State state = model.start();
  for (size_t i = 0; i < maxLen; i++) {
    auto probs = model.probs(state);
    double randN = distribution(generator);
    auto randIt = probs.begin();
    while ((randN -= randIt->second) > 0) {
      randIt++;
    }
    char32_t c = randIt->first;
    if (c == utf::END_STRING) {
      break;
    } else {
      out.append(1, c);
      state = model.step(state, c);
    }
  }
  return out;
}

corpus_t load_corpus(const std::string &filepath,
                     string32_t delim = U"\n#SEP#\n") {
  string32_t s;
  {
    std::ifstream ifs;
    ifs.open(filepath);
    assert(ifs.is_open());
    char32_t codePoint;
    sstream32_t ss;
    while ((codePoint = utf::read_utf8(ifs)) != utf::END_STREAM) {
      utf::write_utf32(codePoint, ss);
    }
    ifs.close();
    s = ss.str();
  }

  std::vector<string32_t> out;
  {
    size_t oldPos = 0;
    size_t newPos = 0;
    while ((newPos = s.find(delim, oldPos)) != string32_t::npos) {
      out.push_back(s.substr(oldPos, newPos - oldPos));
      oldPos = newPos + delim.size();
    }
    out.push_back(s.substr(oldPos, s.size() - oldPos));
  }

  return out;
}

Alphabet<> get_corpus_alphabet(const corpus_t &corpus) {
  std::unordered_set<char32_t> letters;
  for (const string32_t s : corpus) {
    letters.insert(s.cbegin(), s.cend());
  }
  return Alphabet<>(letters);
}

int main(int argc, char *argv[]) {
  corpus_t trainCorpus = load_corpus("data/train.txt");
  corpus_t valCorpus = load_corpus("data/validate.txt");
  Alphabet<> alphabet = get_corpus_alphabet(trainCorpus);
  for (size_t i = 0; i < alphabet.size(); i++) {
    char32_t c = alphabet.deserialize(i);
    std::cout << i << ' ' << c << ' ';
    utf::write_utf8(c, std::cout);
    std::cout << std::endl;
  }
  // CustomNetModel model(16, alphabet);
  // model.add_combo_node(6, U'e', 7, U' ');
  // model.add_combo_node(6, U't', 7, U' ');
  // model.add_combo_node(6, U'e', 7, U'a');
  // model.add_combo_node(6, U' ', 7, U't');
  // model.add_combo_node(6, U's', 7, U't');
  // model.add_combo_node(6, U's', 7, U' ');
  // model.add_combo_node(6, U's', 7, U'h');
  // model.add_combo_node(6, U'k', 7, U' ');
  // model.add_combo_node(6, U'x', 7, U' ');
  // model.add_combo_node(6, U'x', 7, U'k');
  NGramModel model(5, get_corpus_alphabet(trainCorpus));
  train(model, trainCorpus, valCorpus);

  {
    string32_t s = generate_random(model, 30000);
    ofstream8_t ofs;
    ofs.open("out.txt");
    // model.desc_input(ofs);
    // model.desc_combo(ofs, 1);
    for (char32_t c : s) {
      utf::write_utf8(c, ofs);
    }
    ofs.close();
  }

  corpus_t testCorpus = load_corpus("data/test.txt");
  std::cout << perplexity(model, testCorpus) << std::endl;
}