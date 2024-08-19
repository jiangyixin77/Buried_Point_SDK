# 埋点SDK项目
- 本项目把用户需上报的信息上传到服务端，便于开发人员及时监控客户端应用的运行状态，第一时间获取用户的行为数据和用户的使用环境信息。

# 技术栈
- C++部分新特性
- 线程池
- 定时器
- spdlog日志系统
- Json
- mbedtls加密解密算法
- HTTP网络请求
- GoogleTest单元测试框架
- sqlite数据库
- Beast网络库
- pimpl模式

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
- include头文件，放置声明函数
- src核心代码，放置定义函数
- examples示例，放置示例代码
- tests测试，放置测试代码
- scripts脚本，放置编译脚本

# 程序编译
- 使用cmake构建来编译动态库静态库
- 脚本写好后编译程序：```python scripts\build.py --test --example```
- 生成了buried_test.exe和buried_example.exe两个可执行文件
- 执行可执行文件
- ![image](https://github.com/user-attachments/assets/8fac0e5c-5eb4-407a-a865-0fea693a09c7)

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

# 上报协议设计
- **1.通信协议选择**
- SDK流程是隔一段时间上传一次信息/发送一次网络请求
- udp不可靠；tcp不知道读多少字节且占用大量后端资源；所以用http协议
- **2.序列化方式选择**
- JSON集成使用方便，易读，但序列化慢，体积大；Protobuf相反；这里使用JSON
- **3.上报字段选择**
- 上报字段包含了用户号、应用名称等等，把若干个json object组装成一个json array然后统一用一个https去上传，再配合host和topic，上传到host/topic下

# 数据库设计
- 使用sqlite_orm
- 数据库需要储存：数据ID，数据优先级，数据时间戳，数据内容
- 在src/database/database.h以及src/database/database.cc中进行数据库设计

# 发起HTTP请求
- **HTTP请求是基于TCP协议的，它涉及以下步骤：**
- 1.解析域名获取IP地址与端口号
- 2.建立TCP连接并获取套接字通道
- 3.向通道写入HTTP信息，向服务器发数据
- 4.从通道接收返回数据
- 5.关闭TCP连接
- **在该项目的具体实现**
- 使用Beast作为HTTP请求的库
- 在tests/test_http.cc中使用Beast发起HTTP请求
- 为了避免每次发起网络请求都写大量代码，我们在src/report/http_report.h以及src/report/http_report.cc中，基于需求封装一套好用的HTTP接口并实现
- 接口都实现好后，我们就可以方便的发起HTTP请求。
- 在安装Rust环境的情况下，可以进入server目录，执行cargo run命令进行测试

# AES加密解密算法
- 在src/crypt/crypt.h以及src/crypt/crypt.cc中，生成一个用于AES加密的密钥
- 将随机盐值与用户提供的密码结合，再使用PBKDF2算法和HMAC-SHA256算法来生成密钥
- 将生成密钥转化为std::string类型返回
- 在tests/test_crypt.cc中，对加密解密方法进行测试

# strand多线程开发
- 避免任务并发执行时可能的数据竞争问题，确保任务顺序执行
- 相关代码：
- src/context/context.h;
- src/context/context.cc;
- examples/context_example.cc

# 上报模块实现
- 设置开关，打开开关后，塞进来的消息每隔一段时间自动上报，上传失败的数据需要重新上传
- 通过定时器来保证消息每隔一段时间自动上报
- 存取数据时要加密解密
- 通过strand调度避免数据竞争

# 整体功能组装
- src/buried_core.cc
