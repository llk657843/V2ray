#pragma once
#include <curl/curl.h>
#include "string"
class CurlClient {
public:
    CurlClient();
    ~CurlClient();

    bool init();
    void cleanup();

    // Simple GET request
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp);

    bool get(const char* url, std::string& response);
    bool post(const char* url, const char* data, std::string& response);

private:
    CURL* curl_;
    bool initialized_;
};
