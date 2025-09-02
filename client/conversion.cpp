#include "conversion.h"

#include "repr/User.h"
#include "repr/Message.h"

namespace jasper {
	namespace convert {
		User* user(REST* rest, const json& data) {
			std::string username = data["username"].get<std::string>();
			std::string displayName = data.value("global_name", username);

			auto user_ = new User(
				rest,
				username.c_str(),
				displayName.c_str(),
				(uint16_t)std::stoi(data["discriminator"].get<std::string>()),
				data.value("bot", false)
			);
			
			return user_;
		}

		std::shared_ptr<Message> message(REST* rest, const json& data) {
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

			return msg;
		}
	}
}
