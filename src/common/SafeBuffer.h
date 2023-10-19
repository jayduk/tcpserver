#ifndef COMMON_SAFEBUFFER_H_
#define COMMON_SAFEBUFFER_H_

#include "common/Buffer.h"
#include <mutex>
class SafeBuffer
{
private:
    Buffer buffer_;
    std::mutex access_mt_;
    int access_idx_;

    bool ready_;

public:
    SafeBuffer();
    ~SafeBuffer() = default;

public:
};

#endif  // COMMON_SAFEBUFFER_H_