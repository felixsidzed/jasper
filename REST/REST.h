#pragma once

#include <string>

#include "version.h"

namespace jasper {
    class REST {
	public:
		REST(char* token) : auth(token) {};
        ~REST() {
            if (auth)
                free(auth);
        }

		std::string GET(const char* endpoint) { return request("GET", endpoint); }
        std::string POST(const char* endpoint, const std::string& payload) { return request("POST", endpoint, payload); }
        std::string DELETE(const char* endpoint) { return request("DELETE", endpoint); }
    private:
        char* auth;

        std::string request(const char* method, const char* endpoint, const std::string& payload = "");
	};
}
