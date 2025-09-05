#pragma once

#include <string>

#include "version.h"

namespace jasper {
    class Client;

    class REST {
	public:
        Client* client;

		REST(Client* client) : client(client) {};

        #undef DELETE
        std::string GET(const char* endpoint) { return request("GET", endpoint); }
        std::string POST(const char* endpoint, const std::string& payload) { return request("POST", endpoint, payload); }
        std::string DELETE(const char* endpoint) { return request("DELETE", endpoint); }
    private:
        std::string request(const char* method, const char* endpoint, const std::string& payload = "");
	};
}
