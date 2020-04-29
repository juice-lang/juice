// src/juice/Sema/TypeChecker.cpp - type checking class
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/Sema/TypeChecker.h"

#include "juice/Sema/BuiltinType.h"
#include "juice/Sema/TypeCheckedAST.h"
#include "juice/Sema/TypeCheckedStatementAST.h"
#include "juice/Sema/TypeHint.h"

namespace juice {
    namespace sema {
        TypeChecker::TypeChecker(std::unique_ptr<ast::ModuleAST> ast,
                                 std::shared_ptr<diag::DiagnosticEngine> diagnostics):
            _ast(std::move(ast)), _diagnostics(std::move(diagnostics)) {}

        std::unique_ptr<TypeCheckedModuleAST> TypeChecker::typeCheck() {
            return
                TypeCheckedModuleAST::createByTypeChecking(std::move(_ast),
                                                           ExpectedTypeHint(BuiltinFloatingPointType::createDouble()),
                                                           *_diagnostics);
        }
    }
}
