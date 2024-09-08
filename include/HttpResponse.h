
#pragma once
#include <map>

#include <fstream>
#include "Buffer.h"
using namespace std;
class Buffer;
class HttpResponse
{
 public:
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

  void setStatusMessage(const string& message)
  { statusMessage_ = message; }

  void setCloseConnection(bool on)
  { closeConnection_ = on; }

  bool closeConnection() const
  { return closeConnection_; }

  void setContentType(const string& contentType)
  { addHeader("Content-Type", contentType); }

  // FIXME: replace string with StringPiece
  void addHeader(const string& key, const string& value)
  { headers_[key] = value; }

  void setBody(const string& body)
  { body_ = body; }

  std::string& body()
  {
    return body_;
  }
  std::string read_file(const std::string &path)
  {
    std::ifstream file(path);
    if (!file.is_open())
    {
      return ""; // 如果文件未打开，返回空字符串
    }
    return std::string((std::istreambuf_iterator<char>(file)),
                       (std::istreambuf_iterator<char>()));
  }
  void setFile(const string& file)
  {
    std::string html_content = read_file(file);
    if (!html_content.empty()) 
    {
      body_ = html_content;
    }
  }

  void appendToBuffer(Buffer* output) const;

 private:
  std::map<string, string> headers_;
  HttpStatusCode statusCode_;
  // FIXME: add http version
  std::string statusMessage_;
  bool closeConnection_;
  string body_;

};


