// src/juice/Driver/VersionDriver.cpp - Driver subclass for -v and --version arguments
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/Driver/VersionDriver.h"

#include "juice/Basic/Version.h"
#include "llvm/Support/raw_ostream.h"

namespace juice {
    namespace driver {
        int VersionDriver::execute() {
            auto currentVersion = basic::Version::getCurrent();
            llvm::outs() << "juice version " << currentVersion << '\n';
            return 0;
        }
    }
}
