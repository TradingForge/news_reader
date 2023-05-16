#pragma once

#include <bits/stdc++.h>

class EndpointListener {
    
protected:
    
    using string = std::string;
    using string_view = std::string_view;
    using thread = std::thread;
    template < typename T >
    using vector = std::vector<T>;
    template < typename T >
    using optional = std::optional<T>;
    template < typename T >
    using unique_ptr = std::unique_ptr<T>;
    template < typename T >
    using atomic = std::atomic<T>;
    template < typename T >
    using function = std::function<T>;
    
    string _url;
    atomic<bool> _killed;
    atomic<bool> _watch;
    unique_ptr<thread> _thread;
    unique_ptr<HttpSession> _httpSession;
    
    virtual optional<double> ReceiveHandler(const vector<char> &buffer) = 0;
    
    explicit EndpointListener(string url, const function<void (double)>& matchHandler) : _url(std::move(url)), _testData(nullptr), _counter(0),
                                                                                         _killed(false), _watch(false), _thread(nullptr), _matchHandler(matchHandler),
                                                                                         _httpSession(std::make_unique<HttpSession>(_url)) 
    {
        _receiveHandler = [this](const vector<char> &buffer) {
            const vector<char> *_buffer = &buffer;
            if (_testData && ++_counter > 8) {
                _buffer = _testData.get();
            }
            if (auto match = ReceiveHandler(*_buffer)) {
                _matchHandler(match.value());
                Kill();
            }
        };    
    }
    
private:

    function<void (double)> _matchHandler;
    function<void (const vector<char>&)> _receiveHandler;
    std::unique_ptr<vector<char>> _testData;
    size_t _counter;
    
public:
    
    void Watch(int delay, std::unique_ptr<vector<char>>&& testData = nullptr) {
        _testData = std::move(testData);
        _watch = true;
        _thread = std::make_unique<thread>([this, delay]() {
            while (!_killed) {
                auto stopwatch = Stopwatch::Start();
                _httpSession->SendRequest(_receiveHandler);
                stopwatch.Stop();
                auto elapsed = stopwatch.Elapsed() / 1000;
                if (elapsed < delay) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(delay - elapsed));
                }
            }
        });
    }
    
    virtual void Kill() {
        assert(_watch && !_killed);
        _killed = true;
        _watch = false;
    }
    
    virtual ~EndpointListener() = default;
    
};
