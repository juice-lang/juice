// src/juice/Basic/SourceManager.cpp - Manager for Source Buffers
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors

#include "juice/Basic/SourceManager.h"

#include <utility>

#include "juice/Diagnostics/Diagnostics.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/SMLoc.h"

namespace juice {
    namespace basic {
        SourceManager::SourceManager(llvm::StringRef filename) {
            auto buffer = llvm::MemoryBuffer::getFile(filename);
            if (auto error = buffer.getError()) throw SourceException(error);
            unsigned id = _sourceMgr.AddNewSourceBuffer(std::move(buffer.get()), llvm::SMLoc());

            _buffers.push_back(std::make_shared<SourceBuffer>(_sourceMgr.getMemoryBuffer(id)));
        }

        void SourceManager::printDiagnostic(llvm::Twine message, diag::DiagnosticKind kind, SourceLocation location) {
            llvm::SMDiagnostic diagnostic = _sourceMgr.GetMessage(location.llvm(), kind.llvm(), message);
            _sourceMgr.PrintMessage(kind == diag::DiagnosticKind::error ? llvm::errs() : llvm::outs(), diagnostic);
        }
    }
}
