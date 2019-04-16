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

#include <algorithm>
#include <fstream>
#include <vector>

#include "juice/Basic/StringRef.h"

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

        std::pair<unsigned, unsigned> SourceBuffer::getLineAndColumn(SourceLocation location) const {
            std::vector<unsigned> offsets;
            for (unsigned i = 0; i < getSize(); ++i) {
                if (_start[i] == '\n') {
                    offsets.push_back(i);
                }
            }

            const char * pointer = location.getPointer();

            assert(pointer >= _start && pointer <= _end);
            unsigned pointerOffset = pointer - _start;

            auto eol = std::lower_bound(offsets.begin(), offsets.end(), pointerOffset);

            unsigned line = (1 + (eol - offsets.begin()));

            size_t newlineOffset = StringRef(_start, pointerOffset).lastIndexOfContained("\n\r");
            if (newlineOffset == StringRef::npos) newlineOffset = ~(size_t)0;
            return std::make_pair(line, pointerOffset - newlineOffset);
        }
    }
}
