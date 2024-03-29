// include/juice/Driver/Driver.h - Driver class
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_DRIVER_DRIVER_H
#define JUICE_DRIVER_DRIVER_H

namespace juice {
    namespace driver {
        class Driver {
        public:
            virtual ~Driver() = default;

            virtual int execute() = 0;

            static Driver * create(const char * firstArg);
        };
    }
}

#endif //JUICE_DRIVER_DRIVER_H
