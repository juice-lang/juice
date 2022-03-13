// src/juice/Platform/MacOS/SDKPath.cpp - Helper function for getting the path of the macOS SDK
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2022 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/Platform/Macros.h"

#if OS_MAC
#include "juice/Platform/MacOS/SDKPath.h"

#include <fstream>
#include <system_error>

#include "juice/Basic/Error.h"
#include "juice/Diagnostics/DiagnosticError.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Program.h"


namespace juice {
    namespace platform {
        namespace macOS {
            llvm::Expected<std::string> getSDKPath() {
                static const char * sdkPath;

                if (sdkPath) return sdkPath;

                llvm::SmallString<128> tempFilePath;
                if (std::error_code errorCode = llvm::sys::fs::createTemporaryFile("sdkpath", "", tempFilePath)) {
                    return basic::createError<diag::StaticDiagnosticError>(diag::DiagnosticID::error_creating_temporary,
                                                                           "sdkpath", errorCode);
                }


                auto xcrunPath = llvm::sys::findProgramByName("xcrun");
                if (std::error_code errorCode = xcrunPath.getError()) {
                    return basic::createError<diag::StaticDiagnosticError>(diag::DiagnosticID::error_finding_program,
                                                                           "xcrun", errorCode);
                }

                llvm::StringRef xcrunArguments[] = {
                    *xcrunPath,
                    "--sdk",
                    "macosx",
                    "--show-sdk-path"
                };

                int xcrunReturn = llvm::sys::ExecuteAndWait(*xcrunPath, xcrunArguments, llvm::None,
                                                            {llvm::None, {tempFilePath}, llvm::None});
                if (xcrunReturn) {
                    return basic::createError<diag::StaticDiagnosticError>(diag::DiagnosticID::error_executing,
                                                                           (llvm::StringRef)*xcrunPath);
                }


                std::ifstream tempFile((std::string)tempFilePath);

                auto start = tempFile.tellg();
                char c;
                while (tempFile.get(c) && c != '\n');

                auto end = tempFile.tellg();

                tempFile.seekg(start);

                char * buffer = new char[end - start + 1];

                char * current = buffer;
                while (tempFile.get(c) && c != '\n')
                    *current++ = c;

                *current = '\0';


                sdkPath = buffer;

                return sdkPath;
            }
        }
    }
}
#endif //OS_MAC
