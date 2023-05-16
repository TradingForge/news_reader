#pragma once

#include <bits/stdc++.h>

#include "../../Abstract/EndpointListener.hpp"
#include "../../Utility/HttpSession.hpp"
#include "../../Utility/Stopwatch.hpp"
#include "../../Utility/Logging.hpp"

using namespace Logging;
using namespace Logging::IP7_Targets;

class CpiHtmEndpointListener : public EndpointListener {

private:

    static constexpr const char *CPI_TEXT_ANCHOR = "<!-- HTML Format -->";
    static constexpr const char *CPI_MONTH_ANCHOR = "CONSUMER PRICE INDEX - ";
    static constexpr const char *CPI_INDEX_ANCHOR = "(CPI-U)";

    static const size_t CPI_BEFORE_MONTH_LENGTH = strlen(CPI_MONTH_ANCHOR);

    optional<string> _month;

    optional<double> ReceiveHandler(const vector<char> &buffer) override {
        LOG(TRACE, TM("[CPI/HTM] CONTENT RECEIVED"));
        string_view content(buffer.data(), buffer.size());
        int from, to;
        int textPosition = content.find(CPI_TEXT_ANCHOR);
        {
            if (textPosition == -1) {
                LOG(ERROR, TM("[CPI/HTM] CPI TEXT ANCHOR NOT FOUND"));
                return {};
            }
            int monthPosition = content.find(CPI_MONTH_ANCHOR, textPosition);
            if (monthPosition == -1) {
                LOG(ERROR, TM("[CPI/HTM] CPI MONTH ANCHOR NOT FOUND"));
                return {};
            }
            from = monthPosition + CPI_BEFORE_MONTH_LENGTH;
        }
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
        LOG(DEBUG, TM("[CPI/HTM] MONTH PARSED: '%hs'"), temp);
        if (!_month.has_value()) {
            _month = temp;
            LOG(INFO, TM("[CPI/HTM] MONTH DEFINED: '%hs'"), _month->c_str());
            return {};
        }
        if (_month == temp) {
            LOG(TRACE, TM("[CPI/HTM] MONTH IS EQUAL TO DEFINED: '%hs' | SKIP"), _month->c_str());
            return {};
        }
        LOG(INFO, TM("[CPI/HTM] MONTH IS DIFFERENT WITH DEFINED: '%hs'"), _month->c_str());
        {
            int indexPosition = content.find(CPI_INDEX_ANCHOR, textPosition);
            if (indexPosition == -1) {
                LOG(ERROR, TM("[CPI/HTM] CPI INDEX ANCHOR NOT FOUND"));
                return {};
            }
            from = indexPosition;
        }
        while (!isdigit(content[from])) {
            assert(content.size() > from);
            ++from;
        }
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
        LOG(INFO, TM("[CPI/HTM] MATCHED VALUE: '%f'"), result);
        return result;
    }

public:

    explicit CpiHtmEndpointListener(string url, const function<void (double)>& matchHandler) 
        : EndpointListener(std::move(url), matchHandler), _month() { }

};
