#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
namespace clinear::util {
template <typename T> class ThreadSafeQueue {
public:
  bool empty() const {
    std::unique_lock lock{mutex_};
    return que_.empty();
  }

  bool pop(T &v) {
    std::unique_lock lock{mutex_};
    if (que_.empty())
      return false;
    v = que_.front();
    que_.pop();
    return true;
  }

  bool push(T const &v) {
    std::unique_lock lock{mutex_};
    que_.push(v);
    return true;
  }

private:
  std::queue<T> que_;
  std::mutex mutex_;
};

class ThreadPool {
  template <typename T> using Queue = ThreadSafeQueue<T>;

public:
  ThreadPool() = default;
  ThreadPool(int thread_num) { resize(thread_num); };
  ThreadPool(ThreadPool &&) = delete;
  ThreadPool(const ThreadPool &) = delete;
  ThreadPool operator=(ThreadPool &&) = delete;
  ThreadPool operator=(const ThreadPool &) = delete;
  ~ThreadPool() { stop(true); };

  int size() const { return static_cast<int>(threads_.size()); }
  int waiting_num() const { return waiting_num_; }

  std::thread &get_thread(int index) { return *threads_[index]; }

  void resize(int thread_num) {
    if (is_done_ || is_stop_)
      return;
    int old_thread_num = static_cast<int>(threads_.size());

    if (old_thread_num <= thread_num) {
      threads_.resize(thread_num);
      flags_.resize(thread_num);
      for (int i = old_thread_num; i < thread_num; i++) {
        flags_[i] = std::make_shared<std::atomic<bool>>(false);
        init_thread(i);
      }
    } else {
      for (int i = old_thread_num - 1; i >= thread_num; i--) {
        *flags_[i] = true;
        threads_[i]->detach();
      }
      {
        std::unique_lock<std::mutex> lock{mutex_};
        cv_.notify_all();
      }
      threads_.resize(thread_num);
      flags_.resize(thread_num);
    }
  }

  void clear_queue() {
    std::function<void(int)> *func;
    while (que_.pop(func)) {
      delete func;
    }
  }

  std::function<void(int)> pop() {
    std::function<void(int)> *func = nullptr;
    que_.pop(func);
    std::unique_ptr<std::function<void(int)>> func_{func};
    if (func)
      return *func;
    return nullptr;
  }

  void stop(bool wait = false) {
    if (!wait) {
      if (is_stop_)
        return;
      is_stop_ = true;
      for (int i = 0; i < size(); i++) {
        *flags_[i] = true;
      }
      clear_queue();
    } else {
      if (is_done_ || is_stop_)
        return;
      is_done_ = true;
    }
    {
      std::unique_lock<std::mutex> lock{mutex_};
      cv_.notify_all();
    }
    for (int i = 0; i < size(); i++) {
      if (threads_[i]->joinable()) {
        threads_[i]->join();
      }
    }

    clear_queue();
    threads_.clear();
    flags_.clear();
  }

  template <typename Func> auto push(Func &&f) -> std::future<decltype(f(0))> {
    using return_type = decltype(f(0));
    auto task = std::make_shared<std::packaged_task<return_type(int)>>(
        std::forward<Func>(f));
    std::function<void(int)> *func =
        new std::function<void(int)>([task](int index) { (*task)(index); });
    que_.push(func);
    {
      std::unique_lock<std::mutex> lock{mutex_};
      cv_.notify_one();
    }
    return task->get_future();
  }

  template <typename Func, typename... Args>
  auto push(Func &&f, Args &&...args) -> std::future<decltype(f(0, args...))> {
    using return_type = decltype(f(0, args...));
    auto task = std::make_shared<std::packaged_task<return_type(int)>>(
        std::bind(std::forward<Func>(f), std::placeholders::_1,
                  std::forward<Args>(args)...));
    std::function<void(int)> *func =
        new std::function<void(int)>([task](int index) { (*task)(index); });
    que_.push(func);
    {
      std::unique_lock<std::mutex> lock{mutex_};
      cv_.notify_one();
    }
    return task->get_future();
  }

private:
  void init_thread(int index) {
    auto flag_copy{flags_[index]};

    auto thread_func = [this, index, flag_copy]() {
      auto &_flag = *flag_copy;
      std::function<void(int)> *_func;
      bool has_func = que_.pop(_func);
      while (true) {
        while (has_func) {
          auto func = std::make_unique<decltype(_func)>(_func);
          (*_func)(index);
          if (_flag) {
            return;
          } else {
            has_func = que_.pop(_func);
          }
        }

        // wait task
        {
          std::unique_lock<std::mutex> lock{mutex_};
          waiting_num_++;
          cv_.wait(lock, [this, &has_func, &_func, &flag_copy]() {
            has_func = que_.pop(_func);
            return has_func || is_done_ || *flag_copy;
          });
          waiting_num_--;
          if (!has_func) {
            return;
          }
        }
      }
    };
    threads_[index].reset(new std::thread(thread_func));
  }
  std::vector<std::unique_ptr<std::thread>> threads_;
  std::vector<std::shared_ptr<std::atomic<bool>>> flags_;
  Queue<std::function<void(int)> *> que_;
  std::atomic<bool> is_done_{false};
  std::atomic<bool> is_stop_{false};
  std::atomic<int> waiting_num_{0};

  std::mutex mutex_;
  std::condition_variable cv_;
};

} // namespace clinear::util