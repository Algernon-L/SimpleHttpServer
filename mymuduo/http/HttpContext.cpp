#include "mymuduo/net/Buffer.h"
#include "mymuduo/http/HttpContext.h"

// 处理请求行
// 1.设置方法GET or POST，2.设置URL，3.设置HTTP version
bool HttpContext::processRequestLine(const char* begin, const char* end)
{
  bool succeed = false;
  const char* start = begin;
  const char* space = std::find(start, end, ' ');
  // 设置方法 GET OR POST
  if (space != end && request_.setMethod(start, space))
  {
    start = space+1;
    space = std::find(start, end, ' ');
    // 设置URL
    if (space != end)
    {
      const char* question = std::find(start, space, '?');
      if (question != space)
      {
        request_.setPath(start, question);
        request_.setQuery(question, space);
      }
      else
      {
        request_.setPath(start, space);
      }
      // 设置HTTP version
      start = space+1;
      succeed = end-start == 8 && std::equal(start, end-1, "HTTP/1.");
      if (succeed)
      {
        if (*(end-1) == '1')
        {
          request_.setVersion(HttpRequest::kHttp11);
        }
        else if (*(end-1) == '0')
        {
          request_.setVersion(HttpRequest::kHttp10);
        }
        else
        {
          succeed = false;
        }
      }
    }
  }
  return succeed;
}

// return false if any error
// 解析请求
// 1.处理请求行，2.处理请求头，3.处理请求体
bool HttpContext::parseRequest(Buffer* buf, Timestamp receiveTime)
{
  bool ok = true;
  bool hasMore = true;
  while (hasMore)
  {
    // 初始化状态kExpectRequestLine
    if (state_ == kExpectRequestLine)
    {
      const char* crlf = buf->findCRLF();
      // 存在回车换行
      if (crlf)
      {
        // 处理请求行
        ok = processRequestLine(buf->peek(), crlf);
        if (ok)
        {
          request_.setReceiveTime(receiveTime);
          buf->retrieveUntil(crlf + 2);//"\r\n"
          state_ = kExpectHeaders;// 转为获取请求头状态
        }
        else
        {
          hasMore = false;
        }
      }
      else
      {
        hasMore = false;
      }
    }
    // 获取请求头 用map存储
    else if (state_ == kExpectHeaders)
    {
      const char* crlf = buf->findCRLF();
      if (crlf)
      {
        const char* colon = std::find(buf->peek(), crlf, ':');
        if (colon != crlf)
        {
          request_.addHeader(buf->peek(), colon, crlf);
        }
        else
        {
          // empty line, end of header
          // FIXME:
          state_ = kGotAll;
          hasMore = false;
        }
        buf->retrieveUntil(crlf + 2);
      }
      else
      {
        hasMore = false;
      }
    }
    // 请求体
    else if (state_ == kExpectBody)
    {
      // FIXME:
    }
  }
  return ok;
}
