#include "Buffer.h"
#include <string.h>
#include <iostream>

Buffer::Buffer() {}

Buffer::~Buffer() {}

void Buffer::append(const char* _str, int _size) {
    if (_size > 0) {
        buf.append(_str, _size); 
    }
}

size_t Buffer::size() const {
    return buf.size();  
}

const char* Buffer::c_str() const {
    return buf.c_str();  // 返回底层数据的 c_str()
}

void Buffer::clear() {
    buf.clear();  // 清空缓冲区
}


void Buffer::getline(){
    buf.clear();
    std::getline(std::cin, buf);
}

void Buffer::setBuf(const char* _buf) {
    buf.clear();
    buf.append(_buf, strlen(_buf));  // 确保传入字符串的长度
}