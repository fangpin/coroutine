#ifndef _COROUTINE_H
#define _COROUTINE_H

#include <vector>
#include <ucontex.h>

namespace Fang {

const static int STACK_SIZE = 1024 * 1024 * 2;

enum CoroutineStatus {
  CO_READY,
  CO_RUNNING,
  CO_SUSPEND,
  CO_DEAD,
};

class Coroutine{

public:
  template<typename FUNC, typename... ARGS>
  Coroutine(FUNC&& func, ARGS...&& args)
    : status_(CO_READY), stackSize_(0) {
    run_ = [std::forward<FUNC>(func), std::forward<ARGS...>(args)] () {
            func(...args);
          };
  }

  void operator()() {
    run_();
  }

    CoroutineStatus getStatus() const {
        return status_;
    }

private:
  std::function<void()> run_;
  std::vector<char> stack_;
  size_t stackSize_;
  CoroutineStatus status_;

  friend class Scheduler;
};

class Scheduler {
public:
    Scheduler() {
        stack_.resize(STACK_SIZE);
    }
private:
  size_t nco_;
  size_t running_;
  std::vector<Coroutine> coroutines_;
  vector<char> stack_;
  ucontex_t main_;
};

} // namespace Fang


#endif // #ifndef _COROUTINE_H
