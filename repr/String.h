#pragma once

#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <utility>

namespace jasper {
	class String {
	public:
		char* data;
    
        String() : data(new char[1]) {
            data[0] = '\0';
        }

        String(const char* s) {
            if (!s) {
                data = new char[1];
                data[0] = '\0';
            } else {
                size_t len = std::strlen(s);
                data = new char[len + 1];
                std::strcpy(data, s);
            }
        }

        String(const String& other) {
            size_t len = std::strlen(other.data);
            data = new char[len + 1];
            std::strcpy(data, other.data);
        }

        String(String&& other) noexcept : data(other.data) {
            other.data = nullptr;
        }

        String& operator=(const String& other) {
            if (this != &other) {
                delete[] data;
                size_t len = std::strlen(other.data);
                data = new char[len + 1];
                std::strcpy(data, other.data);
            }
            return *this;
        }

        String& operator=(String&& other) noexcept {
            if (this != &other) {
                delete[] data;
                data = other.data;
                other.data = nullptr;
            }
            return *this;
        }

        // Destructor
        ~String() {
            delete[] data;
        }

        operator const char*() const {
            return data;
        }

        size_t size() const {
            return std::strlen(data);
        }

        char& operator[](size_t index) {
            if (index >= size()) throw std::out_of_range("Index out of range");
            return data[index];
        }

        const char& operator[](size_t index) const {
            if (index >= size()) throw std::out_of_range("Index out of range");
            return data[index];
        }

        String operator+(const String& rhs) const {
            size_t len1 = size();
            size_t len2 = rhs.size();
            char* newData = new char[len1 + len2 + 1];
            std::strcpy(newData, data);
            std::strcat(newData, rhs.data);

            String result;
            delete[] result.data;
            result.data = newData;
            return result;
        }

        String& operator+=(const String& rhs) {
            size_t len1 = size();
            size_t len2 = rhs.size();
            char* newData = new char[len1 + len2 + 1];
            std::strcpy(newData, data);
            std::strcat(newData, rhs.data);
            delete[] data;
            data = newData;
            return *this;
        }

        friend String operator+(const char* lhs, const String& rhs) {
            if (!lhs) lhs = "";
            size_t len1 = std::strlen(lhs);
            size_t len2 = rhs.size();
            char* newData = new char[len1 + len2 + 1];
            std::strcpy(newData, lhs);
            std::strcat(newData, (const char*)rhs);

            String result;
            delete[] result.data;
            result.data = newData;
            return result;
        }
	};
}
