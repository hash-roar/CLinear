#include <algorithm>
#include <cstdint>
#include <cstring>
#include <type_traits>
namespace clinear::base {
// template <typename T, bool IsPod> struct smart_copy_helper;
// // copy pod by memcpy
// template <typename T> struct smart_copy_helper<T, true> {
//   static inline void run(const T *start, const T *end, T *dst) {
//     std::intptr_t size = std::intptr_t(end) - std::intptr_t(start);
//     if (size == 0)
//       return;

//     std::memcpy(dst, start, size);
//   }
// };

// // copy non-pod by copy constructor
// template <typename T> struct smart_copy_helper<T, false> {
//   static inline void run(const T *start, const T *end, T *dst) {
//     std::copy(start, end, dst);
//   }
// };

// template <typename T> void smart_copy(const T *start, const T *end, T *dst) {
//   smart_copy_helper<T, std::is_pod<T>::value>::run(start, end, dst);
// }

template <typename T> void smart_copy(const T *start, const T *end, T *target) {
  if (std::is_pod<T>::value) {
    std::intptr_t size = std::intptr_t(end) - std::intptr_t(start);
    if (size == 0)
      return;

    std::memcpy(target, start, size);
  } else {
    std::copy(start, end, target);
  }
}

template <typename T, bool IsPod> struct smart_memmove_helper;
// move pod by memmove
template <typename T> struct smart_memmove_helper<T, true> {
  static inline void run(T *start, T *end, T *dst) {
    std::intptr_t size = std::intptr_t(end) - std::intptr_t(start);
    if (size == 0)
      return;

    std::memmove(dst, start, size);
  }
};
// move non-pod by move constructor
template <typename T> struct smart_memmove_helper<T, false> {
  static inline void run(T *start, T *end, T *dst) {
    if (std::uintptr_t(dst) < std::uintptr_t(start)) {
      std::copy(start, end, dst);
    } else {
      std::ptrdiff_t count =
          (std::ptrdiff_t(end) - std::ptrdiff_t(start)) / sizeof(T);
      std::copy_backward(start, end, dst + count);
    }
  }
};

template <typename T> void smart_memmove(T *start, T *end, T *dst) {
  smart_memmove_helper<T, std::is_pod<T>::value>::run(start, end, dst);
}

template <typename T> T *smart_move(T *start, T *end, T *target) {
  return std::move(start, end, target);
}

} // namespace clinear::base
