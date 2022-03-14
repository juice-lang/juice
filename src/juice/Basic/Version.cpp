// src/juice/Basic/Version.cpp - Version class to get the current juice version
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
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
            _major(major), _minor(minor), _patch(patch) {}

        Version Version::getCurrent() {
            #if JUICE_HAS_VERSION_PATCHLEVEL
                return {JUICE_VERSION_MAJOR, JUICE_VERSION_MINOR, JUICE_VERSION_PATCHLEVEL};
            #else
                return {JUICE_VERSION_MAJOR, JUICE_VERSION_MINOR};
            #endif
        }

        std::string Version::getCurrentString() {
            Version current = Version::getCurrent();
            return current.getString();
        }

        llvm::Optional<Version> Version::getLLVM() {
            #if JUICE_HAS_LLVM_VERSION
                return {llvm::in_place,
                        JUICE_LLVM_VERSION_MAJOR,
                        JUICE_LLVM_VERSION_MINOR,
                        JUICE_LLVM_VERSION_PATCHLEVEL};
            #else
                return {};
            #endif
        }

        std::string Version::getString() const {
            std::ostringstream oss;
            oss << _major << '.' << _minor;
            if (_patch >= 0) oss << '.' << _patch;

            return oss.str();
        }

        bool operator>=(const Version & lhs, const Version & rhs) {
            if (lhs._major > rhs._major) return true;
            if (lhs._major < rhs._major) return false;

            if (lhs._minor > rhs._minor) return true;
            if (lhs._minor < rhs._minor) return false;

            return lhs._patch >= rhs._patch;
        }

        bool operator<(const Version & lhs, const Version & rhs) {
            return !(lhs >= rhs);
        }

        bool operator==(const Version & lhs, const Version & rhs) {
            return lhs._major == rhs._major && lhs._minor == rhs._minor && lhs._patch == rhs._patch;
        }

        llvm::raw_ostream & operator<<(llvm::raw_ostream & os, const Version & version) {
            os << version._major << '.' << version._minor;
            if (version._patch >= 0) os << '.' << version._patch;

            return os;
        }
    }
}
