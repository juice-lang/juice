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
#include <memory>
#include <vector>

#include "juice/Basic/StringRef.h"

namespace juice {
    namespace basic {
        SourceBuffer::SourceBuffer(const llvm::MemoryBuffer * buffer):
                _start(buffer->getBufferStart()), _end(buffer->getBufferEnd()),
                _filename(buffer->getBufferIdentifier()), _deletePointer(false) {}

        SourceBuffer::~SourceBuffer() {
            if (_deletePointer) {
                delete getStart();
            }
        }
    }
}
