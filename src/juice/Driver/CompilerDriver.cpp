// src/juice/Driver/CompilerDriver.cpp - Driver subclass for compiling a file
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/Driver/CompilerDriver.h"

#include <memory>
#include <utility>

#include "juice/AST/Codegen.h"
#include "juice/Basic/RawStreamHelpers.h"
#include "juice/Basic/SourceManager.h"
#include "juice/Diagnostics/Diagnostics.h"
#include "juice/Parser/Parser.h"
#include "juice/Sema/TypeChecker.h"
#include "juice/Sema/TypeCheckedStatementAST.h"
#include "llvm/ADT/StringRef.h"

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

            auto ast = juiceParser.parseModule();

            if (ast) {
                llvm::outs() << basic::Color::bold << "=== AST ===\n" << basic::Color::reset;
                ast->diagnoseInto(*diagnostics, 0);

                sema::TypeChecker typeChecker(std::move(ast), diagnostics);
                auto typeCheckResult = typeChecker.typeCheck();

                if (!diagnostics->hadError()) {
                    llvm::outs() << basic::Color::bold << "\n\n=== TypeCheckedAST ===\n" << basic::Color::reset;
                    typeCheckResult.ast->diagnoseInto(*diagnostics, 0);

                    llvm::outs() << "\n\nAllocaVectorSize: " << typeCheckResult.allocaVectorSize << "\n";

//                    ast::Codegen codegen(std::move(ast), diagnostics);
//
//                    if (codegen.generate()) {
//                        codegen.dumpProgram();
//                    }

                    return 0;
                }
            }

            return 1;
        }
    }
}
