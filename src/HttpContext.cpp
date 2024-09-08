#include "HttpContext.h"
#include <string>

bool HttpContext::processRequestLine(const char *begin, const char *end)
{
  bool succeed = false;
  const char *start = begin;
  const char *space = std::find(start, end, ' ');
  if (space != end && request_.setMethod(start, space))
  {
    start = space + 1;
    space = std::find(start, end, ' ');
    if (space != end)
    {
      const char *question = std::find(start, space, '?');
      if (question != space)
      {
        request_.setPath(start, question);
        request_.setQuery(question, space);
      }
      else
      {
        request_.setPath(start, space);
      }
      start = space + 1;
      succeed = end - start == 8 && std::equal(start, end - 1, "HTTP/1.");
      if (succeed)
      {
        if (*(end - 1) == '1')
        {
          //std::cout << "http协议为1.1" << std::endl;
          request_.setVersion(HttpRequest::kHttp11);
        }
        else if (*(end - 1) == '0')
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
bool HttpContext::parseRequest(Buffer *buf, Timestamp receiveTime)
{
  bool ok = true;
  bool hasMore = true;
  while (hasMore)
  {
    if (state_ == kExpectRequestLine)
    {
      //LOG_INFO("state_ == kExpectRequestLine");
      const char *crlf = buf->findCRLF();
      if (crlf)
      {
        ok = processRequestLine(buf->data(), crlf);
        if (ok)
        {
          // std::cout << "成功了" <<std::endl;
          // std::cout <<
          // "method" << request_.method() <<
          // " version: "<<request_.getVersion()<<
          // " path: "<<request_.path()<<
          // std::endl;
          request_.setReceiveTime(receiveTime);
          buf->erase(crlf + 2);
          // size_t start = crlf - buf->data();  // 计算起始位置
          // buf->buf_ = buf->buf_.substr(start+2);  // 删除从 start 开始的 length 长度
          state_ = kExpectHeaders;
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
    else if (state_ == kExpectHeaders)
    {
      //std::cout << "kExpectHeaders"<<std::endl;
      const char *crlf = buf->findCRLF();
      if (crlf)
      {
        const char *colon = std::find(buf->data(), crlf, ':');
        if (colon != crlf)
        {
          request_.addHeader(buf->data(), colon, crlf);
        }
        else
        {
          
          // empty line, end of header
          // FIXME:
          state_ = kExpectBody;
          //hasMore = false;
        }
        buf->erase(crlf + 2);
      }
      else
      {
        hasMore = false;
      }
    }
    else if (state_ == kExpectBody)
    {
      //std::cout << "state_ == kExpectBody"<<std::endl;
      // 获取请求体的长度
      if(request_.method() == request_.kGet)
      {
        state_ = kGotAll;
        hasMore = false;
        //std::cout << "request_.kGet"<<std::endl;
      }
      else
      {
        int contentLength = stoi(request_.getHeader("Content-Length"));
        if(contentLength >0)
        {
          request_.setBody(buf->data(), contentLength);
          buf->erase(contentLength);
        }
        // 如果没有 Content-Length，或者 Content-Length 为 0，则请求体为空
        state_ = kGotAll;
        hasMore = false;
      } 
    }
    else
    {
      hasMore = false;
    }
  }
  return ok;
}
