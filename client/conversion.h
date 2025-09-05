#pragma once

#include "REST/REST.h"

#include <memory>
#include <nlohmann/json.hpp>

#define getsnowflake(x) (std::stoull((x).get<std::string>()))

namespace jasper {
	using json = nlohmann::json;
	
	class User;
	class Guild;
	class Message;
	class Channel;

	namespace convert {
		Guild* guild(REST* rest, const json& data);
		Channel* channel(REST* rest, const json& data);
		std::shared_ptr<User> user(REST* rest, const json& data);
		std::shared_ptr<Message> message(REST* rest, Channel* chan, const json& data);

		inline std::shared_ptr<Message> message(REST* rest, const json& data) { return message(rest, nullptr, data); };
	}
}
