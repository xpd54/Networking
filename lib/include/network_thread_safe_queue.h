#pragma once
#include "network_common.h"
#include <cstddef>
#include <deque>
#include <mutex>
#include <utility>

xpd54_namespace_start

    template <typename T>
    class thread_safe_queue {
public:
  thread_safe_queue() = default;
  thread_safe_queue(const thread_safe_queue<T> &) = delete;
  ~thread_safe_queue() { clear(); }

  const T &front() const {
    std::scoped_lock lock(mutex_queue);
    return deqQueue.front();
  }

  const T &back() const {
    std::scoped_lock lock(mutex_queue);
    return deqQueue.back();
  }

  void push_back(const T &item) {
    std::scoped_lock lock(mutex_queue);
    deqQueue.emplace_back(std::move(item));
  }

  void push_front(const T &item) {
    std::scoped_lock lock(mutex_queue);
    deqQueue.emplace_front(std::move(item));
  }

  size_t count() const {
    std::scoped_lock lock(mutex_queue);
    return deqQueue.size();
  }

  void clear() {
    std::scoped_lock lock(mutex_queue);
    deqQueue.clear();
  }

  T pop_front() {
    std::scoped_lock lock(mutex_queue);
    auto item = std::move(deqQueue.front());
    deqQueue.pop_front();
    return item;
  }

  T pop_back() {
    std::scoped_lock lock(mutex_queue);
    auto item = std::move(deqQueue.back());
    deqQueue.pop_back();
    return item;
  }

protected:
  std::mutex mutex_queue;
  std::deque<T> deqQueue;
};

xpd54_namespace_end