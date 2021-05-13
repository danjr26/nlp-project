#include "custom_net.hpp"

#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <numeric>

bool CustomNetModel::ComboSlot::operator<(const ComboSlot &other) const {
  if (&this->node1 < &other.node1)
    return true;
  if (&this->node1 > &other.node1)
    return false;
  if (&this->node2 < &other.node2)
    return true;
  if (&this->node2 > &other.node2)
    return false;
  if (this->index1 < other.index1)
    return true;
  if (this->index1 > other.index1)
    return false;
  if (this->index2 < other.index2)
    return true;
  return false;
}

CustomNetModel::CustomNetModel(size_t windowLen,
                               const Alphabet<char32_t, uint32_t> &alphabet)
    : alphabet(alphabet), inputs(windowLen, InputNode(alphabet.size())),
      serialInputs(), combos(), nObserved(0) {
  for (auto it = inputs.begin(); it != inputs.end(); it++) {
    serialInputs.push_back(it);
  }
}

typename CustomNetModel::State CustomNetModel::start() {
  return State(inputs.size(), alphabet.serialize(utf::BEG_STRING));
}

InputNode &CustomNetModel::get_input_node(size_t index) {
  assert(index < inputs.size());
  return *serialInputs.at(index);
}

void CustomNetModel::add_combo_node(Node &node1, size_t index1, Node &node2,
                                    size_t index2) {
  assert(combo_possible(node1, index1, node2, index2));
  ComboNode combo(node1, index1, node2, index2);
  size_t level = combo.get_level();
  auto &comboLevel = combos
                         .insert(std::make_pair(
                             level, std::shared_ptr<std::list<ComboNode>>()))
                         .first->second;
  if (!comboLevel) {
    comboLevel = std::make_shared<std::list<ComboNode>>();
  }
  comboLevel->push_back(combo);
  comboSlots.insert(ComboSlot{node1, node2, index1, index2});
}

void CustomNetModel::add_combo_node(size_t index1, char32_t c1, size_t index2,
                                    char32_t c2) {
  add_combo_node(get_input_node(index1), alphabet.serialize(c1),
                 get_input_node(index2), alphabet.serialize(c2));
}

bool CustomNetModel::combo_possible(Node &node1, size_t index1, Node &node2,
                                    size_t index2) {
  return &node1 != &node2 &&
         !comboSlots.count(ComboSlot{node1, node2, index1, index2});
}

bool CustomNetModel::combo_possible(size_t index1, char32_t c1, size_t index2,
                                    char32_t c2) {
  return combo_possible(get_input_node(index1), alphabet.serialize(c1),
                        get_input_node(index2), alphabet.serialize(c2));
}

void CustomNetModel::observe(State state, char32_t c) {
  state.push_back(alphabet.serialize(c));
  for (size_t i = 0; i < inputs.size(); i++) {
    InputNode &input = *serialInputs.at(i);
    input.set_word(state[i]);
    input.observe();
    input.forward();
  }

  for (auto &level : combos) {
    for (ComboNode &combo : *level.second) {
      combo.observe();
      combo.forward();
    }
  }

  if (++nObserved % 1000 == 0) {
    size_t count = 0;
    auto openNodes = open_nodes();
    for (OpenNode const &openNode1 : openNodes) {
      for (OpenNode const &openNode2 : openNodes) {
        if (combo_possible(openNode1.node, openNode1.index, openNode2.node,
                           openNode2.index)) {
          add_combo_node(openNode1.node, openNode1.index, openNode2.node,
                         openNode2.index);
          if (count++ >= 1)
            break;
        }
      }
      if (count >= 1)
        break;
    }
  }
}

typename CustomNetModel::State CustomNetModel::step(State state, char32_t c) {
  state.push_back(alphabet.serialize(c));
  state.erase(state.begin());
  return state;
}

std::unordered_map<char32_t, double> CustomNetModel::probs(State state) {
  InputNode &unknownInput = *serialInputs.back();
  std::unordered_set<Node *> known;
  std::unordered_set<Node *> unknown({&unknownInput});

  // forward pass
  for (size_t i = 0; i < inputs.size() - 1; i++) {
    InputNode &input = *serialInputs[i];
    uint32_t c = state[i + 1];
    input.set_word(c);
    input.forward();
    known.insert(&input);
  }
  for (auto &level : combos) {
    for (ComboNode &combo : *level.second) {
      if (unknown.count(&combo.get_node1()) ||
          unknown.count(&combo.get_node2())) {
        unknown.insert(&combo);
        combo.clear_backward();
      } else {
        known.insert(&combo);
        combo.forward();
      }
    }
  }

  // backward pass
  unknownInput.clear_backward();
  for (auto levelIt = combos.rbegin(); levelIt != combos.rend(); levelIt++) {
    for (ComboNode &combo : *levelIt->second) {
      if (unknown.count(&combo)) {
        combo.backward(unknown);
      }
    }
  }
  unknownInput.backward(unknown);

  // deserialize
  std::vector<double> serialPs = unknownInput.probs();
  std::unordered_map<char32_t, double> deserialPs;
  for (uint32_t i = 0; i < serialPs.size(); i++) {
    deserialPs.insert_or_assign(alphabet.deserialize(i), serialPs[i]);
  }
  return deserialPs;
}

std::multiset<typename CustomNetModel::OpenNode,
              std::function<bool(const typename CustomNetModel::OpenNode &,
                                 const typename CustomNetModel::OpenNode &)>>
CustomNetModel::open_nodes() {
  std::function<bool(const OpenNode &, const OpenNode &)> cmp =
      [](const OpenNode &node1, const OpenNode &node2) {
        return node1.potential > node2.potential;
      };
  std::multiset<OpenNode,
                std::function<bool(const OpenNode &, const OpenNode &)>>
      out(cmp);
  for (InputNode &input : inputs) {
    auto potential = input.potential();
    for (size_t i = 0; i < potential.size(); i++) {
      out.insert(OpenNode{input, i, potential[i]});
    }
  }
  for (auto level : combos) {
    for (ComboNode &combo : *level.second) {
      out.insert(OpenNode{combo, 0, combo.potential()[0]});
    }
  }
  return out;
}

ostream8_t &CustomNetModel::desc_input(ostream8_t &os) {
  os << "layer,node,char,entropy\n";
  for (auto it : serialInputs) {
    std::vector<double> entropy = it->entropy();
    for (size_t i = 0; i < it->n_outputs(); i++) {
      os << "input," << std::distance(inputs.begin(), it) << ",";
      utf::write_utf8(alphabet.deserialize(i), os);
      os << "," << entropy[i] << "\n";
    }
  }
  return os;
}

ostream8_t &CustomNetModel::desc_combo(ostream8_t &os, size_t level) {
  os << "layer,node,mutual_info\n";
  for (ComboNode &combo : *combos.at(level)) {
    os << "combo" << level << ",";
    os << combo.mutual_info() << "\n";
    // for (bool v1 : {false, true}) {
    //   for (bool v2 : {false, true}) {
    //     size_t i = combo.xs_index(v1, v2);
    //     os << (v1 ? 't' : 'f') << (v2 ? 't' : 'f') << ',';
    //   }
    // }
  }
  return os;
}