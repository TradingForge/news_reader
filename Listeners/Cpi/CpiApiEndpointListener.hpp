#pragma once

#include <bits/stdc++.h>
#include <cjson/cJSON.h>
#include <fmt/format.h>

#include "../../Abstract/EndpointListener.hpp"
#include "../../Utility/HttpSession.hpp"
#include "../../Utility/Stopwatch.hpp"
#include "../../Utility/Logging.hpp"

using namespace Logging;
using namespace Logging::IP7_Targets;

class CpiApiEndpointListener : public EndpointListener {

private:

    optional<string> _month;

    optional<double> ReceiveHandler(const std::vector<char> &buffer) override {
        LOG(TRACE, TM("[CPI/API] CONTENT RECEIVED"));
        string_view content(buffer.data(), buffer.size());
        auto object = cJSON_Parse(buffer.data());
        if (!object) {
            LOG(ERROR, TM("[CPI/API] CAN'T PARSE JSON FROM DATA"));
            LOG(TRACE, TM("[CPI/API] INVALID JSON DATA: '%hs'"), buffer.data());
            return {};
        }
        auto results = cJSON_GetObjectItemCaseSensitive(object, "Results");
        if (!results) {
            LOG(ERROR, TM("[CPI/API] INVALID JSON FORMAT: 'Results' FIELD NOT FOUND"));
            LOG(TRACE, TM("[CPI/API] INVALID JSON DATA: '%hs'"), buffer.data());
            cJSON_Delete(object);
            return {};
        }
        auto series = cJSON_GetArrayItem(cJSON_GetObjectItemCaseSensitive(results, "series"), 0);
        if (!series) {
            LOG(ERROR, TM("[CPI/API] INVALID JSON FORMAT: 'series' FIELD NOT FOUND"));
            LOG(TRACE, TM("[CPI/API] INVALID JSON DATA: '%hs'"), buffer.data());
            cJSON_Delete(object);
            return {};
        }
        auto data = cJSON_GetArrayItem(cJSON_GetObjectItemCaseSensitive(series, "data"), 0);
        if (!data) {
            LOG(ERROR, TM("[CPI/API] INVALID JSON FORMAT: 'data' FIELD NOT FOUND"));
            LOG(TRACE, TM("[CPI/API] INVALID JSON DATA: '%hs'"), buffer.data());
            cJSON_Delete(object);
            return {};
        }
        char *temp = cJSON_GetObjectItemCaseSensitive(data, "latest")->valuestring;
        if (!temp) {
            LOG(ERROR, TM("[CPI/API] INVALID JSON FORMAT: 'latest' FIELD IS NOT STRING"));
            LOG(TRACE, TM("[CPI/API] INVALID JSON DATA: '%hs'"), buffer.data());
            cJSON_Delete(object);
            return {};
        }
        if (std::strcmp(temp, "true") != 0) {
            LOG(ERROR, TM("[CPI/API] INVALID JSON FORMAT: 'latest' FIELD IS NOT 'true'"));
            LOG(TRACE, TM("[CPI/API] INVALID JSON DATA: '%hs'"), buffer.data());
            cJSON_Delete(object);
            return {};
        }
        temp = cJSON_GetObjectItemCaseSensitive(data, "periodName")->valuestring;
        if (!temp) {
            LOG(ERROR, TM("[CPI/API] INVALID JSON FORMAT: 'periodName' FIELD IS NOT STRING"));
            LOG(TRACE, TM("[CPI/API] INVALID JSON DATA: '%hs'"), buffer.data());
            cJSON_Delete(object);
            return {};
        }
        LOG(DEBUG, TM("[CPI/API] MONTH PARSED: '%hs'"), temp);
        if (!_month.has_value()) {
            _month = temp;
            LOG(INFO, TM("[CPI/API] MONTH DEFINED: '%hs'"), _month->c_str());
            LOG(TRACE, TM("[CPI/API] JSON DATA: '%hs'"), buffer.data());
            cJSON_Delete(object);
            return {};
        }
        if (_month == temp) {
            LOG(TRACE, TM("[CPI/API] MONTH IS EQUAL TO DEFINED: '%hs' | SKIP"), _month->c_str());
            cJSON_Delete(object);
            return {};
        }
        temp = cJSON_GetObjectItemCaseSensitive(data, "value")->valuestring;
        if (!temp) {
            LOG(ERROR, TM("[CPI/API] INVALID JSON FORMAT: 'value' FIELD IS NOT STRING"));
            LOG(TRACE, TM("[CPI/API] INVALID JSON DATA: '%hs'"), buffer.data());
            cJSON_Delete(object);
            return {};
        }
        auto result = std::stod(temp);
        LOG(INFO, TM("[CPI/API] MONTH IS DIFFERENT WITH DEFINED: '%hs' | MATCHED VALUE: %f"), _month->c_str(), result);
        LOG(TRACE, TM("[CPI/API] JSON DATA: '%hs'"), buffer.data());
        cJSON_Delete(object);
        return result;
    }

public:

    explicit CpiApiEndpointListener(string url, const function<void (double)>& matchHandler) 
        : EndpointListener(std::move(url), matchHandler), _month() { }
};
