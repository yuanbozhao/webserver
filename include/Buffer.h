#pragma once
#include <string>
#include<iostream>
#include <string.h>

#include <algorithm>
class Buffer{
private:

  const uint16_t sep_;    // 报文的分割符，0-无分割符，1-四字节的报头，2“\r\n\r\n”(http协议)
  static const char kCRLF[];

public:
  std::string buf_;       // 用于存放数据
  Buffer(uint16_t sep=1);
  ~Buffer();

  void append(const char* data,size_t size);  // 把数据追加到buf中
  void append(std::string& data);             // 追加
  void append(const std::string& data);
  void appendwithsep(const char* data,size_t size);// 附加报文头部
  void erase(size_t nn);      // 从pos开始删除nn个字节
  void erase(const char* end);
  size_t size();                          // 返回buf_的大小
  const char* data() const ;                     // 返回buf_的首地址
  const char* end() const ;                       // 返回尾地址
  void clear();                           // 清空buf_
  bool pickmessage(std::string &ss);      // 从buf_中拆分一个报文，存放在ss中，如果buf_没有报文，返回false;

  const char* findCRLF() const;
};