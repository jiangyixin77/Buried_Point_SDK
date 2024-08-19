#pragma once

#include <stdint.h>

#include <filesystem>
#include <memory>
#include <string>

#include "buried_common.h"
#include "include/buried.h"

namespace spdlog {
class logger;
}

namespace buried {
class BuriedReport;
}

struct Buried {
 public:
  struct Config {
    std::string host;
    std::string port;
    std::string topic;
    std::string user_id;
    std::string app_version;
    std::string app_name;
    std::string custom_data;
  };

 public:
  Buried(const std::string& work_dir);

  ~Buried();

  BuriedResult Start(const Config& config);// 通过config启动上报

  BuriedResult Report(std::string title, std::string data, uint32_t priority);// 使用Report接口上报数据

 public:
  std::shared_ptr<spdlog::logger> Logger();// 对外的logger接口，用于获取此对象内部的logger实例

 private:
  void InitWorkPath_(const std::string& work_dir);

  void InitLogger_();

 private:
  std::shared_ptr<spdlog::logger> logger_;// logger对象
  std::unique_ptr<buried::BuriedReport> buried_report_; //BuriedReport对象
  std::filesystem::path work_path_;// 工作路径
};
// 通过调用公共接口，构造对象、启动上报能力、上报数据
