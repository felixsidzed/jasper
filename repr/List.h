#pragma once

#include <new>
#include <utility>
#include <stdexcept>
#include <initializer_list>

#define JASPER_LIST_CHUNK_SIZE 4

namespace jasper {
	template <typename T>
    class List {
    public:
        List(uint32_t capacity = JASPER_LIST_CHUNK_SIZE) : capacity(capacity) {
            data = (T*)::operator new(sizeof(T) * capacity);
        }

        ~List() {
            clear();
            ::operator delete(data);
        }

       void push(const T& value) {
            if (length >= capacity) grow();
            new(&data[length++]) T(value);
        }

        void push(T&& value) {
            if (length >= capacity) grow();
            new(&data[length++]) T(std::move(value));
        }

        void pop() {
            if (length == 0)
                throw std::out_of_range("list is empty");
            data[--length].~T();
        }

        uint32_t size() const {
            return length;
        }

        void clear() {
            for (uint32_t i = 0; i < length; i++)
                data[i].~T();
            length = 0;
        }

        T& operator[](uint32_t index) {
            if (index >= length)
                throw std::out_of_range("index out of range");
            return data[index];
        }

        const T& operator[](uint32_t index) const {
            if (index >= length)
                throw std::out_of_range("index out of range");
            return data[index];
        }

    private:
        T* data = nullptr;

        uint32_t length = 0;
        uint32_t capacity = 0;

        void grow() {
           const uint32_t newCapacity = capacity + JASPER_LIST_CHUNK_SIZE;
            T* newData = (T*)::operator new(sizeof(T) * newCapacity);

            for (uint32_t i = 0; i < length; i++)
                new(&newData[i]) T(std::move(data[i]));

            ::operator delete(data);
            data = newData;
            capacity = newCapacity;
        }
    };
}
