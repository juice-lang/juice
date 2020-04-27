// include/juice/Basic/SourceLocation.h - Source location and range for use with diagnostics
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_BASIC_SOURCELOCATION_H
#define JUICE_BASIC_SOURCELOCATION_H

#include "llvm/Support/SMLoc.h"

namespace juice {
    namespace basic {
        class SourceLocation {
            const char * _pointer = nullptr;

        public:
            SourceLocation() = default;
            explicit SourceLocation(const char * pointer): _pointer(pointer) {}

            bool isValid() const { return _pointer != nullptr; }
            bool isInvalid() const { return !isValid(); }

            bool operator==(const SourceLocation & rhs) const { return rhs._pointer == _pointer; }
            bool operator!=(const SourceLocation & rhs) const { return !operator==(rhs); }

            const char * getPointer() const { return _pointer; }

            llvm::SMLoc llvm() const { return llvm::SMLoc::getFromPointer(_pointer); }
        };

        class SourceRange {
            SourceLocation _start, _end;

        public:
            SourceRange() = default;
            explicit SourceRange(SourceLocation location): _start(location), _end(location) {}
            SourceRange(SourceLocation start, SourceLocation end): _start(start), _end(end) {}

            bool isValid() const { return _start.isValid(); }
            bool isInvalid() const { return !isValid(); }

            bool operator==(const SourceRange & other) const {
                return _start == other._start && _end == other._end;
            }
            bool operator!=(const SourceRange & other) const { return !operator==(other); }
        };
    }
}

#endif //JUICE_BASIC_SOURCELOCATION_H
