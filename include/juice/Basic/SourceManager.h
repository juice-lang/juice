// include/juice/Basic/SourceManager.h - Manager for Source Buffers
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_SOURCEMANAGER_H
#define JUICE_SOURCEMANAGER_H

#include <exception>
#include <memory>
#include <system_error>
#include <utility>
#include <vector>

#include "juice/Basic/SourceBuffer.h"
#include "juice/Basic/StringRef.h"
#include "llvm/Support/SourceMgr.h"

namespace juice {
    namespace diag {
        class DiagnosticKind;
    }

    namespace basic {
        class SourceException: public std::exception {
            std::error_code _code;

        public:
            explicit SourceException(std::error_code code): _code(code) {}

            std::error_code getCode() const { return _code; }
        };

        class SourceManager {
            llvm::SourceMgr _sourceMgr;

            std::vector<std::shared_ptr<SourceBuffer>> _buffers;

        public:
            SourceManager() = delete;
            SourceManager(const SourceManager &) = delete;
            SourceManager & operator=(const SourceManager &) = delete;

            explicit SourceManager(StringRef filename);

            llvm::SourceMgr & getLLVMSourceMgr() {
                return _sourceMgr;
            }

            const llvm::SourceMgr & getLLVMSourceMgr() const {
                return _sourceMgr;
            }

            std::shared_ptr<SourceBuffer> getMainBuffer() const {
                return _buffers[0];
            }

            unsigned getLineNumber(SourceLocation location) const {
                return _sourceMgr.FindLineNumber(location.llvm());
            }

            std::pair<unsigned, unsigned> getLineAndColumn(SourceLocation location) const {
                return _sourceMgr.getLineAndColumn(location.llvm());
            }

            void printDiagnostic(StringRef text, diag::DiagnosticKind kind, SourceLocation location);
        };
    }
}

#endif //JUICE_SOURCEMANAGER_H
