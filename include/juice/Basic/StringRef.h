// include/juice/Basic/StringRef.h - StringRef class, represents a constant reference to a string
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_STRINGREF_H
#define JUICE_STRINGREF_H

#include <cstddef>
#include <cstring>
#include <functional>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace juice {
    namespace basic {
        class StringRef {
            const char * _data = nullptr;
            size_t _length = 0;

            static int compareMemory(const char * lhs, const char * rhs, size_t length);
            static int compareMemoryLower(const char * lhs, const char * rhs, size_t length);

        public:
            static const size_t npos = ~size_t(0);


            StringRef() = default;
            StringRef(std::nullptr_t) = delete;

            StringRef(const char * string): _data(string), _length(string ? strlen(string) : 0) {}
            StringRef(const std::string & string): _data(string.data()), _length(string.length()) {}

            constexpr StringRef(const char * data, size_t length): _data(data), _length(length) {}

            static StringRef withNullAsEmpty(const char * data) { return {data ? data : ""}; }

            const char * begin() const { return _data; }
            const char * end() const { return _data + size(); }

            size_t size() const { return _length; }

            char first() const { return isEmpty() ? 0 : *begin(); }
            char last() const { return isEmpty() ? 0 : _data[size() - 1]; }

            std::string str() const { return begin() ? std::string(begin(), size()) : std::string(); }

            bool isEmpty() const { return size() == 0; }
            bool isNotEmpty() const { return size() > 0; }

            bool equals(StringRef rhs) const;
            bool equalsLower(StringRef rhs) const;

            int compare(StringRef rhs) const;
            int compareLower(StringRef rhs) const;

            char operator[](size_t index) const {
                assert(index < size() && "Invalid index!");
                return begin()[index];
            }

            bool operator==(StringRef rhs) const { return equals(rhs); }
            bool operator!=(StringRef rhs) const { return !equals(rhs); }

            /// Disallow accidental assignment from a temporary std::string.
            ///
            /// The declaration here is extra complicated so that `stringRef = {}`
            /// and `stringRef = "abc"` continue to select the move assignment operator.
            template <typename T>
            typename std::enable_if<std::is_same<T, std::string>::value, StringRef>::type &
            operator=(T &&) = delete;
        };

        std::ostream & operator<<(std::ostream & os, const StringRef & string);
    }
}

#endif //JUICE_STRINGREF_H
