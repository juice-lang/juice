// include/juice/Platform/MacOS/SDKPath.h - Helper function for getting the path of the macOS SDK
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2022 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_MACOS_SDKPATH_H
#define JUICE_MACOS_SDKPATH_H


#include "juice/Platform/Macros.h"

#if OS_MAC
#include <string>
#include "llvm/Support/Error.h"


namespace juice {
    namespace platform {
        namespace macOS {
            llvm::Expected<std::string> getSDKPath();
        }
    }
}
#endif //OS_MAC

#endif //JUICE_MACOS_SDKPATH_H
