#include "report/buried_report.h"

#include <chrono>
#include <filesystem>

#include "boost/asio/deadline_timer.hpp"
#include "boost/asio/io_service.hpp"
#include "context/context.h"
#include "crypt/crypt.h"
#include "database/database.h"
#include "report/http_report.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

namespace buried {

static const char kDbName[] = "buried.db";

class BuriedReportImpl {
 // 整个Report相关方法，都是通过ReportStrand调度的，既在子线程调用，又确保了所有方法都在一个线程实现
 public:
  BuriedReportImpl(std::shared_ptr<spdlog::logger> logger,
                   CommonService common_service, std::string work_path)
      : logger_(std::move(logger)),
        common_service_(std::move(common_service)),
        work_dir_(std::move(work_path)) {
    if (logger_ == nullptr) {
      logger_ = spdlog::stdout_color_mt("buried");
    }
    std::string key = AESCrypt::GetKey("buried_salt", "buried_password");
    crypt_ = std::make_unique<AESCrypt>(key);
    SPDLOG_LOGGER_INFO(logger_, "BuriedReportImpl init success");
    Context::GetGlobalContext().GetReportStrand().post([this]() { Init_(); });
    // 构造或赋值了logger和crypt对象后，通过ReportStrand调度Init方法
  }

  ~BuriedReportImpl() = default;

  void Start();

  void InsertData(const BuriedData& data);

 private:
  void Init_();

  void ReportCache_();

  void NextCycle_();

  BuriedDb::Data MakeDbData_(const BuriedData& data);

  std::string GenReportData_(const std::vector<BuriedDb::Data>& datas);

  bool ReportData_(const std::string& data);

 private:
  std::shared_ptr<spdlog::logger> logger_;//打印日志的实例
  std::string work_dir_;//工作目录
  std::unique_ptr<BuriedDb> db_;//数据库实例
  CommonService common_service_;//公共数据获取
  std::unique_ptr<buried::Crypt> crypt_;//加密解密
  std::unique_ptr<boost::asio::deadline_timer> timer_;//定时器
  std::vector<BuriedDb::Data> data_caches_;//上报数据装载
};

void BuriedReportImpl::Init_() {
  std::filesystem::path db_path = work_dir_;
  SPDLOG_LOGGER_INFO(logger_, "BuriedReportImpl init db path: {}",
                     db_path.string());
  db_path /= kDbName;
  db_ = std::make_unique<BuriedDb>(db_path.string());
}// 传入数据库路径，构建数据库对象

void BuriedReportImpl::Start() {
  SPDLOG_LOGGER_INFO(logger_, "BuriedReportImpl start");

  timer_ = std::make_unique<boost::asio::deadline_timer>(
      Context::GetGlobalContext().GetMainContext(),
      boost::posix_time::seconds(5));

  timer_->async_wait(Context::GetGlobalContext().GetReportStrand().wrap(
      [this](const boost::system::error_code& ec) {
        if (ec) {
          logger_->error("BuriedReportImpl::Start error: {}", ec.message());
          return;
        }
        ReportCache_();
      }));
}// 创建定时器，每5秒调用一次ReportCache_

void BuriedReportImpl::InsertData(const BuriedData& data) {
  Context::GetGlobalContext().GetReportStrand().post(
      [this, data]() { db_->InsertData(MakeDbData_(data)); });
}// 向上报模块插入数据

bool BuriedReportImpl::ReportData_(const std::string& data) {
  HttpReporter reporter(logger_);
  return reporter.Host(common_service_.host)
      .Topic(common_service_.topic)
      .Port(common_service_.port)
      .Body(data)
      .Report();
}

void BuriedReportImpl::ReportCache_() {
  SPDLOG_LOGGER_INFO(logger_, "BuriedReportImpl report cache");
  if (data_caches_.empty()) {
    data_caches_ = db_->QueryData(10);
  }

  if (!data_caches_.empty()) {
    std::string report_data = GenReportData_(data_caches_);
    if (ReportData_(report_data)) {
      db_->DeleteDatas(data_caches_);
      data_caches_.clear();
    }
  }

  NextCycle_();
}// 按优先级取10条数据，再构造网络上报的body，再去上报数据；成功后删除相应数据，然后启动下一次定时任务

std::string BuriedReportImpl::GenReportData_(
    const std::vector<BuriedDb::Data>& datas) {
  nlohmann::json json_datas;
  for (const auto& data : datas) {
    std::string content =
        crypt_->Decrypt(data.content.data(), data.content.size());
    SPDLOG_LOGGER_INFO(logger_, "BuriedReportImpl report data content size: {}",
                       data.content.size());
    json_datas.push_back(content);
  }
  std::string ret = json_datas.dump();
  return ret;
}// 构造网络上报的body，解密，构造成json array，最后序列化为string

BuriedDb::Data BuriedReportImpl::MakeDbData_(const BuriedData& data) {
  BuriedDb::Data db_data;
  db_data.id = -1;
  db_data.priority = data.priority;
  db_data.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                          std::chrono::system_clock::now().time_since_epoch())
                          .count();
  nlohmann::json json_data;
  json_data["title"] = data.title;
  json_data["data"] = data.data;
  json_data["user_id"] = common_service_.user_id;
  json_data["app_version"] = common_service_.app_version;
  json_data["app_name"] = common_service_.app_name;
  json_data["custom_data"] = common_service_.custom_data;
  json_data["system_version"] = common_service_.system_version;
  json_data["device_name"] = common_service_.device_name;
  json_data["device_id"] = common_service_.device_id;
  json_data["buried_version"] = common_service_.buried_version;
  json_data["lifecycle_id"] = common_service_.lifecycle_id;
  json_data["priority"] = data.priority;
  json_data["timestamp"] = CommonService::GetNowDate();
  json_data["process_time"] = CommonService::GetProcessTime();
  json_data["report_id"] = CommonService::GetRandomId();
  std::string report_data = crypt_->Encrypt(json_data.dump());
  db_data.content = std::vector<char>(report_data.begin(), report_data.end());
  SPDLOG_LOGGER_INFO(logger_, "BuriedReportImpl insert data size: {}",
                     db_data.content.size());

  return db_data;
} //把数据存入数据库中

void BuriedReportImpl::NextCycle_() {
  SPDLOG_LOGGER_INFO(logger_, "BuriedReportImpl next cycle");
  timer_->expires_at(timer_->expires_at() + boost::posix_time::seconds(5));
  timer_->async_wait([this](const boost::system::error_code& ec) {
    if (ec) {
      logger_->error("BuriedReportImpl::NextCycle_ error: {}", ec.message());
      return;
    }
    Context::GetGlobalContext().GetReportStrand().post(
        [this]() { ReportCache_(); });
  });
} //用定时器，启动下一次循环任务

// ========

BuriedReport::BuriedReport(std::shared_ptr<spdlog::logger> logger,
                           CommonService common_service, std::string work_path)
    : impl_(std::make_unique<BuriedReportImpl>(
          std::move(logger), std::move(common_service), std::move(work_path))) {
}

void BuriedReport::Start() { impl_->Start(); }

void BuriedReport::InsertData(const BuriedData& data) {
  impl_->InsertData(data);
}

BuriedReport::~BuriedReport() {}

}  // namespace buried
