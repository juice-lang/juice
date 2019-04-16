// src/juice/Basic/StringRef.cpp - StringRef class, represents a constant reference to a string
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/Basic/StringRef.h"

#include <algorithm>
#include <bitset>
#include <climits>

#include "juice/Basic/StringHelpers.h"

namespace juice {
    namespace basic {
        inline int StringRef::compareMemory(const char * lhs, const char * rhs, size_t length) {
            if (length == 0) return 0;
            return memcmp(lhs, rhs, length);
        }

        inline int StringRef::compareMemoryLower(const char * lhs, const char * rhs, size_t length) {
            for (size_t i = 0; i < length; ++i) {
                unsigned char lhc = toLower(lhs[i]);
                unsigned char rhc = toLower(rhs[i]);
                if (lhc != rhc) return lhc < rhc ? -1 : 1;
            }
            return 0;
        }

        inline bool StringRef::equals(StringRef rhs) const {
            return size() == rhs.size() && compareMemory(begin(), rhs.begin(), size()) == 0;
        }

        inline bool StringRef::equalsLower(StringRef rhs) const {
            return size() == rhs.size() && compareLower(rhs) == 0;
        }

        inline int StringRef::compare(StringRef rhs) const {
            if (int result = compareMemory(begin(), rhs.begin(), std::min(size(), rhs.size()))) {
                return result < 0 ? -1 : 1;
            }

            if (size() == rhs.size()) return 0;

            return size() < rhs.size() ? -1 : 1;
        }

        inline int StringRef::compareLower(StringRef rhs) const {
            if (int result = compareMemoryLower(begin(), rhs.begin(), std::min(size(), rhs.size()))) {
                return result < 0 ? -1 : 1;
            }

            if (size() == rhs.size()) return 0;

            return size() < rhs.size() ? -1 : 1;
        }

        inline bool StringRef::startsWith(StringRef prefix) const {
            return size() >= prefix.size() && compareMemory(begin(), prefix.begin(), prefix.size()) == 0;
        }

        inline bool StringRef::startsWithLower(StringRef prefix) const {
            return size() >= prefix.size() && compareMemoryLower(begin(), prefix.begin(), prefix.size()) == 0;
        }

        inline bool StringRef::endsWith(StringRef suffix) const {
            return size() >= suffix.size() &&
                   compareMemory(end() - suffix.size(), suffix.begin(), suffix.size()) == 0;
        }

        inline bool StringRef::endsWithLower(StringRef suffix) const {
            return size() >= suffix.size() &&
                   compareMemoryLower(end() - suffix.size(), suffix.begin(), suffix.size()) == 0;
        }

        size_t StringRef::indexOf(char c, size_t from) const {
            if (from < size()) {
                if (const void * p = memchr(begin() + from, c, size() - from)) {
                    return static_cast<const char *>(p) - begin();
                }
            }
            return npos;
        }

        size_t StringRef::indexOfNot(char c, size_t from) const {
            for (size_t i = std::min(from, size()); i != size(); ++i) {
                if (begin()[i] != c) return i;
            }
            return npos;
        }

        size_t StringRef::indexOfLower(char c, size_t from) const {
            char l = toLower(c);
            return indexWhere([l](char d) { return toLower(d) == l; }, from);
        }

        size_t StringRef::indexWhere(const std::function<bool(char)> & predicate, size_t from) const {
            StringRef s = dropFirst(from);
            while (s.isNotEmpty()) {
                if (predicate(s.first())) return size() - s.size();
                s = s.dropFirst();
            }
            return npos;
        }

        size_t StringRef::indexWhereNot(const std::function<bool(char)> & predicate, size_t from) const {
            return indexWhere([predicate](char c) { return !predicate(c); }, from);
        }

        size_t StringRef::indexOf(StringRef string, size_t from) const {
            if (from > size()) return npos;

            const char * start = begin() + from;
            size_t length = size() - from;

            const char * needle = string.begin();
            size_t n = string.size();
            if (n == 0) return from;
            if (length < n) return npos;
            if (n == 1) {
                const char * pointer = (const char *) memchr(start, *needle, length);
                return pointer == nullptr ? npos : pointer - begin();
            }

            const char * stop = start + (length - n + 1);

            do {
                if (memcmp(start, needle, n) == 0) return start - begin();
                ++start;
            } while (start < stop);

            return npos;
        }

