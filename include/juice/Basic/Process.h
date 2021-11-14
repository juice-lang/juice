// include/juice/Basic/Process.h - Helpers for getting information about the current process
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2021 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_BASIC_PROCESS_H
#define JUICE_BASIC_PROCESS_H

#include <string>

#include "llvm/ADT/StringRef.h"

namespace juice {
    namespace basic {
        std::string getMainExecutablePath(const char * firstArgument);
    }
}

#endif //JUICE_BASIC_PROCESS_H
