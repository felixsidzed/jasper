#include "REST.h"

#include "version.h"

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
        headers = curl_slist_append(headers, ("Authorization: " + std::string(auth)).c_str());
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

   /* class REST {
	public:
		REST();
		~REST();

		std::string GET(const char* endpoint) { return request("GET", endpoint); }
        std::string POST(const char* endpoint, const std::string& payload) { return request("POST", endpoint, payload); }
        std::string DELETE(const char* endpoint) { return request("DELETE", endpoint); }
    private:
       char* token;

        std::string request(const char* method, const char* endpoint, const std::string& payload = "")  {
            CURL* curl = curl_easy_init();
            if (!curl) throw std::runtime_error("Failed to init curl");

            std::string response;
            std::string url = baseUrl + endpoint;

            struct curl_slist* headers = nullptr;
            headers = curl_slist_append(headers, ("Authorization: Bot " + botToken).c_str());
            headers = curl_slist_append(headers, "Content-Type: application/json");

            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

            if (method == "POST") {
                curl_easy_setopt(curl, CURLOPT_POST, 1L);
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
            } else if (method == "DELETE") {
                curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
            }

            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                curl_easy_cleanup(curl);
                curl_slist_free_all(headers);
                throw std::runtime_error(std::string("CURL error: ") + curl_easy_strerror(res));
            }

            curl_easy_cleanup(curl);
            curl_slist_free_all(headers);

            return response;
        }
	};*/
}
