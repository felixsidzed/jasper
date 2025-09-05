#include "conversion.h"

#include "client.h"
#include "repr/User.h"
#include "repr/Guild.h"
#include "repr/Channel.h"
#include "repr/Message.h"

namespace jasper {
	namespace convert {
		std::shared_ptr<User> user(REST* rest, const json& data) {
			std::string username = data["username"].get<std::string>();
			std::string displayName = data["global_name"].is_null() ? username : data["global_name"].get<std::string>();

			return std::make_shared<User>(
				rest,
				username.c_str(), displayName.c_str(),
				(uint16_t)std::stoi(data["discriminator"].get<std::string>()),
				data.value("bot", false)
			);
		}

		Guild* guild(REST* rest, const json& data) {
			Snowflake id = getsnowflake(data["id"]);
			Guild* g = rest->client->guildCache.get(id);
			if (!g) {
				std::string name = data["name"].get<std::string>();
				rest->client->guildCache.emplace(id, rest, name.c_str(), id);
				g = rest->client->guildCache.get(id);
			}
			return g;
		}

		Channel* channel(REST* rest, const json& data) {
			Snowflake id = getsnowflake(data["id"]);
			Channel* chan = rest->client->channelCache.get(id);
			if (!chan) {
				std::string name = data["name"].get<std::string>();

				Snowflake parentId = 0;
				if (data.contains("parent_id") && !data["parent_id"].is_null())
					parentId = getsnowflake(data["parent_id"]);

				rest->client->channelCache.emplace(id,
					rest,
					id, name.c_str(),
					data["type"], getsnowflake(data["guild_id"]),
					data.value("nsfw", false),
					parentId
				);
				chan = rest->client->channelCache.get(id);
			}
			return chan;
		}

		std::shared_ptr<Message> message(REST* rest, Channel* chan, const json& data) {
			std::string content;
			if (data.contains("content"))
				content = data["content"].get<std::string>();

			auto msg = std::make_shared<Message>(
				rest, content.c_str(),
				getsnowflake(data["id"]), getsnowflake(data["channel_id"])
			);

			if (data.contains("attachments"))
				for (const auto& attachment : data["attachments"])
					msg->attachments.push(getsnowflake(attachment["id"]));

			if (data.contains("mentions"))
				for (const auto& mention : data["mentions"]) {
					msg->mentions.push(Mention(rest, getsnowflake(mention["id"]), false));
				}

			if (data.contains("mention_roles"))
				for (const auto& role : data["mention_roles"])
					msg->mentions.push(Mention(rest, getsnowflake(role), true));

			msg->author = user(rest, data["author"]);
			if (chan)
				msg->channel = chan;
			else {
				const json& fetched = json::parse(rest->GET(std::format("/channels/{}", (uint64_t)msg->channelId).c_str()));
				msg->channel = channel(rest, fetched);
			}

			return msg;
		}
	}
}
