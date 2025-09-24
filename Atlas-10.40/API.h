#pragma once

// Include curl before Windows headers to avoid Winsock conflicts
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif
#include <curl/curl.h>

namespace api
{
    inline size_t write_callback(void* ptr, size_t size, size_t nmemb, std::string* data) {
        data->append((char*)ptr, size * nmemb);
        return size * nmemb;
    }

    inline void getResponseAsync(const std::string& url) {
        std::thread([url = url]() {
            curl_global_init(CURL_GLOBAL_ALL);
            CURL* curl = curl_easy_init();
            if (!curl) {
                curl_global_cleanup();
                return;
            }

            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

            std::string response_body;
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);

            curl_easy_perform(curl);
            curl_easy_cleanup(curl);

            curl_global_cleanup();
            }).detach();
    }

    inline std::string getResponse(const std::string& url)
    {
        curl_global_init(CURL_GLOBAL_ALL);
        CURL* curl = curl_easy_init();
        std::string response_body;

        if (curl)
        {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK)
            {
                response_body = "error";
            }

            curl_easy_cleanup(curl);
        }

        curl_global_cleanup();
        return response_body;
    }
}