        size_t StringRef::indexOfLower(StringRef string, size_t from) const {
            StringRef subString = substr(from);
            while (subString.size() >= string.size()) {
                if (subString.startsWithLower(string)) return from;
                subString = subString.dropFirst();
                ++from;
            }
            return npos;
        }

        size_t StringRef::lastIndexOf(char c, size_t from) const {
            from = std::min(from, size());
            size_t i = from;
            while (i != 0) {
                --i;
                if (begin()[i] == c) return i;
            }
            return npos;
        }

        size_t StringRef::lastIndexOfNot(char c, size_t from) const {
            for (size_t i = std::min(from, size()) - 1; i != -1; --i) {
                if (_data[i] != c) return i;
            }
            return npos;
        }

        size_t StringRef::lastIndexOfLower(char c, size_t from) const {
            from = std::min(from, size());
            size_t i = from;
            while (i != 0) {
                --i;
                if (toLower(begin()[i]) == toLower(c)) return i;
            }
            return npos;
        }

        size_t StringRef::lastIndexOf(StringRef string) const {
            size_t n = string.size();
            if (n > size()) return npos;
            for (size_t i = size() - n + 1; i != 0;) {
                --i;
                if (substr(i, n) == string) return i;
            }
            return npos;
        }

        size_t StringRef::lastIndexOfLower(StringRef string) const {
            size_t n = string.size();
            if (n > size()) return npos;
            for (size_t i = size() - n + 1; i != 0;) {
                --i;
                if (substr(i, n).equalsLower(string)) return i;
            }
            return npos;
        }

        size_t StringRef::indexOfContained(StringRef chars, size_t from) const {
            std::bitset<1 << CHAR_BIT> charBits;
            for (char i : chars) {
                charBits.set((unsigned char) i);
            }

            for (size_t i = std::min(from, size()); i != size(); ++i) {
                if (charBits.test((unsigned char) _data[i])) return i;
            }
            return npos;
        }

        size_t StringRef::indexOfNotContained(StringRef chars, size_t from) const {
            std::bitset<1 << CHAR_BIT> charBits;
            for (char i : chars) {
                charBits.set((unsigned char) i);
            }

            for (size_t i = std::min(from, size()); i != size(); ++i) {
                if (!charBits.test((unsigned char) _data[i])) return i;
            }
            return npos;
        }

        size_t StringRef::lastIndexOfContained(StringRef chars, size_t from) const {
            std::bitset<1 << CHAR_BIT> charBits;
            for (char i : chars) {
                charBits.set((unsigned char) i);
            }

            for (size_t i = std::min(from, size()) - 1; i != -1; --i) {
                if (charBits.test((unsigned char) _data[i])) return i;
            }
            return npos;
        }

        size_t StringRef::lastIndexOfNotContained(StringRef chars, size_t from) const {
            std::bitset<1 << CHAR_BIT> charBits;
            for (char i : chars) {
                charBits.set((unsigned char) i);
            }

            for (size_t i = std::min(from, size()) - 1; i != -1; --i) {
                if (!charBits.test((unsigned char) _data[i])) return i;
            }
            return npos;
        }

        size_t StringRef::count(char c) const {
            size_t count = 0;
            for (size_t i = 0; i < size(); ++i) {
                if (begin()[i] == c) ++count;
            }
            return count;
        }

        size_t StringRef::count(StringRef string) const {
            size_t count = 0;
            size_t n = string.size();
            if (n > size()) return 0;
            for (size_t i = 0; i != size() - n + 1; ++i)
                if (substr(i, n).equals(string)) {
                    ++count;
                }
            return count;
        }

        std::string StringRef::lower() const {
            return std::string();
        }

        std::string StringRef::upper() const {
            return std::string();
        }

        StringRef StringRef::substr(size_t start, size_t n) const {
            start = std::min(start, size());
            return {begin() + start, std::min(n, size() - start)};
        }

        StringRef StringRef::prefix(size_t n) const {
            if (n >= size()) return *this;
            return dropLast(size() - n);
        }

