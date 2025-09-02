#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include <fstream>
#include <iostream>

#include "client/client.h"

#include <curl/curl.h>

int main() {
	curl_global_init(CURL_GLOBAL_DEFAULT);

	jasper::Client client(jasper::INTENTS_ALL);

	client.onReady = [](jasper::User* user) {
		printf("logged in as '%s'\n", user->name);
	};

	client.onMessage = [](std::shared_ptr<jasper::Message> msg) {
		printf("%s: %s", msg->author->name, msg->content);

		uint32_t nattachments = msg->attachments.size();
		if (nattachments > 0)
			printf(" [%u attachments]", nattachments);

		uint32_t nmentions = msg->mentions.size();
		if (nmentions > 0) {
			printf(" [mentions:");
			for (int i = 0; i < nmentions; i++) {
				auto& mention = msg->mentions[i];
				printf(" %s", mention.fetch()->name);
			}
			printf("]");
		}

		putchar('\n');

		if (!strcmp(msg->content, "6"))
			msg->reply("7");
	};

	std::ifstream stream("TOKEN");
	std::string token;
	stream >> token;
	client.login(token.c_str());

	curl_global_cleanup();
}
