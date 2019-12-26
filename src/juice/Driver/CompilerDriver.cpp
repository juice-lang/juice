// src/juice/Driver/CompilerDriver.cpp - Driver subclass for compiling a file
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/Driver/CompilerDriver.h"

#include <memory>
#include <string>
#include <utility>

#include "juice/AST/Codegen.h"
#include "juice/Basic/SourceManager.h"
#include "juice/Diagnostics/Diagnostics.h"
#include "juice/Parser/Parser.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"

namespace juice {
    namespace driver {
        int CompilerDriver::execute() {
            llvm::StringRef filename(_filename);

            auto manager = basic::SourceManager::mainFile(filename);
            if (manager == nullptr) {
                diag::DiagnosticEngine::diagnose(diag::DiagnosticID::file_not_found, filename);
                return 1;
            }

            auto diagnostics = std::make_shared<diag::DiagnosticEngine>(std::move(manager));

            parser::Parser juiceParser(diagnostics);

            auto ast = juiceParser.parseProgram();

            if (ast != nullptr) {
                ast->diagnoseInto(*diagnostics, 0);

                ast::Codegen codegen(std::move(ast));

                std::string error;
                llvm::raw_string_ostream errorStream(error);

                if (codegen.generate(errorStream)) {
                    codegen.dumpProgram(llvm::outs());
                } else {
                    errorStream.flush();

                    diagnostics->diagnose(diag::DiagnosticID::codegen_error, error);
                }
            }

            return diagnostics->hadError() ? 1 : 0;
        }
    }
}
