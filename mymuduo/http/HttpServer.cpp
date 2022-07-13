#include "mymuduo/http/HttpServer.h"
#include "mymuduo/utils/Type.h"
#include "mymuduo/utils/Logger.h"
#include "mymuduo/http/HttpContext.h"
#include "mymuduo/http/HttpRequest.h"
#include "mymuduo/http/HttpResponse.h"

void defaultHttpCallback(const HttpRequest&, HttpResponse* resp)
{
  resp->setStatusCode(HttpResponse::k404NotFound);
  resp->setStatusMessage("Not Found");
  resp->setCloseConnection(true);
}
// httpserver base on server
// HTTP服务器初始化
// 设置默认的http回调404，注册连接和消息到达回调
// 消息到达需要返回html页面
// 连接到达设置httpcontext
HttpServer::HttpServer(EventLoop* loop,
                       const InetAddress& listenAddr,
                       const std::string& name,
                       TcpServer::Option option)
  : server_(loop, listenAddr, name, option),
    httpCallback_(defaultHttpCallback)
{
  server_.setConnectionCallback(
      std::bind(&HttpServer::onConnection, this, std::placeholders::_1));
  server_.setMessageCallback(
      std::bind(&HttpServer::onMessage, this, 
	  			std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void HttpServer::start()
{
  LOG_INFO("HttpServer[%s] starts listenning on %s\n",
	server_.getName(),server_.getIpPort());
  server_.start();
}

// 新连接到来，设置any为HttpContext，状态为等待请求
void HttpServer::onConnection(const TcpConnectionPtr& conn)
{
  if (conn->connected())
  {
    conn->setContext(HttpContext());
  }
}

// 此处HTTP请求到达
// 1.查看是否为HTTP内容
// 2.是否已经拿到全部内容，如果是，回调onRequest
void HttpServer::onMessage(const TcpConnectionPtr& conn,
                           Buffer* buf,
                           Timestamp receiveTime)
{
  HttpContext* context = std::any_cast<HttpContext>(conn->getMutableContext());

  if (!context->parseRequest(buf, receiveTime))
  {
    conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
    conn->shutdown();
  }

  if (context->gotAll())
  {
    onRequest(conn, context->request());
    context->reset();
  }
}

void HttpServer::onRequest(const TcpConnectionPtr& conn, const HttpRequest& req)
{
  //是否回复完就关闭连接
  const std::string& connection = req.getHeader("Connection");
  bool close = connection == "close" ||
    (req.getVersion() == HttpRequest::kHttp10 && connection != "Keep-Alive");
  HttpResponse response(close);
  // 设置行、头、体
  httpCallback_(req, &response);
  // 开辟缓冲区，将回复信息载入
  Buffer buf;
  response.appendToBuffer(&buf);
  conn->send(buf.retrieveAllAsString());
  // 是否关闭连接
  if (response.closeConnection())
  {
    conn->shutdown();
  }
}

