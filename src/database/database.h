#pragma once

#include <stdint.h>

#include <memory>
#include <string>
#include <vector>

namespace buried {

class BuriedDbImpl;
class BuriedDb {
 public:
  struct Data {
    int32_t id;
    int32_t priority;
    uint64_t timestamp;
    std::vector<char> content;
  };

 public:
  BuriedDb(std::string db_path);

  ~BuriedDb();

  //增删查的函数
  void InsertData(const Data& data);

  void DeleteData(const Data& data);

  void DeleteDatas(const std::vector<Data>& datas);

  //使用vector<char>是因为sqlite_orm库中文本类型需要是vector<char>
  std::vector<Data> QueryData(int32_t limit);

 private:
  //使用pimpl模式，头文件不做具体定义和实现，只保留一个指针，具体定义和实现放在源文件中
  //否则就需要在database.h中引入sqlite_orm.h，若其他头文件都引入database.h，就会使sqlite_orm.h重复引入，代码体积过大
  std::unique_ptr<BuriedDbImpl> impl_;
};

}  // namespace buried
