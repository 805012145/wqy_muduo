#pragma once

/*
    noncopyable 被继承以后，派生类可以正常构造和析构，但是无法被拷贝赋值
 */
class noncopyable {
public:
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;
protected:
    noncopyable() = default;
    ~noncopyable() = default;    
};