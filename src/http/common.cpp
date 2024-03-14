#include "http/common.h"

std::string HttpVersionToString(HttpVersion version)
{
    switch (version) {
        case HttpVersion::Http1_0:
            return "HTTP/1.0";
        case HttpVersion::Http1_1:
            return "HTTP/1.1";
    }
}