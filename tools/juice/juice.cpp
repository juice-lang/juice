//
// juice.cpp
// juice
//
// Created by Josef Zoller on 2019-03-30.
//

#include "juice/Basic/Version.h"

#include <iostream>

using juice::version::Version;

int main(int argv, char ** argc) {
    Version version = Version::getCurrent();
    std::cout << "juice version " << version << std::endl;
}
