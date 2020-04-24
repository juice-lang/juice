// include/juice/AST/CodegenError.h - Errors for exiting the codegen process
//
// This source file is part of the juice open source project
//
// Copyright (c) 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_CODEGENERROR_H
#define JUICE_CODEGENERROR_H

#include "juice/Basic/SourceLocation.h"
#include "juice/Diagnostics/Diagnostics.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/raw_ostream.h"

namespace juice {
    namespace ast {
        class CodegenError: public llvm::ErrorInfo<CodegenError> {
        protected:
            diag::DiagnosticID _diagnosticID;
            basic::SourceLocation _location;

        public:
            static char ID;

            CodegenError() = delete;

            CodegenError(diag::DiagnosticID diagnosticID, basic::SourceLocation location):
                _diagnosticID(diagnosticID), _location(location) {}

            virtual void diagnoseInto(const std::shared_ptr<diag::DiagnosticEngine> & diagnostics) const;

            void log(llvm::raw_ostream & os) const override {
                os << "CodegenError";
            }

            std::error_code convertToErrorCode() const override {
                return llvm::inconvertibleErrorCode();
            }
        };

        class CodegenErrorWithString: public CodegenError {
            llvm::StringRef _name;

        public:
            CodegenErrorWithString(diag::DiagnosticID diagnosticID, basic::SourceLocation location, llvm::StringRef name):
                CodegenError(diagnosticID, location), _name(name) {}

            void diagnoseInto(const std::shared_ptr<diag::DiagnosticEngine> &diagnostics) const override;

            void log(llvm::raw_ostream & os) const override {
                os << "CodegenErrorWithString";
            }
        };
    }
}

#endif //JUICE_CODEGENERROR_H
