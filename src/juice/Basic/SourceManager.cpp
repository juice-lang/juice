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

#include "juice/Basic/RawStreamHelpers.h"
#include "juice/Diagnostics/Diagnostics.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/SMLoc.h"

namespace juice {
    namespace basic {
        std::unique_ptr<SourceManager> SourceManager::mainFile(llvm::StringRef filename) {
            std::unique_ptr<SourceManager> manager(new SourceManager);
            llvm::SourceMgr & mgr = manager->_sourceMgr;

            auto buffer = llvm::MemoryBuffer::getFile(filename);

            if (buffer.getError()) return nullptr;

            unsigned int index = mgr.AddNewSourceBuffer(std::move(buffer.get()), llvm::SMLoc());

            manager->_mainBuffer = std::make_shared<SourceBuffer>(mgr.getMemoryBuffer(index)->getBufferStart(),
                                                                  mgr.getMemoryBuffer(index)->getBufferEnd(), filename,
                                                                  false);

            return manager;
        }

        void SourceManager::printDiagnostic(llvm::Twine message, diag::DiagnosticKind kind, SourceLocation location) {
            llvm::raw_ostream & os = kind == diag::DiagnosticKind::error ? llvm::errs() : llvm::outs();

            llvm::SMDiagnostic diagnostic = _sourceMgr.GetMessage(location.llvm(), kind.llvm(), message);

            os << Color::bold << Color::yellow << "juice: " << Color::reset;

            _sourceMgr.PrintMessage(os, diagnostic);
        }
    }
}
