#include "client.h"

#include <iostream>
#include <winsock2.h>

#include "conversion.h"
#include "gateway/opcode.h"
#include "gateway/version.h"

#include <nlohmann/json.hpp>

namespace jasper {
	using json = nlohmann::json;

	void Client::login(const char* token) {
		if (!token)
			return;
		auth = token;
		rest = std::make_unique<REST>(this);

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
						{"token", auth},
						{"intents", intents},
						{"properties", {
							{"os", "linux"},
							{"browser", "discord"},
							{"device", "discord"}
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

			if (ev == "READY") {
				user = convert::user(rest.get(), data["user"]);
				for (const auto& guild : data["guilds"]) {
					Guild* g = convert::guild(rest.get(), guild);
					std::thread([g]() {
						g->channels(true, true);
					}).detach();
				}

				onReady(this, user.get());
			} else if (ev == "MESSAGE_CREATE") {
				auto msg = convert::message(rest.get(), data);
				onMessage(this, std::move(msg));
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
