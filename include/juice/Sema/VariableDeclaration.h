// include/juice/Sema/VariableDeclaration.h - struct that stores all important information about a variable declaration
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2022 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_SEMA_VARIABLEDECLARATION_H
#define JUICE_SEMA_VARIABLEDECLARATION_H

#include "Type.h"
#include "llvm/ADT/StringRef.h"

namespace juice {
    namespace sema {
        struct VariableDeclaration {
            llvm::StringRef name;
            Type type;
            size_t index;
            bool isMutable;

            VariableDeclaration() = default;

            VariableDeclaration(llvm::StringRef name, Type type, size_t index, bool isMutable):
                name(name), type(type), index(index), isMutable(isMutable) {}
        };
    }
}

#endif //JUICE_SEMA_VARIABLEDECLARATION_H
