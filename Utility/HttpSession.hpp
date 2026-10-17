#pragma once

#include <bits/stdc++.h>
#include <curl/curl.h>
#include "Stopwatch.hpp"

class HttpSession final {

private:
    
    using Buffer = std::vector<char>;
    using ReceiveHandler = std::function<void(const Buffer &buffer)>;
    using string = std::string;
    template < typename T >
    using unique_ptr = std::unique_ptr<T>;

    CURL *_curl;
    string _url;
    Buffer _buffer;
    double total;
    int count;

    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
        Buffer &callbackHandler = *static_cast<Buffer*>(userp);
        char *_contents = static_cast<char *>(contents);
        size_t totalSize = size * nmemb;
        callbackHandler.insert(callbackHandler.end(), _contents, _contents + totalSize);
        return totalSize;
    }

public:

    explicit HttpSession(string url) : _url(std::move(url)) {
        total = 0;
        count = 0;
        _curl = curl_easy_init();
        curl_easy_setopt(_curl, CURLOPT_URL, _url.c_str());
        curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(_curl, CURLOPT_WRITEDATA, &_buffer);
    }
    
    [[nodiscard]] string GetUrl() const noexcept {
        return _url;
    }

    CURLcode SendRequest(const ReceiveHandler &receiveHandler) {
        LOG(TRACE, TM("SENDING REQUEST ON URL: %hs"), _url.c_str());
        auto stopwatch = Stopwatch::Start();
        auto result = curl_easy_perform(_curl);
        stopwatch.Stop();
        auto elapsed = stopwatch.Elapsed() / 1000;
        receiveHandler(_buffer);
        total += elapsed;
        ++count;
        LOG(TRACE, TM("REQUEST EXECUTED: %ldms ELAPSED | AVERAGE: %fms"), elapsed, total / count);
        _buffer.clear();
        return result;
    }

    ~HttpSession() {
        curl_easy_cleanup(_curl);
    }

};
