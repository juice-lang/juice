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

#include "juice/Basic/Error.h"
#include "juice/Diagnostics/DiagnosticError.h"
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
                                                std::unique_ptr<TypeCheckedExpressionAST> initialization,
                                                Type variableType, size_t index):
            TypeCheckedDeclarationAST(Kind::variableDeclaration, NothingType::get()), _keyword(std::move(keyword)),
            _name(std::move(name)), _initialization(std::move(initialization)), _variableType(variableType),
            _index(index) {}

        void TypeCheckedVariableDeclarationAST::diagnoseInto(diag::DiagnosticEngine & diagnostics,
                                                             unsigned int level) const {
            basic::SourceLocation location(getLocation());

            diagnostics.diagnose(location, diag::DiagnosticID::type_checked_variable_declaration_ast, getColor(level),
                                 (unsigned int)_index, level, _name.get(), _variableType);
            _initialization->diagnoseInto(diagnostics, level + 1);
            diagnostics.diagnose(location, diag::DiagnosticID::ast_end, getColor(level), level);
        }

        std::unique_ptr<TypeCheckedVariableDeclarationAST>
        TypeCheckedVariableDeclarationAST::createByTypeChecking(std::unique_ptr<ast::VariableDeclarationAST> ast,
                                                                const TypeHint & hint, TypeChecker::State & state,
                                                                diag::DiagnosticEngine & diagnostics) {
            basic::SourceLocation location(ast->getLocation());
            auto name = std::move(ast->_name);

            Type annotatedType;
            if (ast->_typeAnnotation) {
                auto annotation = std::move(ast->_typeAnnotation);

                auto type = annotation->resolve(state);
                if (!basic::handleAllErrors(type.takeError(), [&](const diag::DiagnosticError & error) {
                    error.diagnoseInto(diagnostics);
                })) {
                    annotatedType = *type;
                }
            }

            const TypeHint & expectedHint = ExpectedTypeHint(annotatedType);
            const TypeHint & unknownHint = UnknownTypeHint();
            const TypeHint & initializationHint = annotatedType ? expectedHint : unknownHint;

            auto initialization = TypeCheckedExpressionAST
                ::createByTypeChecking(std::move(ast->_initialization), initializationHint, state, diagnostics);

            Type variableType = annotatedType ? annotatedType : initialization->getType();

            auto index = state.addVariableDeclaration(name->string, variableType);

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
                    Type expectedType = llvm::cast<ExpectedTypeHint>(hint).getType();
                    diagnostics.diagnose(location, diag::DiagnosticID::statement_ast_expected_type, expectedType);
                    break;
                }
            }

            return std::unique_ptr<TypeCheckedVariableDeclarationAST>(
                new TypeCheckedVariableDeclarationAST(std::move(ast->_keyword), std::move(name),
                                                      std::move(initialization), variableType, index.getValueOr(0)));
        }
    }
}
