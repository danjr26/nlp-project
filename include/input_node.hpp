#ifndef INPUT_NODE_HPP
#define INPUT_NODE_HPP

#include "node.hpp"

class InputNode : public Node {
public:
  explicit InputNode(size_t nWords);

  void set_word(uint32_t word);

  std::vector<double> probs() const;
  std::vector<double> logprobs() const;

public:
  void observe() override;
  void forward() override;
  void backward(const std::unordered_set<Node *> &unknown) override;
  std::vector<double> entropy() const override;
  std::vector<double> potential() const override;

private:
  uint32_t word;
  std::vector<size_t> xs;
  size_t n;
};

#endif