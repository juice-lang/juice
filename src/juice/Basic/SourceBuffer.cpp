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
        SourceBuffer::SourceBuffer(const char * start, const char * end, bool deletePointer):
                _start(start), _end(end), _deletePointer(deletePointer) {
            for (unsigned i = 0; i < getSize(); ++i) {
                if (_start[i] == '\n') {
                    _offsets.push_back(i);
                }
            }
        }

        SourceBuffer::~SourceBuffer() {
            if (_deletePointer) {
                delete _start;
            }
        }

        std::shared_ptr<SourceBuffer> SourceBuffer::getFile(StringRef filename) {
            std::ifstream file(filename.str());

            if (!file.is_open()) return nullptr;

            file.seekg(0, file.end);
            size_t length = file.tellg();
            file.seekg(0, file.beg);

            char * buffer = new char[length + 2];

            file.read(buffer, length);

            if (buffer[length - 1] != '\n') {
                buffer[length++] = '\n';
            }
            
            buffer[length] = 0;

            return std::make_shared<SourceBuffer>(buffer, buffer + length, true);
        }

        std::pair<unsigned, unsigned> SourceBuffer::getLineAndColumn(SourceLocation location) const {
            const char * pointer = location.getPointer();

            assert(pointer >= _start && pointer <= _end);
            unsigned pointerOffset = pointer - _start;

            auto eol = std::lower_bound(_offsets.begin(), _offsets.end(), pointerOffset);

            unsigned line = (1 + (eol - _offsets.begin()));

            size_t newlineOffset = StringRef(_start, pointerOffset).lastIndexOfContained("\n\r");
            if (newlineOffset == StringRef::npos) newlineOffset = ~(size_t)0;
            return std::make_pair(line, pointerOffset - newlineOffset);
        }

        StringRef SourceBuffer::getLineString(SourceLocation location) const {
            const char * pointer = location.getPointer();

            assert(pointer >= _start && pointer <= _end);
            unsigned pointerOffset = pointer - _start;

            auto eol = std::lower_bound(_offsets.begin(), _offsets.end(), pointerOffset);

            unsigned eolBefore;
            if (eol != _offsets.begin()) eolBefore = *(eol - 1);
            else eolBefore = 0;

            StringRef string(_start + eolBefore, *eol - eolBefore);
            return string.trim("\n\r");
        }
    }
}
