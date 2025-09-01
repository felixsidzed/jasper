#pragma once

#include <string>
#include <atomic>

#include "repr/Message.h"
#include "gateway/intents.h"

#include "ixwebsocket/IXWebSocket.h"

namespace jasper {
	typedef void(__fastcall* onMessage_t)(Message* message);

	class Client {
	public:
		Client(Intents intents) : intents(intents) {};

		onMessage_t onMessage = nullptr;

		void login(const std::string& token);
	private:
		Intents intents = (Intents)0;
		std::shared_ptr<ix::WebSocket> ws = nullptr;

		std::unordered_map<std::string, std::weak_ptr<User>> userCache;

		std::string token;
		struct {
			std::atomic<int> seq{-1};
			uint32_t interval;
			std::thread th;
		} heartbeat;
	};
}
