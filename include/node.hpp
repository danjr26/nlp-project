#ifndef NODE_HPP
#define NODE_HPP

#include <unordered_set>
#include <vector>

class Node {
public:
  Node(size_t level, size_t nInputs, size_t nOutputs);
  virtual ~Node();

public:
  size_t get_level() const;
  void set_level(size_t level);
  bool get_forward_bit(size_t index) const;
  void contribute_backward_loglh(size_t index, double loglh);
  double get_backward_logprob(size_t index) const;
  void clear_backward();
  size_t n_inputs() const;
  size_t n_outputs() const;

public:
  virtual void observe() = 0;
  virtual void forward() = 0;
  virtual void backward(const std::unordered_set<Node *> &unknown) = 0;
  virtual std::vector<double> entropy() const = 0;
  virtual std::vector<double> potential() const = 0;

protected:
  std::vector<bool> forwardBits;
  std::vector<double> backwardLogPs;
  std::vector<size_t> backwardCounts;
  size_t level;
};

#endif