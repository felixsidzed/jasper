#pragma once

namespace jasper {
	struct User {
		char* name;
		char* displayName;

		User(const char* name, const char* displayName) {
			this->name = name ? _strdup(name) : nullptr;
			this->displayName = displayName ? _strdup(displayName) : nullptr;
		}
		~User() {
			if (name)
				free(name);
			if (displayName)
				free(displayName);
		};
	};
}
