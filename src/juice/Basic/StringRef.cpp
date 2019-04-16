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

        std::ostream & operator<<(std::ostream & os, const StringRef & string) {
            return os.write(string.begin(), string.size());
        }
    }
}
