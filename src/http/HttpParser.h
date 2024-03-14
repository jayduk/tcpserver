#ifndef __HTTPPARSER_H__
#define __HTTPPARSER_H__

#include "common/ByteBuffer.h"
#include "http-parser/http_parser.h"
#include "http/common.h"
#include <map>
#include <string>

class HttpParser
{
    HttpMethod*                         method_{nullptr};
    std::string*                        url_{nullptr};
    HttpVersion*                        version_{nullptr};
    std::map<std::string, std::string>* header_{nullptr};

    http_parser          http_parser_{};
    http_parser_settings settings_{};

    bool parse_header_field{false};

public:
    int parse(ByteBuffer<>& buffer){

    };
};

#endif  //_HttpParser_h_