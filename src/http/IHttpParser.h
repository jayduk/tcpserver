#ifndef __IHTTPPARSER_H__
#define __IHTTPPARSER_H__

#include <map>
#include <string>

#include "common/Buffer.h"
#include "http/common.h"

class IHttpParser
{
public:
    virtual void bind_method(HttpMethod* method)                         = 0;
    virtual void bind_url(std::string* url)                              = 0;
    virtual void bind_version(HttpVersion* version)                      = 0;
    virtual void bind_header(std::map<std::string, std::string>* header) = 0;

    virtual int parse(Buffer& buffer) = 0;
};

#endif  //_IHttpParser_h_