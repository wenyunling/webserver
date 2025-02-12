#pragma once
#include <string>

class Buffer
{
private:
    std::string buf;
public:
    Buffer();
    ~Buffer();
    
    void append(const char* _str, int _size);  // 向缓冲区追加数据
    size_t size() const;  // 获取缓冲区的大小
    const char* c_str() const;  // 获取缓冲区的 C 风格字符串
    void clear();  // 清空缓冲区
    void getline();  // 从指定输入流读取数据
    void setBuf(const char* _buf);  // 设置缓冲区内容
};