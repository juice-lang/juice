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

#include <memory>
#include <system_error>
#include <utility>
#include <vector>

#include "juice/Basic/SourceBuffer.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Support/SourceMgr.h"

namespace juice {
    namespace diag {
        class DiagnosticKind;
    }

    namespace basic {
        class SourceManager {
            llvm::SourceMgr _sourceMgr;

            std::shared_ptr<SourceBuffer> _mainBuffer;

            SourceManager() = default;

        public:
            SourceManager(const SourceManager &) = delete;
            SourceManager & operator=(const SourceManager &) = delete;

            static std::unique_ptr<SourceManager> mainFile(llvm::StringRef filename);

            llvm::SourceMgr & getLLVMSourceMgr() {
                return _sourceMgr;
            }

            const llvm::SourceMgr & getLLVMSourceMgr() const {
                return _sourceMgr;
            }

            std::shared_ptr<SourceBuffer> getMainBuffer() const {
                return _mainBuffer;
            }

            unsigned int getLineNumber(SourceLocation location) const {
                return _sourceMgr.FindLineNumber(location.llvm());
            }

            std::pair<unsigned int, unsigned int> getLineAndColumn(SourceLocation location) const {
                return _sourceMgr.getLineAndColumn(location.llvm());
            }

            void printDiagnostic(llvm::Twine message, diag::DiagnosticKind kind, SourceLocation location);
        };
    }
}

#endif //JUICE_SOURCEMANAGER_H
