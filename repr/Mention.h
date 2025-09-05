#pragma once

#include "User.h"
#include "Snowflake.h"
#include "REST/REST.h"
#include "client/conversion.h"

#include <nlohmann/json.hpp>

namespace jasper {
	using json = nlohmann::json;

	class Mention {
	public:
		bool role;
		Snowflake id;

		Mention(REST* rest, Snowflake id, bool role) : rest(rest), id(id), role(role) {}
		~Mention() {}

		std::string str() const {
			return role ? "<@&" + std::to_string(id) + ">" : "<@" + std::to_string(id) + ">";
		}

		std::shared_ptr<User> fetch() {
			if (role)
				return nullptr;

			json resp = json::parse(rest->GET(std::format("/users/{}/profile", (uint64_t)id).c_str()));
			return convert::user(rest, resp["user"]);
		}

	private:
		REST* rest;
	};
}
