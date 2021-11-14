// include/juice/Diagnostics/DiagnosticError.h - Error classes that contain parameters for DiagnosticEngine::diagnose
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2021 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_DIAGNOSTICS_DIAGNOSTICERROR_H
#define JUICE_DIAGNOSTICS_DIAGNOSTICERROR_H

#include <memory>
#include <utility>
#include <vector>

#include "Diagnostics.h"
#include "juice/Basic/SourceLocation.h"
#include "llvm/Support/Error.h"

namespace juice {
    namespace diag {
        class DiagnosticError: public llvm::ErrorInfo<DiagnosticError> {
            basic::SourceLocation _location;
            diag::DiagnosticID _diagnosticID;
            std::vector<DiagnosticArg> _args;

        public:
            static const char ID;

            DiagnosticError() = delete;

            DiagnosticError(basic::SourceLocation location, DiagnosticID diagnosticID, std::vector<DiagnosticArg> args);

            template<typename... Args>
            static std::unique_ptr<DiagnosticError>
            create(basic::SourceLocation location, DiagnosticID diagnosticID, Args... args) {
                std::vector<DiagnosticArg> vector;
                DiagnosticArg::getAllInto(vector, args...);

                return std::make_unique<DiagnosticError>(location, diagnosticID, std::move(vector));
            }


            void diagnoseInto(diag::DiagnosticEngine & diagnostics) const;


            basic::SourceLocation getLocation() const { return _location; }
            diag::DiagnosticID getDiagnosticID() const { return _diagnosticID; }
            const std::vector<DiagnosticArg> & getArgs() const { return _args; }

            void log(llvm::raw_ostream & os) const override {
                os << "DiagnosticError";
            }

            std::error_code convertToErrorCode() const override {
                return llvm::inconvertibleErrorCode();
            }
        };

        class StaticDiagnosticError: public llvm::ErrorInfo<StaticDiagnosticError> {
            diag::DiagnosticID _diagnosticID;
            std::vector<DiagnosticArg> _args;

        public:
            static const char ID;

            StaticDiagnosticError() = delete;

            StaticDiagnosticError(DiagnosticID diagnosticID, std::vector<DiagnosticArg> args);

            template<typename... Args>
            static std::unique_ptr<StaticDiagnosticError> create(DiagnosticID diagnosticID, Args... args) {
                std::vector<DiagnosticArg> vector;
                DiagnosticArg::getAllInto(vector, args...);

                return std::make_unique<StaticDiagnosticError>(diagnosticID, std::move(vector));
            }


            void diagnose() const;


            diag::DiagnosticID getDiagnosticID() const { return _diagnosticID; }
            const std::vector<DiagnosticArg> & getArgs() const { return _args; }

            void log(llvm::raw_ostream & os) const override {
                os << "StaticDiagnosticError";
            }

            std::error_code convertToErrorCode() const override {
                return llvm::inconvertibleErrorCode();
            }
        };
    }
}

#endif //JUICE_DIAGNOSTICS_DIAGNOSTICERROR_H
