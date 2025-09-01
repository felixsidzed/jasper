#pragma once

#include "User.h"
#include "List.h"
#include "Snowflake.h"

#include <memory>

namespace jasper {
	struct Message {
		char* content = nullptr;
		std::unique_ptr<User> author;
		List<Snowflake> attachments; // TODO: 'Attachment' struct
		Snowflake a;

		Message(const char* content) {
			this->content = content ? _strdup(content) : nullptr;
		}
		~Message() {
			if (content)
				free(content);
		};
	};
}
