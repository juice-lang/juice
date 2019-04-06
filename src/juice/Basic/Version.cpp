// src/juice/Basic/Version.cpp - Version class to get the current juice version
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/Basic/Version.h"

#include <sstream>

#define TO_STRING_2(X) #X
#define TO_STRING(X) TO_STRING_2(X)

#ifdef JUICE_VERSION_PATCHLEVEL
/// Helper macro for JUICE_VERSION_STRING.
#define JUICE_MAKE_VERSION_STRING(X, Y, Z) TO_STRING(X) "." TO_STRING(Y) "." TO_STRING(Z)

/// A string that describes the JUICE version number, e.g., "1.0".
#define JUICE_VERSION_STRING \
  JUICE_MAKE_VERSION_STRING(JUICE_VERSION_MAJOR, JUICE_VERSION_MINOR, \
                            JUICE_VERSION_PATCHLEVEL)
#else
/// Helper macro for JUICE_VERSION_STRING.
#define JUICE_MAKE_VERSION_STRING(X, Y) TO_STRING(X) "." TO_STRING(Y)

/// A string that describes the JUICE version number, e.g., "1.0".
#define JUICE_VERSION_STRING \
  JUICE_MAKE_VERSION_STRING(JUICE_VERSION_MAJOR, JUICE_VERSION_MINOR)
#endif

namespace juice {
    namespace basic {
        Version::Version(int major, int minor, int patch):
            major(major), minor(minor), patch(patch) {}

        Version Version::getCurrent() {
            #if JUICE_VERSION_PATCHLEVEL
                return {JUICE_VERSION_MAJOR, JUICE_VERSION_MINOR, JUICE_VERSION_PATCHLEVEL};
            #else
                return {JUICE_VERSION_MAJOR, JUICE_VERSION_MINOR};
            #endif
        }

        std::string Version::getCurrentString() {
            Version current = Version::getCurrent();
            return current.getString();
        }

        std::string Version::getString() const {
            std::ostringstream oss;
            oss << major << '.' << minor;
            if (patch >= 0) oss << '.' << patch;

            return oss.str();
        }

        bool operator>=(const Version &lhs, const Version &rhs) {
            if (lhs.major > rhs.major) return true;
            if (lhs.major < rhs.major) return false;

            if (lhs.minor > rhs.minor) return true;
            if (lhs.minor < rhs.minor) return false;

            return lhs.patch >= rhs.patch;
        }

        bool operator<(const Version &lhs, const Version &rhs) {
            return !(lhs >= rhs);
        }

        bool operator==(const Version &lhs, const Version &rhs) {
            return lhs.major == rhs.major && lhs.minor == rhs.minor && lhs.patch == rhs.patch;
        }

        std::ostream &operator<<(std::ostream &os, const Version &version) {
            os << version.major << '.' << version.minor;
            if (version.patch >= 0) os << '.' << version.patch;

            return os;
        }
    }
}
