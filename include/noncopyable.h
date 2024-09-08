#pragma once  //编译级别，防止重复包含

class noncopyable
{
    /**
     *  派生类可以正常构造和析构，但是不能拷贝构造和赋值。
    */
public:
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete; //如果要做连续赋值的话就返回noncopyable&，不然就返回void
protected:
    noncopyable() = default;
    ~noncopyable() = default;
};

