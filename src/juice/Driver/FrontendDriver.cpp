// src/juice/Driver/FrontendDriver.cpp - Driver subclass that does the actual compiler work
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2021 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/Driver/FrontendDriver.h"

#include <memory>
#include <string>
#include <utility>

#include "juice/Basic/RawStreamHelpers.h"
#include "juice/Basic/SourceManager.h"
#include "juice/Diagnostics/Diagnostics.h"
#include "juice/IRGen/IRGen.h"
#include "juice/Parser/Parser.h"
#include "juice/Sema/TypeChecker.h"
#include "juice/Sema/TypeCheckedStatementAST.h"
#include "llvm/ADT/StringRef.h"

namespace juice {
    namespace driver {
        llvm::cl::SubCommand frontendSubcommand("frontend");

        static llvm::cl::opt<std::string> inputPath(
            "input-file",
            llvm::cl::sub(frontendSubcommand)
        );

        static llvm::cl::opt<std::string> outputPath(
            "output-file",
            llvm::cl::sub(frontendSubcommand)
        );

        int FrontendDriver::execute() {
            llvm::StringRef filePath(inputPath);

            auto manager = basic::SourceManager::mainFile(filePath);
            if (manager == nullptr) {
                diag::DiagnosticEngine::diagnose(diag::DiagnosticID::file_not_found, filePath);
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

                    irgen::IRGen codegen(std::move(typeCheckResult), diagnostics);

                    if (codegen.generate()) {
                        llvm::outs() << basic::Color::bold << "\n\n=== LLVM IR ===\n\n" << basic::Color::reset;
                        codegen.dumpProgram();

                        if (codegen.emitObject(outputPath)) {
                            return 0;
                        }
                    }
                }
            }

            return 1;
        }
    }
}
