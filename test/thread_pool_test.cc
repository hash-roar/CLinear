#include "ThreadPool.hpp"
#include <atomic>
#include <fmt/core.h>
#include <gtest/gtest.h>
#include <iostream>

using namespace clinear::util;
using namespace std;
TEST(ThreadPoolTest, TestBasic) {
  ThreadPool pool{};

  pool.resize(4);
  ASSERT_EQ(pool.size(), 4);
  //   cout << "waiting_num: " << pool.waiting_num() << endl;
}

int func1(int id, int arg1) {
  cout << "func1 id:" << id << "\n";
  return arg1;
}
TEST(ThreadPoolTest, TestFuncParama) {
  ThreadPool pool{3};
  ASSERT_EQ(pool.size(), 3);
  pool.resize(4);
  ASSERT_EQ(pool.size(), 4);
  //   cout << "waiting_num: " << pool.waiting_num() << endl;
  pool.push([](int id) { cout << id << "\n"; });
  auto fut = pool.push(func1, 3);
  ASSERT_EQ(fut.get(), 3);
  auto fut2 = pool.push(
      [](int id, int arg1) {
        cout << "lambda id:" << id << "\n";
        return arg1;
      },
      3);
  ASSERT_EQ(fut2.get(), 3);
}

TEST(ThreadPoolTest, TestComplicated) {

  std::atomic<int> counter{0};
  {
    ThreadPool pool{3};
    for (int i = 0; i < 10000; i++) {
      pool.push([&counter](int id) { counter++; });
    }
    pool.resize(1);
    for (int i = 0; i < 10000; i++) {
      pool.push([&counter](int id) { counter--; });
    }
  }
  fmt::print("counter: {}\n", counter.load());
  ASSERT_EQ(counter, 0);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}