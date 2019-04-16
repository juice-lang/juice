// include/juice/Basic/SourceBuffer.h - SourceBuffer class, provides simple read-only access to source files
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_SOURCEBUFFER_H
#define JUICE_SOURCEBUFFER_H

#include <memory>
#include <string>
#include <utility>

#include "SourceLocation.h"

namespace juice {
    namespace basic {
        class SourceBuffer {
            const char * _start;
            const char * _end;
            bool _deletePointer;

        public:
            SourceBuffer(const SourceBuffer &) = delete;

            SourceBuffer & operator=(const SourceBuffer &) = delete;

            SourceBuffer(const char * start, const char * end, bool deletePointer):
                    _start(start), _end(end), _deletePointer(deletePointer) {}

            ~SourceBuffer();

            static std::shared_ptr<SourceBuffer> getFile(const std::string & filename);

            const char * getStart() const { return _start; }

            const char * getEnd() const { return _end; }

            size_t getSize() const { return _end - _start; }

            std::pair<unsigned, unsigned> getLineAndColumn(SourceLocation location) const;

            unsigned getLine(SourceLocation location) const {
                return getLineAndColumn(location).first;
            }
        };
    }
}

#endif //JUICE_SOURCEBUFFER_H
