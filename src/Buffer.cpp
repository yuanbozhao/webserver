#include "Buffer.h"

// class Buffer{
// private:
//   std::string buf_;
// public:
//   Buffer();
//   ~Buffer();

//   void append(const char* data,size_t size);  // 把数据追加到buf中
//   size_t size();                          // 返回buf_的大小
//   const char* data();                     // 返回buf_的首地址
//   void clear();                           // 清空buf_

// };

const char Buffer::kCRLF[] = "\r\n";
Buffer::Buffer(uint16_t sep) : sep_(sep) {}
Buffer::~Buffer() {}

void Buffer::append(const char *data, size_t size)
{
  buf_.append(data, size);
}
void Buffer::append(std::string& data)
{
  buf_ += data;
}
void Buffer::append(const std::string& data)
{
  buf_ += data;
}
void Buffer::appendwithsep(const char *data, size_t size) // 附加报文头部
{
  if (sep_ == 0)
  {
    buf_.append(data, size);
  }
  else if (sep_ == 1)
  {
    buf_.append((char *)&size, 4); // 处理头部
    buf_.append(data, size);       // 处理报文内容
  }
  else
  {
    
  }
}

void Buffer::erase(size_t nn) // 从pos开始删除nn个字节
{
  size_t startPos = 0;
  buf_.erase(startPos, nn);
}
void Buffer::erase(const char* end) // 从pos开始删除nn个字节
{
  size_t startPos = 0;
  size_t endPos = end - this->data();
  buf_.erase(startPos,endPos);
}

size_t Buffer::size()
{
  return buf_.size();
}

const char *Buffer::data() const 
{
  return buf_.data();
}

const char* Buffer::end() const 
{
  return buf_.data() + buf_.size();
}      

void Buffer::clear()
{
  buf_.clear();
}

const char* Buffer::findCRLF() const
{
  const char* crlf = std::search(this->data(),this->end(), kCRLF, kCRLF+2);
  return crlf == this->end() ? NULL : crlf;
}

bool Buffer::pickmessage(std::string &ss) // 从buf_中拆分一个报文，存放在ss中，如果buf_没有报文，返回false;
{
  if (buf_.size() == 0)
    return false;

  if (sep_ == 0)
  {
    ss = buf_;
    buf_.clear();
  }
  else if (sep_ == 1)
  {
    int len = 0;
    memcpy(&len, buf_.data(), 4); // len
    if (buf_.size() < len + 4)
      return false;
    ss = buf_.substr(4,len);
    //std::string message(buf_.data() + 4, len);
    //message.append(inputbuffer_.data()+4,len);
    //message.insert(0,inputbuffer_.data(),4,len);
    buf_.erase(0, len + 4);
  }
  else{}
  return true;
}