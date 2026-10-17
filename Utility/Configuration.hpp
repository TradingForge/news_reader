#pragma once

#include <bits/stdc++.h>
#include <cjson/cJSON.h>

class ListenerConfiguration final {

private:

    using string = std::string;
    using ifstream = std::ifstream;

    template < typename T >
    using shared_ptr = std::shared_ptr<T>;
    template < typename T >
    using optional = std::optional<T>;
    template < typename T >
    using vector = std::vector<T>;

    explicit ListenerConfiguration(const cJSON *json) {
        assert(json && cJSON_IsObject(json));
        assert(cJSON_HasObjectItem(json, "identifier")
               && cJSON_HasObjectItem(json, "url")
               && cJSON_HasObjectItem(json, "minDelay")
               && cJSON_HasObjectItem(json, "maxDelay")
               && cJSON_HasObjectItem(json, "count"));
        auto identifier = cJSON_GetObjectItem(json, "identifier");
        auto url = cJSON_GetObjectItem(json, "url");
        auto minDelay = cJSON_GetObjectItem(json, "minDelay");
        auto maxDelay = cJSON_GetObjectItem(json, "maxDelay");
        auto count = cJSON_GetObjectItem(json, "count");
        auto apiKey = cJSON_HasObjectItem(json, "apiKey") ? cJSON_GetObjectItem(json, "apiKey") : nullptr;
        assert(cJSON_IsString(identifier)
               && cJSON_IsString(url)
               && cJSON_IsNumber(minDelay)
               && cJSON_IsNumber(maxDelay)
               && cJSON_IsNumber(count)
               && (!apiKey || cJSON_IsString(apiKey)));
        _identifier = cJSON_GetStringValue(identifier);
        _url = cJSON_GetStringValue(url);
        _minDelay = cJSON_GetNumberValue(minDelay);
        _maxDelay = cJSON_GetNumberValue(maxDelay);
        _count = cJSON_GetNumberValue(count);
        _apiKey = apiKey ? cJSON_GetStringValue(apiKey) : optional<string>();
    }

    string _source;
    string _identifier;
    string _url;
    int _minDelay;
    int _maxDelay;
    int _count;
    optional<string> _apiKey;

public:

    static shared_ptr<ListenerConfiguration> Create(const cJSON* json) {
        return shared_ptr<ListenerConfiguration>(new ListenerConfiguration(json));
    }

    [[nodiscard]] string GetIdentifier() const noexcept {
        return _identifier;
    }

    [[nodiscard]] string GetUrl() const noexcept {
        return _url;
    }

    [[nodiscard]] int GetMinDelay() const noexcept {
        return _minDelay;
    }

    [[nodiscard]] int GetMaxDelay() const noexcept {
        return _maxDelay;
    }

    [[nodiscard]] int GetCount() const noexcept {
        return _count;
    }

    [[nodiscard]] optional<string> GetApiKey() const noexcept {
        return _apiKey;
    }

};

class Configuration final {

private:
    
    using string = std::string;
    using ifstream = std::ifstream;
    
    template < typename T >
    using shared_ptr = std::shared_ptr<T>;
    template < typename T >
    using optional = std::optional<T>;
    template < typename T >
    using vector = std::vector<T>;
    
    Configuration(const cJSON *json) : _listeners()
    {
        assert(json && cJSON_IsObject(json));
        assert(cJSON_HasObjectItem(json, "listeners"));
        auto listeners = cJSON_GetObjectItem(json, "listeners");
        assert(cJSON_IsArray(listeners));
        int count = cJSON_GetArraySize(listeners);
        for (int i = 0; i < count; ++i) {
            _listeners.push_back(ListenerConfiguration::Create(cJSON_GetArrayItem(listeners, i)));
        }
    }

    vector<shared_ptr<ListenerConfiguration>> _listeners;
    
public:
    
    static shared_ptr<Configuration> Parse(const string& filepath) {
        ifstream in(filepath);
        string content;
        while (in) {
            content += in.get();
        }
        return shared_ptr<Configuration>(new Configuration(cJSON_Parse(content.c_str())));
    }
    
    [[nodiscard]] vector<shared_ptr<ListenerConfiguration>> GetListeners() const noexcept {
        return _listeners;
    }
    
};
