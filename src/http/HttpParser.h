#ifndef __HTTPPARSER_H__
#define __HTTPPARSER_H__

#include "http-parser/http_parser.h"
#include "http/IHttpParser.h"
#include "http/common.h"
#include <string>

class HttpParser : public IHttpParser
{
    HttpMethod*                         method_{nullptr};
    std::string*                        url_{nullptr};
    HttpVersion*                        version_{nullptr};
    std::map<std::string, std::string>* header_{nullptr};

    http_parser          http_parser_{};
    http_parser_settings settings_{};

    bool parse_header_field{false};

public:
    HttpParser() = default;

    void bind_method(HttpMethod* method) override;
    void bind_url(std::string* url) override;
    void bind_version(HttpVersion* version) override;
    void bind_header(std::map<std::string, std::string>* header) override;

    int parse(Buffer& buffer) override;
};

#endif  //_HttpParser_h_