#include "client.h"

#include <iostream>
#include <winsock2.h>
#include <nlohmann/json.hpp>

#include "gateway/opcode.h"
#include "gateway/version.h"

using json = nlohmann::json;

namespace jasper {
	void Client::login(const std::string& token_) {
		token = token_;
		userCache.clear();

		WSADATA wsaData;
		int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (wsaResult != 0)
			return;

		ws = std::make_shared<ix::WebSocket>();
		ws->setUrl("wss://gateway.discord.gg/?v=" JASPER_GATEWAY_VERSION "&encoding=" JASPER_GATEWAY_ENCODING);

		ix::SocketTLSOptions tls;
		ws->setTLSOptions(tls);

		std::atomic<bool> running(true);
		ws->setOnMessageCallback([this, &running](const ix::WebSocketMessagePtr& msg) {
			if (msg->type == ix::WebSocketMessageType::Close)
				running.store(false);
			else if (msg->type == ix::WebSocketMessageType::Error) {
				std::cout << msg->errorInfo.reason << std::endl;
				running.store(false);
			}

			if (msg->type != ix::WebSocketMessageType::Message)
				return;
				
			json payload;
			try {
				payload = json::parse(msg->str);
			} catch (const json::parse_error& err) {
				std::cout << err.what() << std::endl;
				return;
			}

			int op = payload["op"];
			json data = payload["d"];
			int seq = payload["s"].is_null() ? 0 : payload["s"].get<int>();
			const std::string& ev = payload["t"].is_string() ? payload["t"].get<std::string>() : "";

			if (seq != 0)
				heartbeat.seq = seq;

			if (op == OP_HELLO) {
				heartbeat.interval = (uint32_t)data["heartbeat_interval"].get<double>();

				json identify = {
					{"op", OP_IDENTIFY},
					{"d", {
						{"token", token},
						{"intents", intents},
						{"properties", {
							{"os", "windows"},
							{"browser", "jasper"},
							{"device", "jasper"}
						}}
					}}
				};
				ws->send(identify.dump());

				heartbeat.th = std::thread([this, &running]() {
					while (running.load()) {
						std::this_thread::sleep_for(std::chrono::milliseconds(heartbeat.interval));
						json beat = { {"op", OP_HEARTBEAT}, {"d", heartbeat.seq.load()}};
						ws->send(beat.dump());
					}
				});
				heartbeat.th.detach();
			}

			if (ev.empty())
				return;

			if (ev == "MESSAGE_CREATE") {
				std::string content;
				if (data.contains("content"))
					content = data["content"].get<std::string>();

				Message msg(content.c_str());
				if (data.contains("attachments"))
					for (const auto& attachment : data["attachments"])
						msg.attachments.push(std::stoull(attachment.get<std::string>()));

				std::string username = data["author"]["username"].get<std::string>();
				std::string displayName = data["author"].value("global_name", username);
				auto author = std::make_unique<User>(
					username.c_str(),
					displayName.c_str()
				);
				msg.author = std::move(author);

				onMessage(&msg);
			}
		});

		ws->start();
		while (!heartbeat.th.joinable())
			std::this_thread::sleep_for(std::chrono::seconds(1));
		heartbeat.th.join();
		ws->stop();

		WSACleanup();
	}
}
