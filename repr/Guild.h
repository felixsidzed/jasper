#pragma once

#include <string>

#include "Channel.h"
#include "Snowflake.h"
#include "REST/REST.h"
#include "repr/List.h"
#include "client/conversion.h"

namespace jasper {
	class Guild {
	public:
		String name;
		Snowflake id;

		Guild(REST* rest, const char* name, Snowflake id)
			: rest(rest), name(name), id(id) {}

		List<Channel*>& channels(bool nocache = false, bool populateCategories = false) {
			if (nocache || channelCache.size() == 0) {
				channelCache.clear();

				const nlohmann::ordered_json& resp = nlohmann::ordered_json::parse(rest->GET(std::format("/guilds/{}/channels", (uint64_t)id).c_str()));
				for (const auto& chan : resp) 
					channelCache.push(convert::channel(rest, chan));

				if (populateCategories) {
					for (uint32_t i = 0; i < channelCache.size(); i++) {
						Channel* chan = channelCache[i];
						if (chan->type == CHANNEL_GUILD_CATEGORY)
							chan->channels();
					}
				}
			}
			return channelCache;
		}

	private:
		REST* rest;
		List<Channel*> channelCache;
	};
}
