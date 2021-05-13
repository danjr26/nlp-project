#ifndef COMBO_NODE_HPP
#define COMBO_NODE_HPP

#include "node.hpp"

class ComboNode : public Node {
public:
  ComboNode(Node &node1, size_t index1, Node &node2, size_t index2);

  double mutual_info() const;
  Node &get_node1();
  Node &get_node2();

public:
  void observe() override;
  void forward() override;
  void backward(const std::unordered_set<Node *> &unknown) override;
  std::vector<double> entropy() const override;
  std::vector<double> potential() const override;
  size_t xs_index(bool bit1, bool bit2) const;

private:
  Node &node1;
  Node &node2;
  size_t index1;
  size_t index2;
  size_t xs[4];
  size_t n;
};

#endif