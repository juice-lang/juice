//
// Version.h
// juice
//
// Created by Josef Zoller on 2019-03-30.
//

#ifndef juice_version_h
#define juice_version_h

#include <iostream>

namespace juice {
    namespace version {
        class Version {
            int major, minor, patch;
        public:
            Version(int major, int minor, int patch = -1);
            static Version getCurrent();

            std::string getString() const;

            friend bool operator>=(const Version &lhs, const Version &rhs);
            friend bool operator==(const Version &lhs, const Version &rhs);
            friend std::ostream &operator<<(std::ostream &os, const Version &version);
        };

        bool operator>=(const Version &lhs, const Version &rhs);
        bool operator<(const Version &lhs, const Version &rhs);
        bool operator==(const Version &lhs, const Version &rhs);
        inline bool operator!=(const Version &lhs, const Version &rhs) {
            return !(lhs == rhs);
        }

        std::ostream &operator<<(std::ostream &os, const Version &version);
    }
}

#endif //juice_version_h
