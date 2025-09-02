#pragma once

#include <atomic>

#include "REST/REST.h"
#include "repr/Message.h"
#include "gateway/intents.h"

#include "ixwebsocket/IXWebSocket.h"

namespace jasper {
	typedef void(__fastcall* onReady_t)(User* user);
	typedef void(__fastcall* onMessage_t)(std::shared_ptr<Message> message);

	class Client {
	public:
		Client(Intents intents) : intents(intents) {};
		~Client() {
			if (auth)
				free(auth);
		}
		
		onReady_t onReady = nullptr;
		onMessage_t onMessage = nullptr;

		void login(const char* token);
	private:
		Intents intents = (Intents)0;
		
		std::unique_ptr<REST> rest = nullptr;
		std::unique_ptr<User> user = nullptr;
		std::shared_ptr<ix::WebSocket> ws = nullptr;

		char* auth = nullptr;
		struct {
			std::atomic<int> seq{-1};
			uint32_t interval;
			std::thread th;
		} heartbeat;
	};
}
