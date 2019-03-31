// tools/juice/juice.cpp - Dummy juice tool
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/Basic/Version.h"

#include <iostream>

using juice::version::Version;

int main(int argv, char ** argc) {
    Version version = Version::getCurrent();
    std::cout << "juice version " << version << std::endl;
}
