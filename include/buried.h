#pragma once

#include <stdint.h>

#define BURIED_EXPORT __declspec(dllexport)

// extern "C"表明对外暴露的是C接口，但对内可以使用C++开发
extern "C" {

// Buried定义在src/buried_core.h中
typedef struct Buried Buried;

// 实例配置
struct BuriedConfig {
  const char* host;// url
  const char* port;// 端口号
  const char* topic;// 话题
  const char* user_id;// 用户ID
  const char* app_version;// 应用版本号
  const char* app_name;// 应用名称
  const char* custom_data;// 自定义数据
};

// 该接口用于创建一个实例，work_dir是本实例的工作目录
BURIED_EXPORT Buried* Buried_Create(const char* work_dir);

// 该接口用于销毁一个实例
BURIED_EXPORT void Buried_Destroy(Buried* buried);

// 该接口用于开启上传能力，buried传入具体实例，config是实例配置
BURIED_EXPORT int32_t Buried_Start(Buried* buried, BuriedConfig* config);

// 该接口用户上传具体数据，buried传入具体实例，title数据标题，data具体数据，priority优先级
BURIED_EXPORT int32_t Buried_Report(Buried* buried, const char* title,
                                    const char* data, uint32_t priority);
}
