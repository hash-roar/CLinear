#pragma once
#include "Constant.h"
#include "Memory.h"
#include "Micros.h"
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <type_traits>
#include <vector>
namespace clinear {
namespace base {
// naive implementation

// TODO: align memory
template <typename Dtype, int Size> struct data_array {
  Dtype data[Size];
  constexpr data_array() {}
};

template <typename Dtype> struct data_array<Dtype, 0> {
  Dtype data[1];
  constexpr data_array() {}
};

struct data_array_helper {
  template <typename T, int Size>
  static void copy(const data_array<T, Size> &src, int size,
                   data_array<T, Size> &dst) {
    smart_copy(src.data, src.data + size, dst.data);
  }

  template <typename T, int Size>
  static void swap(data_array<T, Size> &a, int64_t a_size,
                   data_array<T, Size> &b, int64_t b_size) {
    if (a_size < b_size) {
      std::swap_ranges(b.data, b.data + a_size, a.data);
      smart_move(b.data + a_size, b.data + b_size, a.data + a_size);
    } else if (a_size > b_size) {
      std::swap_ranges(a.data, a.data + b_size, b.data);
      smart_move(a.data + b_size, a.data + a_size, b.data + b_size);
    } else {
      std::swap_ranges(a.data, a.data + a_size, b.data);
    }
  }
};
} // namespace base

template <typename Dtype, int Size, int Rows_, int Cols_> class BaseStore {
public:
  BaseStore() = default;
  ~BaseStore() = default;
  constexpr BaseStore(const BaseStore &) = default;
  constexpr BaseStore &operator=(const BaseStore &) = default;
  constexpr BaseStore(BaseStore &&) = default;
  constexpr BaseStore &operator=(BaseStore &&) = default;
  constexpr void swap(BaseStore &other) { std::swap(data_, other.data_); }

  static constexpr int rows() { return Rows_; }
  static constexpr int cols() { return Cols_; }
  constexpr void resize(int, int, int);
  constexpr const Dtype *data() const { return data_.data; }
  constexpr Dtype *data() { return data_.data; }

private:
  base::data_array<Dtype, Size> data_;
};

template <typename Dtype, int Size, int Rows_, int Cols_> class BaseStore;
template <typename Dtype, int Rows_, int Cols_>
class BaseStore<Dtype, 0, Rows_, Cols_> {
public:
  BaseStore() = default;
  ~BaseStore() = default;
  constexpr BaseStore(const BaseStore &) = default;
  constexpr BaseStore &operator=(const BaseStore &) = default;
  constexpr BaseStore(BaseStore &&) = default;
  constexpr BaseStore &operator=(BaseStore &&) = default;
  constexpr void swap(BaseStore &other) {}

  static constexpr int rows() { return Rows_; }
  static constexpr int cols() { return Cols_; }
  constexpr void resize(int, int, int){};
  constexpr const Dtype *data() const { return 0; }
  constexpr Dtype *data() { return 0; }
};

//
template <typename Dtype> class BaseStore<Dtype, 0, Dynamic, Dynamic> {
private:
  int rows_;
  int cols_;

public:
  BaseStore() : rows_(0), cols_(0){};
  ~BaseStore() = default;
  constexpr BaseStore(const BaseStore &other)
      : rows_(other.rows_), cols_(other.cols_){};
  constexpr BaseStore &operator=(const BaseStore &other) {
    rows_ = other.rows_;
    cols_ = other.cols_;
    return *this;
  }

  constexpr BaseStore(BaseStore &&) = default;
  constexpr BaseStore &operator=(BaseStore &&) = default;
  constexpr void swap(BaseStore &other) {
    std::swap(rows_, other.rows_);
    std::swap(cols_, other.cols_);
  }

  constexpr int rows() { return rows_; }
  constexpr int cols() { return cols_; }
  const Dtype *data() const { return nullptr; }
  Dtype *data() { return nullptr; }
  void resize(int, int rows, int cols) {
    rows_ = rows;
    cols_ = cols;
  }
};

// dynamic size with fixed-size storage
template <typename Dtype, int Size>
class BaseStore<Dtype, Size, Dynamic, Dynamic> {
private:
  int rows_;
  int cols_;
  base::data_array<Dtype, Size> data_;

public:
  constexpr BaseStore() : data_(), rows_(0), cols_(0){};
  ~BaseStore() = default;
  constexpr BaseStore(const BaseStore &other)
      : rows_(other.rows_), cols_(other.cols_) {
    base::data_array_helper::copy(other.data_, rows_ * cols_, data_);
  };

  constexpr BaseStore &operator=(const BaseStore &other) {
    if (this == &other) {
      return *this;
    }
    rows_ = other.rows_;
    cols_ = other.cols_;
    base::data_array_helper::copy(other.data_, rows_ * cols_, data_);
    return *this;
  }
  void swap(BaseStore &other) {
    std::swap(rows_, other.rows_);
    std::swap(cols_, other.cols_);
    base::data_array_helper::swap(data_, rows_ * cols_, other.data_,
                                  other.rows_ * other.cols_);
  }
  // no move
  constexpr int rows() { return rows_; }
  constexpr int cols() { return cols_; }
  constexpr void resize(int, int rows, int cols) {
    rows_ = rows;
    cols_ = cols;
  }
  constexpr const Dtype *data() const { return data_.data; }
  constexpr Dtype *data() { return data_.data; }
};

// storage with dynamic size
template <typename Dtype> class BaseStore<Dtype, Dynamic, Dynamic, Dynamic> {
  Dtype *data_;
  int rows_;
  int cols_;

public:
  constexpr BaseStore() : data_(nullptr), rows_(0), cols_(0){};
  BaseStore(int size, int row, int col) {
    data_ = new Dtype[size];
    rows_ = row;
    cols_ = col;
  }
  // copy constructor
  BaseStore(const BaseStore &other)
      : data_(new Dtype[rows_ * cols_]), rows_(other.rows_),
        cols_(other.cols_) {
    std::copy(other.data_, other.data_ + rows_ * cols_, data_);
  }
  // operator =
  BaseStore &operator=(const BaseStore &other) {
    if (this == &other) {
      return *this;
    }
    BaseStore tmp(other);
    swap(tmp);
    return *this;
  }
  // move constructor
  BaseStore(BaseStore &&other)
      : data_(other.data_), rows_(other.rows_), cols_(other.cols_) {
    other.data_ = nullptr;
    other.rows_ = 0;
    other.cols_ = 0;
  }
  // move operator =
  BaseStore &operator=(BaseStore &&other) {
    std::swap(data_, other.data_);
    std::swap(rows_, other.rows_);
    std::swap(cols_, other.cols_);
    return *this;
  }
  // destructor
  ~BaseStore() {
    if (data_ != nullptr) {
      delete[] data_;
    }
  }

  void swap(BaseStore &other) {
    std::swap(data_, other.data_);
    std::swap(rows_, other.rows_);
    std::swap(cols_, other.cols_);
  }

  constexpr int rows() { return rows_; }
  constexpr int cols() { return cols_; }
  constexpr void resize(int size, int rows, int cols) {
    if (size != rows_ * cols_) {
      if (data_ != nullptr) {
        delete[] data_;
      }
      if (size > 0) {
        data_ = new Dtype[size];
      } else {
        data_ = nullptr;
      }
    }
  }
  constexpr const Dtype *data() const { return data_; }
  constexpr Dtype *data() { return data_; }
};

} // namespace clinear