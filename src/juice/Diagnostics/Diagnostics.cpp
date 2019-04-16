// src/juice/Diagnostics/Diagnostics.cpp - DiagnosticEngine class, manages any diagnostics emitted by juice
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/Diagnostics/Diagnostics.h"

#include <iostream>
#include <utility>

namespace juice {
    namespace diag {
        static const constexpr DiagnosticKind diagnosticKinds[] {
            #define ERROR(ID, Text) DiagnosticKind::error,
            #define WARNING(ID, Text) DiagnosticKind::warning,
            #include "juice/Diagnostics/Diagnostics.def"
        };

        static constexpr const char * const diagnosticStrings[] {
            #define ERROR(ID, Text) Text,
            #define WARNING(ID, Text) Text,
            #include "juice/Diagnostics/Diagnostics.def"
        };

        DiagnosticEngine::DiagnosticEngine(std::shared_ptr<juice::basic::SourceBuffer> sourceBuffer):
                _sourceBuffer(std::move(sourceBuffer)), _output(std::cout), _errorOutput(std::cerr) {}

        DiagnosticEngine::DiagnosticEngine(std::shared_ptr<basic::SourceBuffer> sourceBuffer, std::ostream & output,
                                           std::ostream & errorOutput):
                _sourceBuffer(std::move(sourceBuffer)), _output(output), _errorOutput(errorOutput) {}

        void DiagnosticEngine::diagnose(basic::SourceLocation location, DiagnosticID id,
                                        const std::vector<DiagnosticArg> & args) {
            _output << "location: " << location.getPointer() << std::endl;
            
            DiagnosticKind kind = diagnosticKindFor(id);
            basic::StringRef text(diagnosticStringFor(id));
            
            switch (kind) {
                case DiagnosticKind::error: {
                    _output << "kind: error" << std::endl;
                    break;
                }
                case DiagnosticKind::warning: {
                    _output << "kind: warning" << std::endl;
                    break;
                }
            }
            _output << "text: " << text << std::endl;
        }
    }
}
