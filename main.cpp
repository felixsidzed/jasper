#include <thread>
#include <fstream>
#include <iostream>

#include "UI/UI.h"
#include "client/client.h"

#include <curl/curl.h>

int main() {
	curl_global_init(CURL_GLOBAL_DEFAULT);

	jasper::Client client(jasper::INTENTS_ALL);

	client.onReady = [](jasper::Client* self, jasper::User* user) {
		printf("logged in as '%s'\n", user->name);
		std::thread(jasper::UI::init, self).detach();
	};

	client.onMessage = [](jasper::Client* self, std::shared_ptr<jasper::Message> msg) {
		printf("%s: %s\n", msg->author->name, msg->content);
		if (!strcmp(msg->content, "6"))
			msg->reply("7");
	};

	std::ifstream stream("TOKEN");
	std::string token;
	stream >> token;
	client.login(token.c_str());
	(void)getchar();

	curl_global_cleanup();
}
