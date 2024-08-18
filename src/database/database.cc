#include "database/database.h"

#include "third_party/sqlite/sqlite_orm.h"

using namespace sqlite_orm;
namespace buried {

//创建一个数据库存储对象
inline auto InitStorage(const std::string& path) {
  return make_storage(
      path, make_table("buried_data",
                       make_column("id", &BuriedDb::Data::id,
                                   primary_key().autoincrement()),
                       make_column("priority", &BuriedDb::Data::priority),
                       make_column("timestamp", &BuriedDb::Data::timestamp),
                       make_column("content", &BuriedDb::Data::content)));
  //建立了一个表，含有id，priority等字段
}

class BuriedDbImpl {
 public:
  //定义DBStorage类型，通过调用InitStorage函数返回类型推断得到
  using DBStorage = decltype(InitStorage(""));

//用inline auto + decltype关键字推断整个storage的类型

 public:
  BuriedDbImpl(std::string db_path) : db_path_(db_path) {
    storage_ = std::make_unique<DBStorage>(InitStorage(db_path_));
    storage_->sync_schema();
  }

  ~BuriedDbImpl() {}

  //增删查操作都是使用的storage相关方法
  //这里使用了RAII概念，若中间有步骤失败，return后会在guard析构函数中触发回滚操作
  void InsertData(const BuriedDb::Data& data) {
    auto guard = storage_->transaction_guard();//开启事务并创建guard对象
    storage_->insert(data);//插入数据
    guard.commit();//提交事务
  }

  void DeleteData(const BuriedDb::Data& data) {
    auto guard = storage_->transaction_guard();
    storage_->remove_all<BuriedDb::Data>(
        where(c(&BuriedDb::Data::id) == data.id));
    guard.commit();
  }

  void DeleteDatas(const std::vector<BuriedDb::Data>& datas) {
    auto guard = storage_->transaction_guard();
    for (const auto& data : datas) {
      storage_->remove_all<BuriedDb::Data>(
          where(c(&BuriedDb::Data::id) == data.id));
    }
    guard.commit();
  }

  std::vector<BuriedDb::Data> QueryData(int32_t limit_size) {
    auto limited = storage_->get_all<BuriedDb::Data>(
        order_by(&BuriedDb::Data::priority).desc(), limit(limit_size));
    return limited;
  }

 private:
  std::string db_path_;

  //DBStorage类型的指针，用于操作数据库存储对象
  std::unique_ptr<DBStorage> storage_;
};

//BuriedDB所有成员函数的实现，都是调用的impl对应方法，实际定义都在BuriedDbImpl中
BuriedDb::BuriedDb(std::string db_path)
    : impl_{std::make_unique<BuriedDbImpl>(std::move(db_path))} {}

BuriedDb::~BuriedDb() {}

void BuriedDb::InsertData(const Data& data) { impl_->InsertData(data); }

void BuriedDb::DeleteData(const Data& data) { impl_->DeleteData(data); }

void BuriedDb::DeleteDatas(const std::vector<Data>& datas) {
  impl_->DeleteDatas(datas);
}

std::vector<BuriedDb::Data> BuriedDb::QueryData(int32_t limit) {
  return impl_->QueryData(limit);
}

}  // namespace buried
