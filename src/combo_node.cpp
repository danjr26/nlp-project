#include "combo_node.hpp"
#include "util.hpp"

#include <algorithm>
#include <cassert>

std::vector<double> &normalize_logprobs(std::vector<double> &logps) {
  std::vector<double> ps(logps.size());
  double winner = *std::max_element(logps.begin(), logps.end());
  double total = 0.0;
  for (size_t i = 0; i < logps.size(); i++) {
    ps[i] = std::exp2(logps[i] - winner);
    total += ps[i];
  }
  for (size_t i = 0; i < logps.size(); i++) {
    logps[i] = std::log2(ps[i]) - std::log2(total);
  }
  return logps;
}

ComboNode::ComboNode(Node &node1, size_t index1, Node &node2, size_t index2)
    : Node(std::max(node1.get_level(), node2.get_level()) + 1, 2, 4),
      node1(node1), node2(node2), index1(index1),
      index2(index2), xs{1, 1, 1, 1}, n(4) {}

double ComboNode::mutual_info() const {
  double info = 0.0;
  const double logN = std::log2(n);
  for (bool bit1 : {true, false}) {
    const double logp1 =
        std::log2(xs[xs_index(bit1, true)] + xs[xs_index(bit1, false)]) - logN;
    for (bool bit2 : {true, false}) {
      const double logp2 =
          std::log2(xs[xs_index(true, bit2)] + xs[xs_index(false, bit2)]) -
          logN;
      const size_t x = xs[xs_index(bit1, bit2)];
      const double probJoint = (double)x / (double)n;
      const double logpJoint = std::log2(x) - std::log2(n);
      info += probJoint * (logpJoint - (logp1 + logp2));
    }
  }
  return info;
}

Node &ComboNode::get_node1() { return node1; }

Node &ComboNode::get_node2() { return node2; }

void ComboNode::observe() {
  const bool bit1 = node1.get_forward_bit(index1);
  const bool bit2 = node2.get_forward_bit(index2);
  const size_t index = xs_index(bit1, bit2);
  xs[index]++;
  n++;
}

void ComboNode::forward() {
  const bool bit1 = node1.get_forward_bit(index1);
  const bool bit2 = node2.get_forward_bit(index2);
  forwardBits[0] = bit1 && bit2;
}

void ComboNode::backward(const std::unordered_set<Node *> &unknown) {
  // if (mutual_info() < 0.001 || n < 50000) {
  //   return;
  // }

  const bool known1 = !unknown.count(&node1);
  const bool known2 = !unknown.count(&node2);
  if (known1 == known2) {
    return;
  }

  const bool bit1 = node1.get_forward_bit(index1);
  const bool bit2 = node2.get_forward_bit(index2);
  if (!bit1 && !bit2) {
    return;
  }
  double logMass = -std::numeric_limits<double>::infinity();
  for (size_t i = 0; i < 4; i++) {
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

  const double trueLogP = known1 ? backwardLogPs[xs_index(bit1, true)]
                                 : backwardLogPs[xs_index(true, bit2)];
  const double falseLogP = known1 ? backwardLogPs[xs_index(bit1, false)]
                                  : backwardLogPs[xs_index(false, bit2)];
  const double logp = trueLogP - log_add_exp(trueLogP, falseLogP);
  const double logBaseRate =
      known1 ? log_add_exp(backwardLogPs[xs_index(bit1, true)],
                           backwardLogPs[xs_index(bit1, false)])
             : log_add_exp(backwardLogPs[xs_index(true, bit2)],
                           backwardLogPs[xs_index(false, bit2)]);
  const double loglh = logp - logBaseRate;
  Node &node = known1 ? node2 : node1;
  node.contribute_backward_loglh(index2, loglh);
}

std::vector<double> ComboNode::entropy() const {
  std::vector<double> out(n_outputs());
  for (size_t i = 0; i < n_outputs(); i++) {
    const double p = (double)xs[i] / (double)n;
    out[i] = -p * std::log2(p) + -(1.0 - p) * std::log2(1.0 - p);
  }
  return out;
}

std::vector<double> ComboNode::potential() const {
  return std::vector<double>(n_outputs(), mutual_info());
}

size_t ComboNode::xs_index(bool bit1, bool bit2) const {
  return ((size_t)bit1 << 0) + ((size_t)bit2 << 1);
}