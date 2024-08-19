#include "buried_core.h"

#include "common/common_service.h"
#include "context/context.h"
#include "report/buried_report.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include "third_party/nlohmann/json.hpp"

void Buried::InitWorkPath_(const std::string& work_dir) {
  std::filesystem::path _work_dir(work_dir);
  if (!std::filesystem::exists(_work_dir)) {
    std::filesystem::create_directories(_work_dir);
  }

  work_path_ = _work_dir / "buried";
  if (!std::filesystem::exists(work_path_)) {
    std::filesystem::create_directories(work_path_);
  }
}

void Buried::InitLogger_() {
  // 构造console_sink
  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

  std::filesystem::path _log_dir = work_path_ / "buried.log";

  // 构造file_sink
  auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
      _log_dir.string(), true);

  //把两个sink作为参数传入到spdlog::logger的构造函数中
  logger_ = std::shared_ptr<spdlog::logger>(
      new spdlog::logger("buried_sink", {console_sink, file_sink}));

  // 自定义Formatter
  logger_->set_pattern("[%c] [%s:%#] [%l] %v");

  //设置某个logger对象的日志级别，低于此级别的日志会被忽略
  logger_->set_level(spdlog::level::trace);
}

std::shared_ptr<spdlog::logger> Buried::Logger() { return logger_; }

Buried::Buried(const std::string& work_dir) {
  buried::Context::GetGlobalContext().Start();
  InitWorkPath_(work_dir);
  InitLogger_();

  SPDLOG_LOGGER_INFO(Logger(), "Buried init success");
}
// 启动context模块，设置工作目录和logger对象

Buried::~Buried() {}

BuriedResult Buried::Start(const Config& config) {

  // 把config各项配置赋给common_service的对应成员变量
  buried::CommonService common_service;
  common_service.host = config.host;
  common_service.port = config.port;
  common_service.topic = config.topic;
  common_service.user_id = config.user_id;
  common_service.app_version = config.app_version;
  common_service.app_name = config.app_name;
  common_service.custom_data = nlohmann::json::parse(config.custom_data);

  // 创建新的BuriedReport对象，然后调用start方法，开启埋点上报
  buried_report_ = std::make_unique<buried::BuriedReport>(
      logger_, std::move(common_service), work_path_.string());
  buried_report_->Start();
  return BuriedResult::kBuriedOk;
}

BuriedResult Buried::Report(std::string title, std::string data,
                            uint32_t priority) {

  // 创建新的BuriedData对象，把title和data赋给BuriedData对应成员变量
  buried::BuriedData buried_data;
  buried_data.title = std::move(title);
  buried_data.data = std::move(data);
  buried_data.priority = priority;

  // 把BuriedData对象传递给BuriedReport对象，实现数据上报
  buried_report_->InsertData(buried_data);
  return BuriedResult::kBuriedOk;
}
