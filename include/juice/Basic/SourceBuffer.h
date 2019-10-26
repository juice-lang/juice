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

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/MemoryBuffer.h"
#include "SourceLocation.h"

namespace juice {
    namespace basic {
        class SourceBuffer {
            const char * _start, * _end;
            llvm::StringRef _filename;
            bool _deleteBuffer;

        public:
            SourceBuffer() = delete;
            SourceBuffer(const SourceBuffer &) = delete;

            SourceBuffer & operator=(const SourceBuffer &) = delete;

            SourceBuffer(const char * start, const char * end, llvm::StringRef filename, bool deleteBuffer):
                    _start(start), _end(end), _filename(filename), _deleteBuffer(deleteBuffer) {}

            ~SourceBuffer();

            const char * getStart() const { return _start; }
            const char * getEnd() const { return _end; }
            size_t getSize() const { return getEnd() - getStart(); }

            llvm::StringRef getString() const { return {getStart(), getSize()}; }

            llvm::StringRef getFilename() const { return _filename; }
        };
    }
}

#endif //JUICE_SOURCEBUFFER_H
