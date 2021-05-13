#include "progress.hpp"

#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>

Progress::Progress(size_t total) : count(0), total(total), width(80) {
  update();
}

Progress::~Progress() {
  // std::cerr << '\r' << std::string(width, ' ') << '\r' << std::flush;
  std::cerr << std::endl;
}

void Progress::set(size_t count) {
  this->count = count;
  update();
}

void Progress::add(size_t count) {
  this->count += count;
  update();
}

void Progress::update() {
  size_t precision = 2;
  float progress = (float)std::min(count, total) / (float)total;
  float percent = progress * 100.0f;
  size_t barMaxWidth = width - 8 - precision;
  size_t barWidth = (size_t)(barMaxWidth * progress);
  std::string barString = std::string(barWidth, '#');
  float partialBarWidth = (barMaxWidth * progress - barWidth);
  std::string charProgression(" -=o");
  if (partialBarWidth) {
    size_t index = (size_t)(partialBarWidth * charProgression.size());
    barString.append(1, charProgression[index]);
  }
  barString += std::string(barMaxWidth - barString.size(), ' ');
  std::stringstream ss;
  ss << "[" << barString << "] " << std::fixed << std::setprecision(precision)
     << std::setw(4 + precision) << std::right << percent << '%';
  std::cerr << '\r' << ss.str() << std::flush;
}
