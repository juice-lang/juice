// include/juice/Diagnostics/Diagnostics.h - DiagnosticEngine class, manages any diagnostics emitted by juice
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_DIAGNOSTICS_H
#define JUICE_DIAGNOSTICS_H

#include <cstdint>

namespace juice {
    namespace diag {
        enum class DiagnosticID: uint32_t {
            #define DIAG(KIND, ID, Text) ID,
            #include "Diagnostics.def"
        };

        enum class DiagnosticKind {
            error,
            warning
        };
    }
}

#endif //JUICE_DIAGNOSTICS_H
