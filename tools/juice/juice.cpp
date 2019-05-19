// tools/juice/juice.cpp - Dummy juice tool
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include <string>
#include <vector>

#include "juice/Driver/Driver.h"

using namespace juice::driver;

int main(int argc, char ** argv) {
    std::vector<std::string> args(argv, argv + argc);

    Driver * driver = Driver::withArguments(args);
    int returnValue = driver->execute();

    delete driver;
    return returnValue;
}
