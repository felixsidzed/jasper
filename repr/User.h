#pragma once

#include "String.h"
#include "REST/REST.h"

namespace jasper {
	class User {
	public:
		String name;
		String displayName;
		uint16_t discriminator;

		bool bot;

		User(REST* rest, const char* name, const char* displayName, uint16_t discriminator = 0, bool bot = false) : rest(rest), name(name), displayName(displayName), discriminator(discriminator), bot(bot) {}
	private:
		REST* rest;
	};
}
