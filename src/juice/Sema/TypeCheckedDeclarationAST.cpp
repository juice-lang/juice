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
                                                        TypeChecker::State & state,
                                                        diag::DiagnosticEngine & diagnostics) {
            switch (ast->getKind()) {
                case ast::StatementAST::Kind::variableDeclaration: {
                    auto variableDeclaration = std::unique_ptr<ast::VariableDeclarationAST>(
                        llvm::cast<ast::VariableDeclarationAST>(ast.release()));
                    return TypeCheckedVariableDeclarationAST::createByTypeChecking(std::move(variableDeclaration), hint,
                                                                                   state, diagnostics);
                }
                default:
                    llvm_unreachable("All possible DeclarationStatement kinds should be handled here");
            }
        }

        TypeCheckedVariableDeclarationAST
            ::TypeCheckedVariableDeclarationAST(std::unique_ptr<parser::LexerToken> keyword,
                                                std::unique_ptr<parser::LexerToken> name,
                                                std::unique_ptr<TypeCheckedExpressionAST> initialization, size_t index):
            TypeCheckedDeclarationAST(Kind::variableDeclaration, new NothingType), _keyword(std::move(keyword)),
            _name(std::move(name)), _initialization(std::move(initialization)), _index(index) {}

        void TypeCheckedVariableDeclarationAST::diagnoseInto(diag::DiagnosticEngine & diagnostics,
                                                             unsigned int level) const {
            basic::SourceLocation location(getLocation());

            diagnostics.diagnose(location, diag::DiagnosticID::type_checked_variable_declaration_ast, getColor(level),
                                 getType(), (unsigned int)_index, level, _name.get());
            _initialization->diagnoseInto(diagnostics, level + 1);
            diagnostics.diagnose(location, diag::DiagnosticID::ast_end, getColor(level), level);
        }

        std::unique_ptr<TypeCheckedVariableDeclarationAST>
        TypeCheckedVariableDeclarationAST::createByTypeChecking(std::unique_ptr<ast::VariableDeclarationAST> ast,
                                                                const TypeHint & hint, TypeChecker::State & state,
                                                                diag::DiagnosticEngine & diagnostics) {
            basic::SourceLocation location(ast->getLocation());
            auto name = std::move(ast->_name);

            auto initialization = TypeCheckedExpressionAST
                ::createByTypeChecking(std::move(ast->_initialization),
                                       ExpectedTypeHint(BuiltinFloatingPointType::getDouble()), state, diagnostics);

            auto index = state.addDeclaration(name->string, initialization->getType());

            if (!index) {
                diagnostics.diagnose(location, diag::DiagnosticID::variable_declaration_ast_redeclaration,
                                     name->string);
            }

            switch (hint.getKind()) {
                case TypeHint::Kind::none: break;
                case TypeHint::Kind::unknown: {
                    diagnostics.diagnose(location, diag::DiagnosticID::statement_ast_expected_unknown_type);
                    break;
                }
                case TypeHint::Kind::expected: {
                    auto expectedType = llvm::cast<ExpectedTypeHint>(hint).getType();
                    diagnostics.diagnose(location, diag::DiagnosticID::statement_ast_expected_type, expectedType);
                    break;
                }
            }

            return std::unique_ptr<TypeCheckedVariableDeclarationAST>(
                new TypeCheckedVariableDeclarationAST(std::move(ast->_keyword), std::move(name),
                                                      std::move(initialization), index.getValueOr(0)));
        }
    }
}
