#pragma once

#include "base/BaseStore.h"
#include <initializer_list>
namespace clinear {

template <typename Dtype> class Matrix {
public:
  Matrix() = default;
  Matrix(std::initializer_list<Dtype> initializer_list);
  ~Matrix();

private:
  // BaseStore<Dtype> data_;
};
} // namespace clinear