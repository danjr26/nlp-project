#ifndef CUSTOM_NET_HPP
#define CUSTOM_NET_HPP

#include "alphabet.hpp"
#include "combo_node.hpp"
#include "input_node.hpp"

#include <functional>
#include <list>
#include <map>
#include <memory>
#include <set>

class CustomNetModel {
public:
  using State = std::vector<uint32_t>;
  struct OpenNode {
    Node &node;
    size_t index;
    double potential;
  };

private:
  struct ComboSlot {
    Node &node1;
    Node &node2;
    size_t index1;
    size_t index2;

    bool operator<(const ComboSlot &other) const;
  };

public:
  CustomNetModel(size_t windowLen,
                 const Alphabet<char32_t, uint32_t> &alphabet);

  InputNode &get_input_node(size_t index);
  void add_combo_node(Node &node1, size_t index1, Node &node2, size_t index2);
  void add_combo_node(size_t index1, char32_t c1, size_t index2, char32_t c2);
  bool combo_possible(Node &node1, size_t index1, Node &node2, size_t index2);
  bool combo_possible(size_t index1, char32_t c1, size_t index2, char32_t c2);

  State start();
  void observe(State state, char32_t c);
  State step(State state, char32_t c);
  std::unordered_map<char32_t, double> probs(State state);

  std::multiset<OpenNode,
                std::function<bool(const OpenNode &, const OpenNode &)>>
  open_nodes();

  ostream8_t &desc_input(ostream8_t &os);
  ostream8_t &desc_combo(ostream8_t &os, size_t level);

private:
  Alphabet<char32_t, uint32_t> alphabet;
  std::list<InputNode> inputs;
  std::vector<typename std::list<InputNode>::iterator> serialInputs;
  std::map<size_t, std::shared_ptr<std::list<ComboNode>>> combos;
  std::set<ComboSlot> comboSlots;
  size_t nObserved;
};

template <class Iter>
void weighted_random_choice(Iter begin, Iter end,
                            std::function<double(Iter)> weight) {}
#endif