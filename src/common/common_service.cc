//本代码用以获取公共信息，包括版本号、设备名称及序号、时间戳，等等。
#include "common/common_service.h"

#include <windows.h>

#include <algorithm>
#include <chrono>
#include <ctime>
#include <random>

#include "buried_config.h"

namespace buried {

CommonService::CommonService() { Init(); }
//把设备id号存到注册表内，保证重启设备后，获取到的设备id号一致
static void WriteRegister(const std::string& key, const std::string& value) {
  HKEY h_key;
  LONG ret = ::RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\Buried", 0, NULL,
                               REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
                               &h_key, NULL);
  if (ret != ERROR_SUCCESS) {
    return;
  }
  ret = ::RegSetValueExA(h_key, key.c_str(), 0, REG_SZ,
                         reinterpret_cast<const BYTE*>(value.c_str()),
                         value.size());
  if (ret != ERROR_SUCCESS) {
    return;
  }
  ::RegCloseKey(h_key);
}
//读取注册表，获取之前存储的设备id号
static std::string ReadRegister(const std::string& key) {
  HKEY h_key;
  LONG ret = ::RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Buried", 0,
                             KEY_ALL_ACCESS, &h_key);
  if (ret != ERROR_SUCCESS) {
    return "";
  }
  char buf[1024] = {0};
  DWORD buf_size = sizeof(buf);
  ret = ::RegQueryValueExA(h_key, key.c_str(), NULL, NULL,
                           reinterpret_cast<BYTE*>(buf), &buf_size);
  if (ret != ERROR_SUCCESS) {
    return "";
  }
  ::RegCloseKey(h_key);
  return buf;
}
//获取设备id号，生成随机数作为设备id，然后存到设备注册表内
static std::string GetDeviceId() {
  static constexpr auto kDeviceIdKey = "device_id";
  static std::string device_id = ReadRegister(kDeviceIdKey);
  if (device_id.empty()) {
    device_id = CommonService::GetRandomId();
    WriteRegister(kDeviceIdKey, device_id);
  }
  return device_id;
}
//获取进程id
static std::string GetLifeCycleId() {
  static std::string life_cycle_id = CommonService::GetRandomId();
  return life_cycle_id;
}
//获取Windows系统版本
static std::string GetSystemVersion() {
  OSVERSIONINFOEXA os_version_info;
  ZeroMemory(&os_version_info, sizeof(OSVERSIONINFOEXA));
  os_version_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXA);
  ::GetVersionExA(reinterpret_cast<OSVERSIONINFOA*>(&os_version_info));
  std::string system_version =
      std::to_string(os_version_info.dwMajorVersion) + "." +
      std::to_string(os_version_info.dwMinorVersion) + "." +
      std::to_string(os_version_info.dwBuildNumber);
  return system_version;
}
//获取设备名称
static std::string GetDeviceName() {
  char buf[1024] = {0};
  DWORD buf_size = sizeof(buf);
  ::GetComputerNameA(buf, &buf_size);
  std::string device_name = buf;
  return device_name;
}
//获取进程时间，需从FILETIME转为SYSTEMTIME
std::string CommonService::GetProcessTime() {
  DWORD pid = ::GetCurrentProcessId();
  HANDLE h_process =
      ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
  if (h_process == NULL) {
    return "";
  }
  FILETIME create_time;
  FILETIME exit_time;
  FILETIME kernel_time;
  FILETIME user_time;
  BOOL ret = ::GetProcessTimes(h_process, &create_time, &exit_time,
                               &kernel_time, &user_time);
  ::CloseHandle(h_process);
  if (ret == 0) {
    return "";
  }

  FILETIME create_local_time;
  ::FileTimeToLocalFileTime(&create_time, &create_local_time);

  SYSTEMTIME create_sys_time;
  ::FileTimeToSystemTime(&create_local_time, &create_sys_time);
  char buf[128] = {0};
  // year month day hour minute second millisecond
  sprintf_s(buf, "%04d-%02d-%02d %02d:%02d:%02d.%03d", create_sys_time.wYear,
            create_sys_time.wMonth, create_sys_time.wDay, create_sys_time.wHour,
            create_sys_time.wMinute, create_sys_time.wSecond,
            create_sys_time.wMilliseconds);
  return buf;
}
//获取日期
std::string CommonService::GetNowDate() {
  auto t =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  return std::ctime(&t);
}
//获取随机数
std::string CommonService::GetRandomId() {
  static constexpr size_t len = 32;
  static constexpr auto chars =
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz";
  static std::mt19937_64 rng{std::random_device{}()};
  static std::uniform_int_distribution<size_t> dist{0, 60};
  std::string result;
  result.reserve(len);
  std::generate_n(std::back_inserter(result), len,
                  [&]() { return chars[dist(rng)]; });
  return result;
}
//获取公共信息，包括版本号、设备名称及序号、时间戳，等等。
void CommonService::Init() {
  system_version = GetSystemVersion();
  device_name = GetDeviceName();
  device_id = GetDeviceId();
  buried_version = PROJECT_VER;//从头文件"buried_config.h"中获取
  lifecycle_id = GetLifeCycleId();
}

}  // namespace buried
