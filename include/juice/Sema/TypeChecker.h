// include/juice/Sema/TypeChecker.h - type checking class
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_SEMA_TYPECHECKER_H
#define JUICE_SEMA_TYPECHECKER_H

#include <memory>

#include "TypeCheckedAST.h"
#include "juice/AST/AST.h"

namespace juice {
    namespace sema {
        class TypeChecker {
            std::unique_ptr<ast::ModuleAST> _ast;

            std::shared_ptr<diag::DiagnosticEngine> _diagnostics;

        public:
            TypeChecker(std::unique_ptr<ast::ModuleAST> ast, std::shared_ptr<diag::DiagnosticEngine> diagnostics);

            std::unique_ptr<TypeCheckedModuleAST> typeCheck();
        };
    }
}

#endif //JUICE_SEMA_TYPECHECKER_H
