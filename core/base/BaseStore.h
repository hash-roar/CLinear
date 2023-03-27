#pragma once
#include <vector>
namespace clinear::base {
// naive implementation

template<typename Dtype>
class BaseStore {
public:
  BaseStore();
  ~BaseStore();

private:
  std::vector<std::vector<char>> data_;
};
} // namespace clinear::base