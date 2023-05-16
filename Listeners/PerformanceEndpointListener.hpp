#pragma once

#include <bits/stdc++.h>
#include <cjson/cJSON.h>
#include <fmt/format.h>

#include "../Abstract/EndpointListener.hpp"
#include "../Utility/HttpSession.hpp"
#include "../Utility/Stopwatch.hpp"
#include "../Utility/Logging.hpp"

using namespace Logging;
using namespace Logging::IP7_Targets;

class PerformanceEndpointListener : public EndpointListener {

private:

    optional<vector<char>> _buffer;
    
    optional<double> ReceiveHandler(const std::vector<char> &buffer) override {
        LOG(TRACE, TM("[%hs] CONTENT RECEIVED"), _url.c_str());
        if (!_buffer.has_value()) {
            _buffer = buffer;
            LOG(INFO, TM("[%hs] BUFFER INITIALIZED"), _url.c_str());
            return {};
        }
        if (_buffer.value() == buffer) {
            LOG(TRACE, TM("[%hs] RECEIVED DATA IS EQUAL TO INTERNAL BUFFER DATA | SKIP"), _url.c_str());
            return {};
        }
        LOG(INFO, TM("[%hs] RECEIVED BUFFED IS DIFFERENT TO INTERNAL BUFFER DATA"), _url.c_str());
        char received[buffer.size() + 1];
        char internal[_buffer->size() + 1];
        std::copy(buffer.begin(), buffer.end(), received);
        received[buffer.size() + 1] = '\0';
        std::copy(_buffer->begin(), _buffer->end(), internal);
        internal[_buffer->size() + 1] = '\0';
        LOG(INFO, TM("[%hs] RECEIVED BUFFER: \n\n\n%hs\n\n\n INTERNAL BUFFER: %hs"), _url.c_str(), received, internal);
        return 0;
    }

public:

    explicit PerformanceEndpointListener(string url, const function<void (double)>& matchHandler) 
        : EndpointListener(std::move(url), matchHandler), _buffer() { }
};
