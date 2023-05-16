#pragma once

#include <bits/stdc++.h>

#include "../../Abstract/EndpointListener.hpp"
#include "../../Utility/HttpSession.hpp"
#include "../../Utility/Stopwatch.hpp"
#include "../../Utility/Logging.hpp"

using namespace Logging;
using namespace Logging::IP7_Targets;

class CpiRssEndpointListener : public EndpointListener {

private:

    static constexpr const char *CPI_PARAGRAPH_ANCHOR = "Consumer Price Index (CPI):";
    static constexpr const char *CPI_INDEX_ANCHOR = "class=\"data\">";
    static constexpr const char *CPI_MONTH_ANCHOR = "in ";

    static const size_t CPI_BEFORE_INDEX_LENGTH = strlen(CPI_INDEX_ANCHOR);
    static const size_t CPI_BEFORE_MONTH_LENGTH = strlen(CPI_MONTH_ANCHOR);

    optional <string> _month;

    optional<double> ReceiveHandler(const vector<char> &buffer) override {
        LOG(TRACE, TM("[CPI/RSS] CONTENT RECEIVED"));
        string_view content(buffer.data(), buffer.size());
        int from, to;
        int paragraphPosition = content.find(CPI_PARAGRAPH_ANCHOR);
        if (paragraphPosition == -1) {
            LOG(ERROR, TM("[CPI/RSS] CPI PARAGRAPH ANCHOR NOT FOUND"));
            return {};
        }
        int indexPosition = content.find(CPI_INDEX_ANCHOR, paragraphPosition);
        if (indexPosition == -1) {
            LOG(ERROR, TM("[CPI/RSS] CPI INDEX ANCHOR NOT FOUND"));
            return {};
        }
        int monthPosition = content.find(CPI_MONTH_ANCHOR, indexPosition);
        if (monthPosition == -1) {
            LOG(ERROR, TM("[CPI/RSS] CPI MONTH ANCHOR NOT FOUND"));
            return {};
        }
        from = monthPosition + CPI_BEFORE_MONTH_LENGTH;
        to = from + 1;
        while (std::isalnum(content[to])) {
            assert(content.size() > to);
            ++to;
        }
        char temp[1024];
        for (int i = from; i < to; ++i) {
            temp[i - from] = content[i];
        }
        temp[to - from] = '\0';
        LOG(DEBUG, TM("[CPI/RSS] MONTH PARSED: '%hs'"), temp);
        if (!_month.has_value()) {
            _month = temp;
            LOG(INFO, TM("[CPI/RSS] MONTH DEFINED: '%hs'"), _month->c_str());
            return {};
        }
        if (_month == temp) {
            LOG(TRACE, TM("[CPI/RSS] MONTH IS EQUAL TO DEFINED: '%hs' | SKIP"), _month->c_str());
            return {};
        }
        from = indexPosition + CPI_BEFORE_INDEX_LENGTH;
        to = from;
        do {
            ++to;
            while (isdigit(content[to])) {
                assert(content.size() > to);
                ++to;
            }
        } while (content[to] == '.');
        for (int i = from; i < to; ++i) {
            temp[i - from] = content[i];
        }
        temp[to - from] = '\0';
        auto result = std::stod(temp);
        LOG(INFO, TM("[CPI/RSS] MATCHED VALUE: '%f'"), result);
        return result;
    }

public:

    explicit CpiRssEndpointListener(string url, const function<void(double)> &matchHandler) 
        : EndpointListener(std::move(url), matchHandler), _month() { }

};
