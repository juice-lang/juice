// src/juice/Basic/SourceBuffer.cpp - SourceBuffer class, provides simple read-only access to source files
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/Basic/SourceBuffer.h"

#include <fstream>

namespace juice {
    namespace basic {
        SourceBuffer::~SourceBuffer() {
            if (_deletePointer) {
                delete _start;
            }
        }

        std::shared_ptr<SourceBuffer> SourceBuffer::getFile(const std::string & filename) {
            std::ifstream file(filename);

            if (!file.is_open()) return nullptr;

            file.seekg(0, file.end);
            size_t length = file.tellg();
            file.seekg(0, file.beg);

            char * buffer = new char[length + 1];

            file.read(buffer, length);

            buffer[length] = 0;

            return std::make_shared<SourceBuffer>(buffer, buffer + length, true);
        }
    }
}
