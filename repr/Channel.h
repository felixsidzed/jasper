#pragma once

#include "List.h"
#include "User.h"
#include "String.h"
#include "Snowflake.h"
#include "REST/REST.h"
#include "client/conversion.h"

#include <memory>
#include <format>

#include <nlohmann/json.hpp>

#define JASPER_CHANNEL_ISTEXT(t) ( \
    (t) == CHANNEL_GUILD_TEXT || \
    (t) == CHANNEL_DM || \
    (t) == CHANNEL_GROUP_DM || \
    (t) == CHANNEL_GUILD_ANNOUNCEMENT || \
    (t) == CHANNEL_ANNOUNCEMENT_THREAD || \
    (t) == CHANNEL_PUBLIC_THREAD || \
    (t) == CHANNEL_PRIVATE_THREAD \
)

namespace jasper {
	using json = nlohmann::json;

	enum ChannelType : uint8_t {
		CHANNEL_GUILD_TEXT,
		CHANNEL_DM,
		CHANNEL_GUILD_VOICE,
		CHANNEL_GROUP_DM,
		CHANNEL_GUILD_CATEGORY,
		CHANNEL_GUILD_ANNOUNCEMENT,
		CHANNEL_ANNOUNCEMENT_THREAD = 10,
		CHANNEL_PUBLIC_THREAD,
		CHANNEL_PRIVATE_THREAD,
		CHANNEL_GUILD_STAGE_VOICE,
		CHANNEL_GUILD_DIRECTORY,
		CHANNEL_GUILD_FORUM,
		CHANNEL_GUILD_MEDIA
	};

	class Channel {
	public:
		String name;

		Snowflake id;
		Snowflake guildId;
		Snowflake parentId;
		
		bool nsfw;
		enum ChannelType type;

		Channel(REST* rest, Snowflake id, const char* name, enum ChannelType type, Snowflake guildId, bool nsfw = false, Snowflake parentId = 0) : rest(rest), id(id), name(name), type(type), guildId(guildId), nsfw(nsfw), parentId(parentId) {}

		void send(const char* content) {
			if (!JASPER_CHANNEL_ISTEXT(type))
				return;

			json payload = { {"content", content} };
			rest->POST(std::format("/channels/{}/messages", (uint64_t)id).c_str(), payload.dump().c_str());
		}

		List<std::shared_ptr<class Message>> messages(uint32_t limit = 50, bool nocache = false) {
			 if (!JASPER_CHANNEL_ISTEXT(type))
				return messageCache;

            if (nocache || messageCache.size() == 0) {
                messageCache.clear();

                const nlohmann::ordered_json& data = nlohmann::ordered_json::parse(rest->GET(std::format("/channels/{}/messages?limit={}", (uint64_t)id, limit).c_str()));
                for (auto& msg : data)
					messageCache.push(std::move(convert::message(rest, this, msg)));
            }
			return messageCache;
		}

		List<Channel*>& channels(bool nocache = false) {
            if (type != CHANNEL_GUILD_CATEGORY)
                throw channelCache;

            if (nocache || channelCache.size() == 0) {
                channelCache.clear();

                const nlohmann::ordered_json& data = nlohmann::ordered_json::parse(rest->GET(std::format("/guilds/{}/channels", (uint64_t)guildId).c_str()));
                for (auto& chan : data) {
                    if (chan.contains("parent_id") && !chan["parent_id"].is_null() && getsnowflake(chan["parent_id"]) == id)
                        channelCache.push(convert::channel(rest, chan));
                }
            }

            return channelCache;
        }

	private:
		REST* rest;
		List<Channel*> channelCache;
		List<std::shared_ptr<class Message>> messageCache;
	};
}
