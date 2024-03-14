
#include <cstddef>

class HttpStream
{
public:
    HttpStream()  = default;
    ~HttpStream() = default;

public:
    void append(const char* data, size_t len);
};