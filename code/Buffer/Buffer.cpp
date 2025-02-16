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
void Buffer::append_str(const std::string& _str) {
    buf.append(_str);
}
size_t Buffer::size() const {
    return buf.size();  
}

const char* Buffer::c_str() const {
    return buf.c_str(); 
}

void Buffer::clear() {
    buf.clear(); 
}


void Buffer::getline(){
    buf.clear();
    std::getline(std::cin, buf);
}

void Buffer::setBuf(const char* _buf) {
    buf.clear();
    buf.append(_buf, strlen(_buf));  
}

