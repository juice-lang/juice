// src/juice/AST/DeclarationAST.cpp - AST nodes for declaration parsing
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors

#include "juice/AST/DeclarationAST.h"

#include <utility>

namespace juice {
    namespace ast {
        VariableDeclarationAST::VariableDeclarationAST(std::unique_ptr<parser::LexerToken> keyword,
                                                       std::unique_ptr<parser::LexerToken> name,
                                                       std::unique_ptr<TypeRepr> typeAnnotation,
                                                       std::unique_ptr<ExpressionAST> initialization):
                DeclarationAST(Kind::variableDeclaration), _keyword(std::move(keyword)), _name(std::move(name)),
                _typeAnnotation(std::move(typeAnnotation)), _initialization(std::move(initialization)) {}

        void VariableDeclarationAST::diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const {
            basic::SourceLocation location(getLocation());

            diagnostics.diagnose(location, diag::DiagnosticID::variable_declaration_ast, getColor(level), level,
                                 _name.get(), _typeAnnotation.get());
            _initialization->diagnoseInto(diagnostics, level + 1);
            diagnostics.diagnose(location, diag::DiagnosticID::ast_end, getColor(level), level);
        }
    }
}
