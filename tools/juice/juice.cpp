// tools/juice/juice.cpp - Dummy juice tool
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/Driver/Driver.h"
#include "juice/Driver/VersionPrinter.h"
#include "llvm/Support/CommandLine.h"

int main(int argc, const char * const * argv) {
    llvm::cl::SetVersionPrinter(juice::driver::VersionPrinter::print);
    llvm::cl::ParseCommandLineOptions(argc, argv, "The juice-lang compiler", nullptr, nullptr, true);

    juice::driver::Driver * driver = juice::driver::Driver::create(argv[0]);
    int returnValue = driver->execute();

    delete driver;
    return returnValue;
}
