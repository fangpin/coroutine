#ifndef _COROUTINE_H
#define _COROUTINE_H

#include <functional>
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <exception>

#include <ucontext.h>

const static int STACK_SIZE = 1024 * 1024 * 2;
const static int DEFAULT_COROUTINES = 16;

class CoroutineBaseException:public std::exception {
public:
  CoroutineBaseException(const std::string& msg): msg_(msg) {}
  virtual const char* what() {
    return msg_.c_str();
  }
private:
  std::string msg_;
};

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
  Coroutine(FUNC &&func, ARGS &&... args) : status_(CO_READY) {
    run_ = [&]() { func(std::forward<ARGS...>(args...)); };
  }

  std::function<void()> run_;
  std::vector<char> stack_;
  ucontext_t context_;
  CoroutineStatus status_;

  friend class Scheduler;
};

class Scheduler {
public:
  Scheduler() : running_(-1) { stack_.resize(STACK_SIZE); }

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
          it = std::make_shared<Coroutine>(std::forward<FUNC>(func), std::forward<ARGS...>(args...));
        }
      }
    }
    ++nco_;
  }

  void yield() {
    Coroutine* co = coroutines_[running_].get();
    running_ = -1;
    co->stack_.clear();
    co->status_ = CO_SUSPEND;
    std::copy(stack_.begin(), stack_.end(), std::back_inserter(co->stack_));
    swap_context(&co->context_, &main_);
  }

  void resume(cid_t cid) {
    if (cid >= nco_) {
      throw CoroutineBaseException(std::string("invalid cid: ") +
        std::to_string(cid));
    }
    coroutines_* co = coroutines_.[cid].get();
    switch (co->status_) {
      case CO_DEAD:
        throw CoroutineBaseException("Current coroutine is dead.");
      case CO_SUSPEND:

        break;
      case CO_SUSPEND:

        break;
      default:
        throw CoroutineBaseException("Unknown coroutine status");
    }
  }

private:
  size_t nco_;
  size_t running_;
  std::vector<std::shared_ptr<Coroutine>> coroutines_;
  std::vector<char> stack_;
  ucontext_t main_;
};

#endif // #ifndef _COROUTINE_H
