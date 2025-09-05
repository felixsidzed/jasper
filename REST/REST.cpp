#include "REST.h"

#include "version.h"
#include "client/client.h"

#include <curl/curl.h>
#include <curl/easy.h>

namespace jasper {
    static size_t writecb(void* contents, size_t size, size_t nmemb, void* userp) {
        size_t total = size * nmemb;
        ((std::string*)userp)->append((char*)contents, total);
        return total;
    }

    std::string REST::request(const char* method, const char* endpoint, const std::string& payload) {
        std::string response;

        CURL* curl = curl_easy_init();
        if (!curl)
            return response;

        const std::string& url = std::string(JASPER_REST_URL) + endpoint;

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Authorization: " + client->auth);
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writecb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        // TODO
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

        if (method == "POST") {
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
        } else if (method == "DELETE")
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");

        curl_easy_perform(curl);
        return response;
    }
}
