#pragma once

#include <bits/stdc++.h>

#include "../../Abstract/EndpointListener.hpp"
#include "../../Utility/HttpSession.hpp"
#include "../../Utility/Stopwatch.hpp"
#include "../../Utility/Logging.hpp"

using namespace Logging;
using namespace Logging::IP7_Targets;

class CpiPdfEndpointListener : public EndpointListener {

private:

    optional<vector<char>> _buffer;

    //  at this moment PDF parsing not implemented
    optional<double> ReceiveHandler(const std::vector<char> &buffer) override {
        LOG(TRACE, TM("[CPI/PDF] CONTENT RECEIVED"));
        if (!_buffer.has_value()) {
            _buffer = buffer;
            LOG(INFO, TM("[CPI/PDF] BUFFER INITIALIZED"));
            return {};
        }
        if (_buffer.value() == buffer) {
            LOG(TRACE, TM("[CPI/PDF] RECEIVED DATA IS EQUAL TO INTERNAL BUFFER DATA | SKIP"));
            return {};
        }
        LOG(INFO, TM("[CPI/PDF] RECEIVED BUFFED IS DIFFERENT TO INTERNAL BUFFER DATA"));
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

    explicit CpiPdfEndpointListener(string url, const function<void (double)>& matchHandler) 
        : EndpointListener(std::move(url), matchHandler), _buffer() { }

};
