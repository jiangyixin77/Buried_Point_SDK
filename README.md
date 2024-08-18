# 埋点SDK项目
- 本项目把用户需上报的信息上传到服务端，便于开发人员及时监控客户端应用的运行状态，第一时间获取用户的行为数据和用户的使用环境信息。

# 技术栈
- C++部分新特性
- 线程池
- 定时器
- spdlog日志系统
- Json
- mbedtls加密解密算法
- HTTPS网络请求
- GoogleTest单元测试框架
- sqlite数据库
- Beast网络库

# 需求分析
- 本项目需要把用户想要上报的信息上传到服务端
- 为了在上传失败后能够重新上传，本项目需要先把上传的数据以数据库的方式持久化到文件中，上传成功后删除
- 上传时需要携带程序版本名称、用户名等信息，方便开发者定位问题
- 为了支持多个部门上传，需要支持多实例

# 流程设计
- 1.创建不同实例
- 2.创建工作目录，存每个实例的数据库文件&日志文件
- 3.获取并生成进程系统等信息（如系统版本号，用户名）
- 4.设备id（device_id）存入注册表
- 5.生成进程周期id（life_cycle_id）
- 6.配置用户相关信息
- 7.用户相关信息+进程系统等信息的json解析并存入内存，用于组装数据然后上传
- 8.启动上传功能+网络模块+数据库（获数据库handle）+定时器（5秒一操作）
- 9.按优先级，从数据库中取10条数据，存入内存，解密，上传后端
- 10.若上传成功，从数据库中删除相关数据
- 11.上传标题+数据+优先级
- 12.标题+数据+优先级连同用户相关信息+进程系统等信息，组装成一条数据，加密，存入数据库
- ![image](https://github.com/user-attachments/assets/f3b0ec2f-3801-4a42-b629-91ebdade1603)

# 代码结构设计
- include头文件
- src核心代码
- examples示例
- tests测试
- scripts脚本

# 对外接口设计
- SDK产物一般是库文件+头文件，其中头文件含对外接口
- 用户使用SDK的功能，其实也是调用这些头文件对外接口，接口越少越好
- 使用C接口而不是C++接口，因为C接口语言交互性最高，ABI兼容性最高
- 在include/buried.h中贴出对外接口
- 在src/buried_core.h中具体定义Buried
- 在src/buried.cc中做一个桥接层，做C到C++的转换

# 日志功能设计
- 为了使不同实例产生的日志放入不同的工作目录互相隔离，引入了多个logger对象，如图所示
- ![image](https://github.com/user-attachments/assets/b507620b-0a4f-4243-96ee-e01827fbd9f3)
- 图中Formatter为日志进行格式化，拼接进程ID、线程ID、时间、文件名等信息
- 图中Sinks控制日志写入的目的地，如console sink写入控制台，file sink写入文件
- 在src/buried_core.cc中定义日志
- ```注意：由于我们使用了spdlog的自定义Formatter，所以需要定义以下宏，避免Custom Formatting异常：add_definitions(-DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)```
- 在tests/test.cc中写一个对应的单元测试
