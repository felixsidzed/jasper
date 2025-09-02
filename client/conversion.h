#pragma once

#include "REST/REST.h"

#include <memory>
#include <nlohmann/json.hpp>

#define getsnowflake(x) (std::stoull((x).get<std::string>()))

namespace jasper {
	using json = nlohmann::json;
	
	class User;
	class Message;

	namespace convert {
		User* user(REST* rest, const json& data);
		std::shared_ptr<Message> message(REST* rest, const json& data);
	}
}
