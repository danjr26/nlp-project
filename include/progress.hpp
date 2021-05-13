#ifndef PROGRESS_HPP
#define PROGRESS_HPP

class Progress {
public:
  Progress(size_t total);
  ~Progress();

  void set(size_t count);
  void add(size_t count);
  void update();

private:
  size_t count;
  size_t total;
  size_t width;
};

#endif