// include/juice/Basic/Version.h - Version class to get the current juice version
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_VERSION_H
#define JUICE_VERSION_H

#include <iostream>

namespace juice {
    namespace basic {
        class Version {
            int _major, _minor, _patch;
        public:
            Version(int major, int minor, int patch = -1);
            static Version getCurrent();
            static std::string getCurrentString();

            std::string getString() const;

            friend bool operator>=(const Version & lhs, const Version & rhs);
            friend bool operator==(const Version & lhs, const Version & rhs);
            friend std::ostream & operator<<(std::ostream & os, const Version & version);
        };

        bool operator>=(const Version & lhs, const Version & rhs);
        bool operator<(const Version & lhs, const Version & rhs);
        bool operator==(const Version & lhs, const Version & rhs);
        inline bool operator!=(const Version & lhs, const Version & rhs) {
            return !(lhs == rhs);
        }

        std::ostream & operator<<(std::ostream & os, const Version & version);
    }
}

#endif //JUICE_VERSION_H
