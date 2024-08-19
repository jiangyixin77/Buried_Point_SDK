#pragma once

#include <stdint.h>

#include <memory>
#include <string>

namespace spdlog {
class logger;
}

namespace buried {

// 封装HTTP接口，封装和简化HTTP请求的过程
class HttpReporter {
 public:
  // 接收记录日志的函数
  explicit HttpReporter(std::shared_ptr<spdlog::logger> logger);

  // 定义公有成员函数，用于设置HTTP请求的各个参数
  HttpReporter& Host(const std::string& host) {
    host_ = host;
    return *this;
  }
  HttpReporter& Topic(const std::string& topic) {
    topic_ = topic;
    return *this;
  }
  HttpReporter& Port(const std::string& port) {
    port_ = port;
    return *this;
  }
  HttpReporter& Body(const std::string& body) {
    body_ = body;
    return *this;
  }

  // 该函数使用之前设的参数，使用Beast库发起HTTP请求，并返回bool值表示请求是否成功
  bool Report();

 private:
  std::string host_;
  std::string topic_;
  std::string port_;
  std::string body_;

  std::shared_ptr<spdlog::logger> logger_;
};

}  // namespace buried
