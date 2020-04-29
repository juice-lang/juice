// src/juice/Sema/TypeCheckedDeclarationAST.cpp - type checked declaration statement AST nodes
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/Sema/TypeCheckedDeclarationAST.h"

#include "juice/Sema/BuiltinType.h"
#include "llvm/Support/Casting.h"

namespace juice {
    namespace sema {
        std::unique_ptr<TypeCheckedDeclarationAST>
        TypeCheckedDeclarationAST::createByTypeChecking(std::unique_ptr<ast::DeclarationAST> ast, const TypeHint & hint,
                                                        diag::DiagnosticEngine & diagnostics) {
            switch (ast->getKind()) {
                case ast::StatementAST::Kind::variableDeclaration: {
                    auto variableDeclaration = std::unique_ptr<ast::VariableDeclarationAST>(
                        llvm::cast<ast::VariableDeclarationAST>(ast.release()));
                    return TypeCheckedVariableDeclarationAST::createByTypeChecking(std::move(variableDeclaration), hint,
                                                                                   diagnostics);
                }
                default:
                    llvm_unreachable("All possible DeclarationStatement kinds should be handled here");
            }
        }

        TypeCheckedVariableDeclarationAST
            ::TypeCheckedVariableDeclarationAST(std::unique_ptr<parser::LexerToken> name,
                                                std::unique_ptr<TypeCheckedExpressionAST> initialization):
            TypeCheckedDeclarationAST(new NothingType), _name(std::move(name)),
            _initialization(std::move(initialization)) {}

        std::unique_ptr<TypeCheckedVariableDeclarationAST>
        TypeCheckedVariableDeclarationAST::createByTypeChecking(std::unique_ptr<ast::VariableDeclarationAST> ast,
                                                                const TypeHint & hint,
                                                                diag::DiagnosticEngine & diagnostics) {
            basic::SourceLocation location(ast->getLocation());

            auto initialization = TypeCheckedExpressionAST
                ::createByTypeChecking(std::move(ast->_initialization),
                                       ExpectedTypeHint(BuiltinFloatingPointType::createDouble()), diagnostics);

            switch (hint.getKind()) {
                case TypeHint::Kind::none: break;
                case TypeHint::Kind::unknown: {
                    diagnostics.diagnose(location,
                                         diag::DiagnosticID::statement_ast_expected_unknown_type);
                    break;
                }
                case TypeHint::Kind::expected: {
                    auto expectedType = llvm::cast<ExpectedTypeHint>(hint).getType();
                    diagnostics.diagnose(location, diag::DiagnosticID::statement_ast_expected_type,
                                         expectedType);
                    break;
                }
            }

            return std::unique_ptr<TypeCheckedVariableDeclarationAST>(
                new TypeCheckedVariableDeclarationAST(std::move(ast->_name), std::move(initialization)));
        }
    }
}
