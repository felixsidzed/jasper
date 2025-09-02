#pragma once

#include <xutility>
#include <stdexcept>

namespace jasper {
	template<typename T>
	struct List {
	public:
		List(uint32_t capacity = 4) : capacity(capacity), length(0) {
			data = (T*)calloc(capacity, sizeof(T));
		}

		~List() {
			free(data);
		}

		void push(const T& value) {
			if (length >= capacity)
				resize(capacity * 2);
			data[length++] = value;
		}

		T pop() {
			if (length > 0)
				return data[--length];
			else
				throw std::out_of_range("List is empty");
		}

		T& operator[](uint32_t index) {
			if (index >= length)
				throw std::out_of_range("Index out of range");
			return data[index];
		}

		const T& operator[](uint32_t index) const {
			if (index >= length)
				throw std::out_of_range("Index out of range");
			return data[index];
		}

		uint32_t size() const {
			return length;
		}

	private:
		T* data = nullptr;
		
		uint32_t length = 0;
		uint32_t capacity = 0;

		void resize(uint32_t newCapacity) {
			T* newData = (T*)calloc(capacity, sizeof(T));
			if (newData)
				throw std::runtime_error("Failed to reallocate");

			for (uint32_t i = 0; i < length; i++)
				newData[i] = data[i];
			free(data);
			data = newData;
			capacity = newCapacity;
		}
	};
}
