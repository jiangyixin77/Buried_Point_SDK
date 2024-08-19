#pragma once

#include <stdint.h>

#include <memory>
#include <string>

#include "common/common_service.h"

namespace spdlog {
class logger;
}

namespace buried {

struct BuriedData {
  std::string title;
  std::string data;
  uint32_t priority;
};//BuriedData结构包含了标题、数据、优先级，由外部调用方传入这些参数

class BuriedReportImpl;
class BuriedReport {
 public:
  BuriedReport(std::shared_ptr<spdlog::logger> logger,
               CommonService common_service, std::string work_path);// 因为整个SDK需支持多实例，所以内部各模块需要传递进来子模块实例对象和工作目录

  ~BuriedReport();

  void Start();// 调用此接口后，开始获取数据并上报

  void InsertData(const BuriedData& data);// 外部调用方上报数据时，调用此接口，然后模块内部择机上报数据

 private:
  std::unique_ptr<BuriedReportImpl> impl_;// pimpl模式，相关实现都放在源文件中，不暴露到外边
};

}  // namespace buried
