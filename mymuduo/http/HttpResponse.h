#pragma once
#include "mymuduo/utils/copyable.h"
#include <string>
#include <map>

class Buffer;
class HttpResponse : public copyable
{
 public:
 // HTTP 状态码
  enum HttpStatusCode
  {
    kUnknown,
    k200Ok = 200,
    k301MovedPermanently = 301,
    k400BadRequest = 400,
    k404NotFound = 404,
  };

  explicit HttpResponse(bool close)
    : statusCode_(kUnknown),
      closeConnection_(close)
  {
  }

  void setStatusCode(HttpStatusCode code)
  { statusCode_ = code; }

  void setStatusMessage(const std::string& message)
  { statusMessage_ = message; }

  void setCloseConnection(bool on)
  { closeConnection_ = on; }

  bool closeConnection() const
  { return closeConnection_; }

  void setContentType(const std::string& contentType)
  { addHeader("Content-Type", contentType); }

  // FIXME: replace string with StringPiece
  void addHeader(const std::string& key, const std::string& value)
  { headers_[key] = value; }

  void setBody(const std::string& body)
  { body_ = body; }

  void appendToBuffer(Buffer* output) const;

 private:
 // HTTP组成：状态码，首部，实体。
  std::map<std::string, std::string> headers_;// 保存所有httpheaders
  HttpStatusCode statusCode_;// 状态码
  std::string statusMessage_;// 状态信息
  bool closeConnection_;// 是否已关闭连接
  std::string body_;// httpbody
};