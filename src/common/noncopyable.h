#ifndef UTIL_NONCOPYABLE_H_
#define UTIL_NONCOPYABLE_H_

class noncopyable
{
public:
    noncopyable(const noncopyable&) = delete;
    void operator=(const noncopyable&) = delete;

    virtual ~noncopyable() = default;

protected:
    noncopyable() = default;
};

#endif  // UTIL_NONCOPYABLE_H_