#ifndef _COROUTINE_H
#define _COROUTINE_H

#include <iostream>
#include <functional>
#include <vector>
#include <memory>

#include <ucontext.h>

const static int STACK_SIZE = 1024 * 1024 * 2;
const static int DEFAULT_COROUTINES = 16;

enum CoroutineStatus {
  CO_READY,
  CO_RUNNING,
  CO_SUSPEND,
  CO_DEAD,
};

class Coroutine {

public:
  void operator()() { run_(); }
  CoroutineStatus getStatus() const { return status_; }

private:
  template <typename FUNC, typename... ARGS>
  Coroutine(FUNC &&func, ARGS &&... args) : status_(CO_READY), stackSize_(0) {
    run_ = [&]() { func(std::forward<ARGS...>(args...)); };
  }

  std::function<void()> run_;
  std::vector<char> stack_;
  size_t stackSize_;
  CoroutineStatus status_;

  friend class Scheduler;
};

class Scheduler {
public:
  Scheduler() { stack_.resize(STACK_SIZE); }

  size_t running() const {
    return running_;
  }

  template <typename FUNC, typename... ARGS>
  void coroutine(FUNC&& func, ARGS&& ...args) {
    if (nco_ == coroutines_.size()) {
      coroutines_.push_back(std::make_shared<Coroutine>(
                              std::forward<FUNC>(func), std::forward<ARGS...>(args...)));
    } else {
      for(auto & it : coroutines_) {
        if (!it) {
          it = std::make_shared<Coroutine>(std::foward<FUNC>(func), std::forward<ARGS...>(args...));
        }
      }
    }
    ++nco_;
  }

private:
  size_t nco_;
  size_t running_;
  std::vector<std::shared_ptr<Coroutine>> coroutines_;
  std::vector<char> stack_;
  ucontext_t main_;
};

#endif // #ifndef _COROUTINE_H
