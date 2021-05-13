#include "input_node.hpp"
#include "util.hpp"

#include <cassert>
#include <limits>

InputNode::InputNode(size_t nWords) : Node(0, 0, nWords), xs(nWords, 1), n(2) {}

void InputNode::set_word(uint32_t word) { this->word = word; }

std::vector<double> InputNode::probs() const {
  std::vector<double> ps = logprobs();
  double total = 0.0;
  for (double &p : ps) {
    p = std::exp2(p);
    total += p;
  }
  for (double &p : ps) {
    p /= total;
  }
  return ps;
}

std::vector<double> InputNode::logprobs() const { return backwardLogPs; }

void InputNode::observe() {
  xs[word]++;
  n++;
}

void InputNode::forward() {
  forwardBits.assign(n_outputs(), false);
  forwardBits.at(word) = true;
}

void InputNode::backward(const std::unordered_set<Node *> &unknown) {
  double logMass = -std::numeric_limits<double>::infinity();
  for (size_t i = 0; i < n_outputs(); i++) {
    if (backwardCounts[i]) {
      backwardLogPs[i] += std::log2(xs[i]) - std::log2(n);
    } else {
      backwardLogPs[i] = std::log2(xs[i]) - std::log2(n);
    }
    logMass = log_add_exp(logMass, backwardLogPs[i]);
  }
  for (size_t i = 0; i < n_outputs(); i++) {
    backwardLogPs[i] -= logMass;
  }
}

std::vector<double> InputNode::entropy() const {
  std::vector<double> out(n_outputs());
  for (size_t i = 0; i < n_outputs(); i++) {
    double p = (double)xs[i] / (double)n;
    out[i] = -p * std::log2(p) - (1.0 - p) * std::log2(1.0 - p);
  }
  return out;
}

std::vector<double> InputNode::potential() const { return entropy(); }