#include "node.hpp"
#include "util.hpp"

#include <limits>

Node::Node(size_t level, size_t nInputs, size_t nOutputs)
    : level(level), forwardBits(nInputs, false), backwardLogPs(nOutputs, 0.0),
      backwardCounts(nOutputs, 0) {}

Node::~Node() {}

size_t Node::get_level() const { return level; }

void Node::set_level(size_t level) { this->level = level; }

bool Node::get_forward_bit(size_t index) const { return forwardBits[index]; }

void Node::contribute_backward_loglh(size_t index, double loglh) {
  backwardLogPs[index] += loglh;
  backwardCounts[index]++;
}

double Node::get_backward_logprob(size_t index) const {
  return backwardLogPs[index] - std::log2(backwardCounts[index]);
}

void Node::clear_backward() {
  backwardLogPs.assign(n_outputs(), 0.0);
  backwardCounts.assign(n_outputs(), 0);
}

size_t Node::n_inputs() const { return forwardBits.size(); }

size_t Node::n_outputs() const { return backwardLogPs.size(); }
