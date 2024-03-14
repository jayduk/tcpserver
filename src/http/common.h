#ifndef __COMMON_H__
#define __COMMON_H__

enum class HttpVersion
{
    Http1_0 = 10,
    Http1_1 = 11,
};

enum class HttpMethod
{
    DELETE = 0,
    GET,
    HEAD,
    POST,
    PUT,
    CONNECT,
    OPTION,
    TRACE,
};

#endif  //_common_h_