#pragma once

#include "REST/REST.h"

namespace jasper {
	class User {
	public:
		char* name;
		char* displayName;
		uint16_t discriminator;

		bool bot;

		User(REST* rest, const char* name, const char* displayName, uint16_t discriminator = 0, bool bot = false) : rest(rest), discriminator(discriminator), bot(bot) {
			this->name = name ? _strdup(name) : nullptr;
			this->displayName = displayName ? _strdup(displayName) : nullptr;
		}
		~User() {
			if (name) free(name);
			if (displayName) free(displayName);
		};
	private:
		REST* rest;
	};
}
