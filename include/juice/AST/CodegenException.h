// include/juice/AST/CodegenException.h - Exceptions for exiting the codegen process
//
// This source file is part of the juice open source project
//
// Copyright (c) 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_CODEGENEXCEPTION_H
#define JUICE_CODEGENEXCEPTION_H

#include <exception>

#include "juice/Basic/SourceLocation.h"
#include "juice/Diagnostics/Diagnostics.h"
#include "llvm/ADT/StringRef.h"

namespace juice {
    namespace ast {
        struct CodegenException: std::exception {
            diag::DiagnosticID id;
            basic::SourceLocation location;

            CodegenException() = delete;

            CodegenException(diag::DiagnosticID id, basic::SourceLocation location): id(id), location(location) {}

            virtual void diagnoseInto(const std::shared_ptr<diag::DiagnosticEngine> & diagnostics) const;
        };

        struct VariableException: CodegenException {
            llvm::StringRef name;

            VariableException(diag::DiagnosticID id, basic::SourceLocation location, llvm::StringRef name):
                    CodegenException(id, location), name(name) {}

            void diagnoseInto(const std::shared_ptr<diag::DiagnosticEngine> &diagnostics) const override;
        };
    }
}

#endif //JUICE_CODEGENEXCEPTION_H