        StringRef StringRef::suffix(size_t n) const {
            if (n >= size()) return *this;
            return dropFirst(size() - n);
        }

        StringRef StringRef::prefixWhile(const std::function<bool(char)> & predicate) const {
            return substr(0, indexWhereNot(predicate));
        }

        StringRef StringRef::prefixUntil(const std::function<bool(char)> & predicate) const {
            return substr(0, indexWhere(predicate));
        }

        StringRef StringRef::dropFirst(size_t n) const {
            assert(size() >= n && "Dropping more elements than exist");
            return substr(n);
        }

        StringRef StringRef::dropLast(size_t n) const {
            assert(size() >= n && "Dropping more elements than exist");
            return substr(0, size() - n);
        }

        StringRef StringRef::dropWhile(const std::function<bool(char)> & predicate) const {
            return substr(indexWhereNot(predicate));
        }

        StringRef StringRef::dropUntil(const std::function<bool(char)> & predicate) const {
            return substr(indexWhere(predicate));
        }

        bool StringRef::consumeFirst(StringRef prefix) {
            if (!startsWith(prefix)) return false;

            *this = dropFirst(prefix.size());
            return true;
        }

        bool StringRef::consumeLast(StringRef suffix) {
            if (!endsWith(suffix)) return false;

            *this = dropLast(suffix.size());
            return true;
        }

        inline StringRef StringRef::slice(size_t start, size_t end) const {
            start = std::min(start, size());
            end = std::min(std::max(start, end), size());
            return {begin() + start, end - start};
        }

        std::pair<StringRef, StringRef> StringRef::split(char separator) const {
            return split(StringRef(&separator, 1));
        }

        std::pair<StringRef, StringRef> StringRef::split(StringRef separator) const {
            size_t index = indexOf(separator);
            if (index == npos) return std::make_pair(*this, StringRef());
            return std::make_pair(slice(0, index), slice(index + separator.size(), npos));
        }

        std::pair<StringRef, StringRef> StringRef::splitLast(char separator) const {
            return splitLast(StringRef(&separator, 1));
        }

        std::pair<StringRef, StringRef> StringRef::splitLast(StringRef separator) const {
            size_t index = lastIndexOf(separator);
            if (index == npos) return std::make_pair(*this, StringRef());
            return std::make_pair(slice(0, index), slice(index + separator.size(), npos));
        }

        void StringRef::splitInto(std::vector<StringRef> & vector, char separator, int maxSplit, bool keepEmpty) const {
            StringRef string = *this;

            while (maxSplit-- != 0) {
                size_t index = string.indexOf(separator);
                if (index == npos) break;

                if (keepEmpty || index > 0) vector.push_back(string.slice(0, index));

                string = string.slice(index + 1, npos);
            }

            if (keepEmpty || string.isNotEmpty()) vector.push_back(string);
        }

        void
        StringRef::splitInto(std::vector<StringRef> & vector, StringRef separator, int maxSplit, bool keepEmpty) const {
            StringRef string = *this;

            while (maxSplit-- != 0) {
                size_t index = string.indexOf(separator);
                if (index == npos) break;

                if (keepEmpty || index > 0) vector.push_back(string.slice(0, index));

                string = string.slice(index + separator.size(), npos);
            }

            if (keepEmpty || string.isNotEmpty()) vector.push_back(string);
        }

        StringRef StringRef::trimFirst(char c) const {
            return dropFirst(std::min(size(), indexOfNot(c)));
        }

        StringRef StringRef::trimFirst(StringRef chars) const {
            return dropFirst(std::min(size(), indexOfNotContained(chars)));
        }

        StringRef StringRef::trimLast(char c) const {
            return dropLast(size() - std::min(size(), lastIndexOfNot(c) + 1));
        }

        StringRef StringRef::trimLast(StringRef chars) const {
            return dropLast(size() - std::min(size(), lastIndexOfNotContained(chars) + 1));
        }

        StringRef StringRef::trim(char c) const {
            return trimFirst(c).trimLast(c);
        }

        StringRef StringRef::trim(StringRef chars) const {
            return trimFirst(chars).trimLast(chars);
        }

        std::ostream & operator<<(std::ostream & os, const StringRef & string) {
            return os.write(string.begin(), string.size());
        }
    }
}
