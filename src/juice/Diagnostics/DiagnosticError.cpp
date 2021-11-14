// src/juice/Diagnostics/DiagnosticError.cpp - Error classes that contain parameters for DiagnosticEngine::diagnose
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2021 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/Diagnostics/DiagnosticError.h"

#include <utility>

namespace juice {
    namespace diag {
        const char DiagnosticError::ID = 0;

        DiagnosticError::DiagnosticError(basic::SourceLocation location, DiagnosticID diagnosticID,
                                         std::vector<DiagnosticArg> args):
            _location(location), _diagnosticID(diagnosticID), _args(std::move(args)) {}

        void DiagnosticError::diagnoseInto(diag::DiagnosticEngine & diagnostics) const {
            diagnostics.diagnose(_location, _diagnosticID, _args);
        }

        const char StaticDiagnosticError::ID = 0;

        StaticDiagnosticError::StaticDiagnosticError(DiagnosticID diagnosticID, std::vector<DiagnosticArg> args):
            _diagnosticID(diagnosticID), _args(std::move(args)) {}

        void StaticDiagnosticError::diagnose() const {
            DiagnosticEngine::diagnose(_diagnosticID, _args);
        }
    }
}
