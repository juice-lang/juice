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

#include "juice/Basic/Error.h"
#include "juice/Basic/RawStreamHelpers.h"
#include "juice/Basic/SourceManager.h"
#include "juice/Diagnostics/DiagnosticError.h"
#include "juice/Diagnostics/Diagnostics.h"
#include "juice/IRGen/IRGen.h"
#include "juice/Parser/Parser.h"
#include "juice/Sema/TypeChecker.h"
#include "juice/Sema/TypeCheckedStatementAST.h"
#include "llvm/ADT/StringRef.h"

namespace juice {
    namespace driver {
        llvm::cl::SubCommand frontendSubcommand("frontend");

        llvm::cl::opt<std::string> FrontendDriver::inputFile(
            llvm::cl::sub(frontendSubcommand),
            "input-file",
            llvm::cl::Required
        );

        llvm::cl::opt<std::string> FrontendDriver::outputFile(
            llvm::cl::sub(frontendSubcommand),
            "output-file",
            llvm::cl::Required
        );

        llvm::cl::opt<FrontendDriver::Action> FrontendDriver::action(
            llvm::cl::sub(frontendSubcommand),
            llvm::cl::values(
                clEnumValN(Action::dumpParse, "dump-parse", ""),
                clEnumValN(Action::dumpAST, "dump-ast", ""),
                clEnumValN(Action::emitIR, "emit-ir", ""),
                clEnumValN(Action::emitObject, "emit-object", "")
            ),
            llvm::cl::Required
        );


        FrontendDriver::~FrontendDriver() {
            delete _outputOS;
        }


        int FrontendDriver::execute() {
            llvm::StringRef filePath(inputFile);

            auto manager = basic::SourceManager::mainFile(filePath);
            if (manager == nullptr) {
                diag::DiagnosticEngine::diagnose(diag::DiagnosticID::file_not_found, filePath);
                return 1;
            }

            auto expectedOutputOS = getOutputOS();
            if (basic::handleAllErrors(expectedOutputOS.takeError(), [](const diag::StaticDiagnosticError & error) {
                error.diagnose();
            })) {
                return 1;
            }

            llvm::raw_pwrite_stream & outputOS = expectedOutputOS.get();

            auto diagnostics = std::make_shared<diag::DiagnosticEngine>(std::move(manager), outputOS);

            parser::Parser juiceParser(diagnostics);

            auto ast = juiceParser.parseModule();

            if (ast) {
                if (action == Action::dumpParse) {
                    ast->diagnoseInto(*diagnostics, 0);

                    return 0;
                }

                sema::TypeChecker typeChecker(std::move(ast), diagnostics);
                auto typeCheckResult = typeChecker.typeCheck();

                if (!diagnostics->hadError()) {
                    if (action == Action::dumpAST) {
                        typeCheckResult.ast->diagnoseInto(*diagnostics, 0);

                        return 0;
                    }

                    irgen::IRGen codegen(std::move(typeCheckResult), diagnostics);

                    if (codegen.generate()) {
                        if (action == Action::emitIR) {
                            codegen.dumpProgram(outputOS);

                            return 0;
                        }

                        if (codegen.emitObject(outputOS)) {
                            return 0;
                        }
                    }
                }
            }

            return 1;
        }

        llvm::Expected<llvm::raw_pwrite_stream &> FrontendDriver::getOutputOS() {
            if (outputFile == "-") {
                return llvm::outs();
            } else if (_outputOS) {
                return *_outputOS;
            } else {
                std::error_code errorCode;
                _outputOS = new llvm::raw_fd_ostream(outputFile, errorCode);

                if (errorCode)
                    return basic::createError<diag::StaticDiagnosticError>(
                        diag::DiagnosticID::error_opening_output_file, (llvm::StringRef)outputFile, errorCode);

                return *_outputOS;
            }
        }
    }
}
