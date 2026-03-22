#include "CurlClient.h"
#include <string>

CurlClient::CurlClient() : curl_(nullptr), initialized_(false) {
}

CurlClient::~CurlClient() {
    cleanup();
}

bool CurlClient::init() {
    if (initialized_) return true;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl_ = curl_easy_init();
    if (curl_) {
        initialized_ = true;
        return true;
    }
    return false;
}

void CurlClient::cleanup() {
    if (curl_) {
        curl_easy_cleanup(curl_);
        curl_ = nullptr;
    }
    if (initialized_) {
        curl_global_cleanup();
        initialized_ = false;
    }
}

size_t CurlClient::writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

bool CurlClient::get(const char* url, std::string& response) {
    if (!curl_) return false;

    curl_easy_setopt(curl_, CURLOPT_URL, url);
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl_);

    if (res != CURLE_OK) {
        return false;
    }

    return true;
}

bool CurlClient::post(const char* url, const char* data, std::string& response) {
    if (!curl_) return false;

    curl_easy_setopt(curl_, CURLOPT_URL, url);
    curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, data);
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl_);

    if (res != CURLE_OK) {
        return false;
    }

    return true;
}
