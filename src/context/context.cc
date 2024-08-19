#include "context/context.h"

namespace buried {

// 因为我们需要两个strand并行执行，所以这里创建了两个线程，分别调度strand 
void Context::Start() {
  if (is_start_.load()) {
    return;
  }
  is_start_.store(true);

  main_thread_ = std::make_unique<std::thread>([this]() {
    for (;;) {
      if (is_stop_) {
        break;
      }
      main_context_.run();
    }
  });
  report_thread_ = std::make_unique<std::thread>([this]() {
    for (;;) {
      if (is_stop_) {
        break;
      }
      report_context_.run();
    }
  });
}

Context::~Context() {
  is_stop_ = true;
  if (main_thread_) {
    main_thread_->join();
  }
  if (report_thread_) {
    report_thread_->join();
  }
}

}  // namespace buried
