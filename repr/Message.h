#pragma once

#include "User.h"
#include "List.h"
#include "String.h"
#include "Mention.h"
#include "REST/REST.h"
#include "Snowflake.h"

#include <memory>
#include <format>

#include <nlohmann/json.hpp>

namespace jasper {
	using json = nlohmann::json;

	class Message {
	public:
		String content;

		Snowflake id;
		Snowflake channelId;
		
		Channel* channel = nullptr;
		std::shared_ptr<User> author;
		
		List<Mention> mentions;
		List<Snowflake> attachments;

		Message(REST* rest, const char* content, Snowflake id, Snowflake channelId = 0) : rest(rest), content(content), id(id), channelId(channelId) {}

		void reply(const char* content) {
			json payload = {
				{"content", content},
				{"message_reference", {
					{"message_id", id},
					{"channel_id", channelId}
				}}
			};
			rest->POST(std::format("/channels/{}/messages", (uint64_t)channelId).c_str(), payload.dump().c_str());
		}

	private:
		REST* rest;
	};
}
