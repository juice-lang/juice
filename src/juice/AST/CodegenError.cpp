// src/juice/AST/CodegenError.cpp - Errors for exiting the codegen process
//
// This source file is part of the juice open source project
//
// Copyright (c) 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors

#include "juice/AST/CodegenError.h"

namespace juice {
    namespace ast {
        char CodegenError::ID = 0;

        void CodegenError::diagnoseInto(const std::shared_ptr<diag::DiagnosticEngine> & diagnostics) const {
            diagnostics->diagnose(_location, _diagnosticID);
        }

        void
        CodegenErrorWithString::diagnoseInto(const std::shared_ptr<diag::DiagnosticEngine> & diagnostics) const {
            diagnostics->diagnose(_location, _diagnosticID, _name);
        }
    }
}
