#pragma once

#include <memory>
#include "repr/List.h"
#include "repr/Snowflake.h"

namespace jasper {
	template<typename T>
	class Cache {
	public:
		struct Entry {
			Snowflake id;
			std::unique_ptr<T> value;

			Entry(Snowflake id, std::unique_ptr<T> value)
				: id(id), value(std::move(value)) {}
		};

		Cache(uint32_t capacity = 4) : entries(capacity) {}

		void put(Snowflake id, const T& value) {
			for (uint32_t i = 0; i < entries.size(); i++) {
				if (entries[i].id == id) {
					*entries[i].value = value;
					return;
				}
			}
			entries.push(Entry(id, std::make_unique<T>(value)));
		}

		void put(Snowflake id, T&& value) {
			for (uint32_t i = 0; i < entries.size(); i++) {
				if (entries[i].id == id) {
					*entries[i].value = std::move(value);
					return;
				}
			}
			entries.push(Entry(id, std::make_unique<T>(std::move(value))));
		}

		template<typename... Args>
		void emplace(Snowflake id, Args&&... args) {
			for (uint32_t i = 0; i < entries.size(); i++) {
				if (entries[i].id == id) {
					*entries[i].value = T(std::forward<Args>(args)...);
					return;
				}
			}
			entries.push(Entry(id, std::make_unique<T>(std::forward<Args>(args)...)));
		}

		T* get(Snowflake id) {
			for (uint32_t i = 0; i < entries.size(); i++) {
				if (entries[i].id == id)
					return entries[i].value.get();
			}
			return nullptr;
		}

		const T* get(Snowflake id) const {
			for (uint32_t i = 0; i < entries.size(); i++) {
				if (entries[i].id == id)
					return entries[i].value.get();
			}
			return nullptr;
		}

		bool contains(Snowflake id) const {
			for (uint32_t i = 0; i < entries.size(); i++) {
				if (entries[i].id == id)
					return true;
			}
			return false;
		}

		bool remove(Snowflake id) {
			for (uint32_t i = 0; i < entries.size(); i++) {
				if (entries[i].id == id) {
					entries[i] = std::move(entries[entries.size() - 1]);
					entries.pop();
					return true;
				}
			}
			return false;
		}

		uint32_t size() const { return entries.size(); }
		T* at(uint32_t i) { return entries[i].value.get(); }

	private:
		List<Entry> entries;
	};
}
