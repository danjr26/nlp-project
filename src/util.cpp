#include "util.hpp"

#include <cmath>

double log_add_exp(double v1, double v2) {
  return std::log2(std::exp2(v1) + std::exp2(v2));
}