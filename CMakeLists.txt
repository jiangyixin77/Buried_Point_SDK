# 设最小版本号
cmake_minimum_required(VERSION 3.20)

# 设置变量  PROJECT_NAME = "BuriedPoint"
set(PROJECT_NAME "BuriedPoint")

# 根目录的CMakeLists都需要有这个
project(${PROJECT_NAME} VERSION 1.1.1.1)

# 定义开关，标明是否编译动态库、静态库、示例代码、单元测试程序
option(BUILD_BURIED_SHARED_LIBS "build shared library" ON)
option(BUILD_BURIED_STATIC_LIBS "build static library" ON)
option(BUILD_BURIED_EXAMPLES "build examples" OFF)
option(BUILD_BURIED_TEST "build unittest" OFF)
option(BUILD_BURIED_FOR_MT "build for /MT" OFF)

# 启动多线程编译，Linux中此句子应删除
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")

if(BUILD_BURIED_FOR_MT)
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
endif()

# 设置变量  LIBS = LIBS + winmm + iphlpapi + ws2_32 + dbghelp + Kernel32
set(LIBS ${LIBS} winmm iphlpapi ws2_32 dbghelp Kernel32)

# 生成pdb文件
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Zi")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE
    "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")

set(INCDIRS . lib ${CMAKE_BINARY_DIR})
include_directories(${INCDIRS})

# 启动C++20标准和C11标准
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++20")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /std:c11")

# 打印信息
message(STATUS "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}")

# Debug模式下定义DEBUG宏，否则定义NDEBUG宏
if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    add_definitions(-DDEBUG)
else()
    add_definitions(-DNDEBUG)
endif()

# src子目录也会有一个cmake文件
add_subdirectory(src)

if(BUILD_BURIED_EXAMPLES)
    add_subdirectory(examples)
endif()

if(BUILD_BURIED_TEST)
    include_directories(
        googletest/googletest
        googletest/googletest/include)
    add_subdirectory(googletest)
    add_subdirectory(tests)
endif()
