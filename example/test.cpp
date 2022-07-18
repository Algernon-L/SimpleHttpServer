#include <mymuduo/http/HttpServer.h>
#include <mymuduo/http/HttpRequest.h>
#include <mymuduo/http/HttpResponse.h>
#include <mymuduo/net/EventLoop.h>
#include <mymuduo/utils/Logger.h>
#include <mymuduo/sql/SqlOperation.h>
#include <mymuduo/sql/SqlConnectionPool.h>

#include <iostream>
#include <map>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string>
#include <regex>

bool benchmark = false;

// 验证正则表达式
std::string pattern = "^[0-9a-zA-Z_]{5,16}$";
std::regex re(pattern);
bool validateString(const std::string &s){
  return std::regex_match(s, re);
}

// 将文件作为响应报文
void fileToString(const char *filename, HttpResponse* resp){
  struct stat file_stat;
  if(stat(filename, &file_stat) < 0){
    resp->setStatusCode(HttpResponse::k404NotFound);
    resp->setStatusMessage("Not Found");
    resp->setCloseConnection(true);
    return;
  };
  resp->setStatusCode(HttpResponse::k200Ok);
  resp->setStatusMessage("OK");
  int fd = open(filename, O_RDONLY);
  void *mapFile = ::mmap(NULL, file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  char *pf = static_cast<char*>(mapFile);
  resp->setBody(std::string(pf, pf + file_stat.st_size));
  close(fd);
  ::munmap(mapFile, file_stat.st_size);
}

// 传入httpserver中的httpcallback，
// 通过onMessage->onRequest->httpCallback调用
void onRequest(const HttpRequest& req, HttpResponse* resp)
{
  // 输出请求
  std::cout << "Headers " << req.methodString() << " " << req.path() << std::endl;

  if (!benchmark)
  {
    // 保存请求信息
    const std::map<std::string, std::string>& headers = req.headers();
    for (const auto& header : headers)
    {
      std::cout << header.first << ": " << header.second << "\n";
    }
  }

  // GET响应
  if (req.getMethod() == HttpRequest::kGet){
    std::string filename = req.path();
    //路由
    if(filename == "/"){
      filename = "../root/login.html";
      fileToString(filename.c_str(), resp);
      return;
    }

    if(filename.find('.') == std::string::npos){
      filename = "../root" + filename + ".html";
      fileToString(filename.c_str(), resp);
    }else{
      filename = "../root" + filename;
      fileToString(filename.c_str(), resp);
    }
  // POST响应
  }else if(req.getMethod() == HttpRequest::kPost){

    // 登录请求
    if(req.path() == "/login"){
      std::string body = req.getBody();
      int namestart = body.find('=');
      int nameend = body.find('&');
      int passwdstart = body.find('=',nameend);

      if(namestart == std::string::npos 
      || nameend == std::string::npos 
      || passwdstart == std::string::npos){
        fileToString("../root/loginfail.html", resp);
        std::cout << "npos called!" << std::endl;
      }

      std::string username = body.substr(namestart + 1, nameend - namestart - 1);
      std::string userpasswd = body.substr(passwdstart + 1, body.size() - passwdstart - 1);

      std::vector<std::string> query_res = getQueryRes(username);
      // 验证账号密码格式 和 是否存在此用户
      if(validateString(username) && validateString(userpasswd) && query_res.size() > 0 
      && username == query_res[1] && userpasswd == query_res[2]){
          fileToString("../root/index.html", resp);
      } else{
        fileToString("../root/loginfail.html", resp);
      }
    }
    // 注册请求
    else if(req.path() == "/register"){
      std::string body = req.getBody();
      int namestart = body.find('=');
      int nameend = body.find('&');
      int passwdstart = body.find('=',nameend);

      if(namestart == std::string::npos 
      || nameend == std::string::npos 
      || passwdstart == std::string::npos){
        fileToString("../root/loginfailuser404.html", resp);
      }

      std::string username = body.substr(namestart + 1, nameend - namestart - 1);
      std::string userpasswd = body.substr(passwdstart + 1, body.size() - passwdstart - 1);

      // 判断格式 and 在数据库匹配数据
      if(validateString(username) && validateString(userpasswd)){
        // FIXME:insert mysql data 
        if(insertNewUser(username, userpasswd)){
          fileToString("../root/registersucceed.html", resp);
        }else{
          fileToString("../root/registerfail.html", resp);
        }
      }
      else{
        fileToString("../root/registerfail.html", resp);
      }
    }
  // 404响应
  }else{
    resp->setStatusCode(HttpResponse::k404NotFound);
    resp->setStatusMessage("Not Found");
    resp->setCloseConnection(true);
  }
}

int main(int argc, char* argv[])
{
  // 可以设置线程数量 ./webserver 6
  int numThreads = 0;
  std::string listenAddr = "0.0.0.0";
  unsigned int listenPort = 8000;
  if (argc > 1)
  {
    benchmark = true;
    //Logger::setLogLevel(Logger::WARN);
    numThreads = atoi(argv[1]);
  }
  if (argc > 2){
    listenAddr = atoi(argv[2]);
  }
  if (argc > 3){
    listenPort = atoi(argv[3]);
  }
  SqlConnectionPool::GetInstance()->init("localhost","httpguest","httpguest","simplehttpserver",3306,4,0);

  EventLoop loop;
  // 1.HTTP服务器初始化
  HttpServer server(&loop, InetAddress(listenPort, listenAddr), "dummy");
  // 2.设置http回调函数
  server.setHttpCallback(onRequest);
  server.setThreadNum(numThreads);
  // 3.服务器运行
  server.start();
  // 4.事件循环开启
  loop.loop();
}
