#include "BaseStore.h"
#include <gtest/gtest.h>
#include <iostream>

//  add google test
using namespace clinear;
TEST(TestBaseStorage, TestBaseStorageNullStorage) {
  BaseStore<float, 0, 10, 20> base_store;
  ASSERT_EQ(base_store.cols(), 20);
  ASSERT_EQ(base_store.rows(), 10);
  ASSERT_EQ(base_store.data(), nullptr);
}

TEST(TestBaseStorage, TestBaseStorageStaticStorage) {
  BaseStore<float, 100, 10, 20> base_store;
  ASSERT_EQ(base_store.cols(), 20);
  ASSERT_EQ(base_store.rows(), 10);
  base_store.data()[0] = 1.0;
  ASSERT_EQ(base_store.data()[0], 1.0);
  base_store.data()[99] = 2.0;
  ASSERT_EQ(base_store.data()[99], 2.0);
  BaseStore<float, 100, 10, 20> other_base_store;
  other_base_store.data()[99] = 3.0;
  base_store.swap(other_base_store);
  ASSERT_EQ(base_store.data()[99], 3.0);
  ASSERT_EQ(other_base_store.data()[99], 2.0);
  auto new_store = std::move(base_store);
  ASSERT_EQ(new_store.data()[99], 3.0);
  ASSERT_EQ(base_store.data()[99], 3.0);
}

TEST(TestBaseStorage, TestBaseStorageDynamicStorage) {
  BaseStore<double, Dynamic, Dynamic, Dynamic> base_store;
  ASSERT_EQ(base_store.cols(), 0);
  ASSERT_EQ(base_store.data(), nullptr);
  base_store.resize(20 * 30 + 1, 20, 30);
  ASSERT_EQ(base_store.cols(), 30);
  base_store.data()[10 * 30 - 1] = 1.0;
  ASSERT_EQ(base_store.data()[10 * 30 - 1], 1.0);
}

int main(int argc, char **argv) {
  // 初始化gtest框架
  testing::InitGoogleTest(&argc, argv);
  // 运行所有测试用例
  return RUN_ALL_TESTS();
}