#pragma once

#include <bits/stdc++.h>

class SignalHandler final {
    
private:

    template < typename T >
    using atomic = std::atomic<T>;
    
    template < typename T >
    using function = std::function<T>;
    
    template < typename T >
    using shared_ptr = std::shared_ptr<T>;
    
    template < typename T >
    using vector = std::vector<T>;
    
    using handler_t = std::function<void(int)>;
    
    using mutex = std::mutex;
    
    static mutex _lock;
    
    static atomic<vector<handler_t>*> _handlers;
    
    static vector<handler_t> GetHandlers() {
        return *_handlers.load(); 
    }
    
public:
    
    static void AttachHandler(const handler_t& handler) {
        _lock.lock();
        auto handlers = new vector<handler_t>(GetHandlers());
        handlers->push_back(handler);
        atomic_exchange(&_handlers, handlers);
        _handlers = handlers;
        _lock.unlock();
    }
    
    static void HandleSignal(int signal) {
        auto handlers = GetHandlers();
        for (const auto& handler : handlers) {
            handler(signal);
        }
        P7_Exceptional_Flush();
        exit(signal);
    }
    
};

std::mutex SignalHandler::_lock {};
std::atomic<std::vector<std::function<void(int)>>*> SignalHandler::_handlers = std::atomic<std::vector<std::function<void(int)>>*>(new std::vector<std::function<void(int)>>());
