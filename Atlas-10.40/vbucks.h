#pragma once
// Include curl before Windows headers to avoid Winsock conflicts
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif
#include <curl/curl.h>
#include <string>
#include "Engine/Plugins/Atlas/Public/Atlas.h"
#include <thread>

namespace vbucks
{
    inline size_t WriteCallbackRequests(void* ptr, size_t size, size_t nmemb, std::string* data) {
        data->append((char*)ptr, size * nmemb);
        return size * nmemb;
    }

    inline size_t returnData(void* contents, size_t size, size_t nmemb, void* userp)
    {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

    inline void BusFare(const std::string& Username)
    {
        std::string url = Atlas::BackendIP + "/busFare/" + Username + "/" + Atlas::BackendAPIKey;
        std::thread([url = url]()
            {
                curl_global_init(CURL_GLOBAL_ALL);
                CURL* curl = curl_easy_init();
                if (!curl) {
                    curl_global_cleanup();
                    return;
                }

                curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallbackRequests);

                std::string response_body;
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);

                curl_easy_perform(curl);
                curl_easy_cleanup(curl);

                curl_global_cleanup();
            }
        ).detach();
    }

    inline void AddHype(const std::string& Username, const int& Amount)
    {
        // Only send hype if arena mode is enabled
        if (!Atlas::bArena) {
            return;
        }
        
        std::string url = Atlas::BackendIP + "/addHype/" + Username + "/" + std::to_string(Amount) + "/" + Atlas::BackendAPIKey;
        std::thread([url = url]()
            {
                curl_global_init(CURL_GLOBAL_ALL);
                CURL* curl = curl_easy_init();
                if (!curl) {
                    curl_global_cleanup();
                    return;
                }

                curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallbackRequests);

                std::string response_body;
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);

                curl_easy_perform(curl);
                curl_easy_cleanup(curl);

                curl_global_cleanup();
            }
        ).detach();
    }

    inline void GiveXP(const std::string& username, const int& xpAmount)
    {
        std::string url = Atlas::BackendIP +"api/v1/"+"/xp/" + username + "/" + std::to_string(xpAmount) + "/" + Atlas::BackendAPIKey;
        std::thread([url = url]()
            {
                curl_global_init(CURL_GLOBAL_ALL);
                CURL* curl = curl_easy_init();
                if (!curl) {
                    curl_global_cleanup();
                    return;
                }

                curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallbackRequests);

                std::string response_body;
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);

                curl_easy_perform(curl);
                curl_easy_cleanup(curl);

                curl_global_cleanup();
            }
        ).detach();
    }

    inline void GiveVBucks(const std::string& username, const int& vbucksAmount)
    {
        std::string url = Atlas::BackendIP + "/api/v1/rewards/" + "/Vbucks/" + username + "/" + std::to_string(vbucksAmount) + "/" + Atlas::BackendAPIKey;
        std::thread([url = url]()
            {
                curl_global_init(CURL_GLOBAL_ALL);
                CURL* curl = curl_easy_init();
                if (!curl) {
                    curl_global_cleanup();
                    return;
                }

                curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallbackRequests);

                std::string response_body;
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);

                curl_easy_perform(curl);
                curl_easy_cleanup(curl);

                curl_global_cleanup();
            }
        ).detach();
    }

    inline void GiveSeasonUmbrella(const std::string& username)
    {
        std::string url = Atlas::BackendIP + "/api/v1/season/" + "/Umbrella/" + username + "/" + Atlas::BackendAPIKey;
        std::thread([url = url]()
            {
                curl_global_init(CURL_GLOBAL_ALL);
                CURL* curl = curl_easy_init();
                if (!curl) {
                    curl_global_cleanup();
                    return;
                }

                curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallbackRequests);

                std::string response_body;
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);

                curl_easy_perform(curl);
                curl_easy_cleanup(curl);

                curl_global_cleanup();
            }
        ).detach();
    }

    inline void GiveUmbrella(const std::string& username)
    {
        std::string url = Atlas::BackendIP + "/api/v1/season/" + "/Umbrella/" + username + "/" + Atlas::BackendAPIKey;
        std::thread([url = url]()
            {
                curl_global_init(CURL_GLOBAL_ALL);
                CURL* curl = curl_easy_init();
                if (!curl) {
                    curl_global_cleanup();
                    return;
                }

                curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallbackRequests);

                std::string response_body;
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);

                curl_easy_perform(curl);
                curl_easy_cleanup(curl);

                curl_global_cleanup();
            }
        ).detach();
    }

    inline void Get(const std::string& url)
    {
        std::thread([url = url]()
            {
                curl_global_init(CURL_GLOBAL_ALL);
                CURL* curl = curl_easy_init();
                if (!curl) {
                    curl_global_cleanup();
                    return;
                }

                curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallbackRequests);

                std::string response_body;
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);

                curl_easy_perform(curl);
                curl_easy_cleanup(curl);

                curl_global_cleanup();
            }
        ).detach();
    }

    inline std::string GetTeamMember(const std::string& username)
    {
        std::string url = Atlas::BackendIP + "/getTeamMember/" + username;
        CURL* curl;
        CURLcode res;
        std::string response_txt;

        curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, returnData);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_txt);
            res = curl_easy_perform(curl);

            if (res != CURLE_OK) {
                fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            }

            curl_easy_cleanup(curl);
        }

        return response_txt;
    }
}
