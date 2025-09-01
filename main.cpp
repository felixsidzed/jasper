#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include <fstream>
#include <iostream>

#include "client/client.h"

int main() {
	jasper::Client client(jasper::INTENTS_ALL);

	client.onMessage = [](jasper::Message* msg) {
		printf("%s: %s", msg->author->name, msg->content);
		uint32_t nattachments = msg->attachments.size();
		if (nattachments > 0)
			printf(" [%u attachments]", nattachments);
		putchar('\n');
	};

	std::ifstream stream("TOKEN");
	std::string token;
	stream >> token;
	client.login(token.c_str());
}
