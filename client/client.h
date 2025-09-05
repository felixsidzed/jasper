#pragma once

#include <atomic>

#include "Cache.h"
#include "REST/REST.h"
#include "repr/Guild.h"
#include "repr/Channel.h"
#include "repr/Message.h"
#include "gateway/intents.h"

#include "ixwebsocket/IXWebSocket.h"

namespace jasper {
	typedef void(__fastcall* onReady_t)(Client* self, User* user);
	typedef void(__fastcall* onMessage_t)(Client* self, std::shared_ptr<Message> message);

	class Client {
	public:
		Client(Intents intents) : intents(intents) {};
		
		onReady_t onReady = nullptr;
		onMessage_t onMessage = nullptr;

		Cache<Guild> guildCache;
		Cache<Channel> channelCache;

		std::shared_ptr<User> user = nullptr;

		void login(const char* token);
	private:
		friend class REST;

		Intents intents = (Intents)0;
		
		std::unique_ptr<REST> rest = nullptr;
		std::shared_ptr<ix::WebSocket> ws = nullptr;

		String auth;
		struct {
			std::atomic<int> seq{-1};
			uint32_t interval = 0;
			std::thread th;
		} heartbeat;
	};
}
