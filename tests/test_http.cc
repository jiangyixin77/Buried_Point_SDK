#include <cstdlib>
#include <iostream>
#include <string>

#include "gtest/gtest.h"
#include "src/third_party/boost/asio/connect.hpp"
#include "src/third_party/boost/asio/ip/tcp.hpp"
#include "src/third_party/boost/beast/core.hpp"
#include "src/third_party/boost/beast/http.hpp"
#include "src/third_party/boost/beast/version.hpp"

namespace beast = boost::beast;  // from <boost/beast.hpp>
namespace http = beast::http;    // from <boost/beast/http.hpp>
namespace net = boost::asio;     // from <boost/asio.hpp>
using tcp = net::ip::tcp;        // from <boost/asio/ip/tcp.hpp>

// Performs an HTTP POST and prints the response
// 这段代码是一个使用Boost的Beast库发起HTTP请求的示例
TEST(BuriedHttpTest, DISABLED_HttpPost) {
  try {
    //指定请求的主机名和目标路径
    auto const host = "localhost";
    auto const target = "/buried";
    int version = 11;

    // 创建io_context对象，其对所有I/O操作都是必须的，可理解是I/O操作运行时环境
    net::io_context ioc;

    // 创建resolver对象和tcp_stream对象，用于执行I/O操作
    tcp::resolver resolver(ioc);
    beast::tcp_stream stream(ioc);

    // 使用resolver解析host获地址和端口号，然后使用tcp_stream与服务器建立TCP连接
    boost::asio::ip::tcp::resolver::query query(host, "5678");
    
    // Look up the domain name
    auto const results = resolver.resolve(query);

    // Make the connection on the IP address we get from a lookup
    stream.connect(results);

    // 设立 HTTP POST 请求信息，设置请求方法、目标路径、HTTP版本、头字段、请求主题
    http::request<http::string_body> req{http::verb::post, target, version};
    req.set(http::field::host, host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    req.set(http::field::content_type, "application/json");
    req.body() = "{\"hello\":\"world\"}";
    req.prepare_payload();// 用于构造整个请求体

    // 发送HTTP请求至远程host
    http::write(stream, req);

    // 接收服务器响应的缓冲区
    beast::flat_buffer buffer;

    // 存储响应的容器
    http::response<http::dynamic_body> res;

    // 接收HTTP响应
    http::read(stream, buffer, res);

    // 响应主体转化为字符串，并打印
    std::string bdy = boost::beast::buffers_to_string(res.body().data());
    std::cout << "bdy " << bdy << std::endl;
    // Write the message to standard out
    std::cout << "res " << res << std::endl;
    std::cout << "res code " << res.result_int() << std::endl;

    // 关闭连接
    beast::error_code ec;
    stream.socket().shutdown(tcp::socket::shutdown_both, ec);

    
    // 报错
    if (ec && ec != beast::errc::not_connected) throw beast::system_error{ec};

  } catch (std::exception const& e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
}
