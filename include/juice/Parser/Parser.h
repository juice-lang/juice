// include/juice/Parser/Parser.h - Parser class, parses source file into an AST
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_PARSER_H
#define JUICE_PARSER_H

#include <exception>
#include <memory>
#include <tuple>

#include "juice/AST/AST.h"
#include "juice/AST/ExpressionAST.h"
#include "juice/AST/StatementAST.h"
#include "juice/AST/DeclarationAST.h"
#include "Lexer.h"
#include "LexerToken.h"
#include "juice/Diagnostics/Diagnostics.h"

namespace juice {
    namespace parser {
        class Parser {
            struct Error: std::exception {
                diag::DiagnosticID id;

                explicit Error(diag::DiagnosticID id): id(id) {}
            };

            struct LexerError: std::exception {};

            std::shared_ptr<diag::DiagnosticEngine> _diagnostics;
            std::unique_ptr<Lexer> _lexer;

            std::unique_ptr<LexerToken> _previousToken;
            std::unique_ptr<LexerToken> _currentToken;
            std::unique_ptr<LexerToken> _matchedToken;

            bool isAtEnd();
            bool check(LexerToken::Type type);

            void advanceOne();
            void skipNewlines();
            std::unique_ptr<LexerToken> advance();
            bool match(LexerToken::Type type);

            void consume(LexerToken::Type type, diag::DiagnosticID errorID);

            std::unique_ptr<ast::ExpressionAST> parseGroupedExpression();
            std::unique_ptr<ast::ExpressionAST> parsePrimaryExpression();
            std::unique_ptr<ast::ExpressionAST> parseMultiplicationPrecedenceExpression();
            std::unique_ptr<ast::ExpressionAST> parseAdditionPrecedenceExpression();
            std::unique_ptr<ast::ExpressionAST> parseAssignmentPrecedenceExpression();
            std::unique_ptr<ast::ExpressionAST> parseExpression();

            std::unique_ptr<ast::ExpressionStatementAST> parseExpressionStatement();

            std::unique_ptr<ast::VariableDeclarationAST> parseVariableDeclaration();

            std::unique_ptr<ast::StatementAST> parseStatement();

            std::unique_ptr<ast::ModuleAST> parseModule();

        public:
            Parser() = delete;
            Parser(const Parser &) = delete;
            Parser & operator=(const Parser &) = delete;

            explicit Parser(std::shared_ptr<diag::DiagnosticEngine> diagnostics);

            std::unique_ptr<ast::ModuleAST> parseProgram();
        };
    }
}

#endif //JUICE_PARSER_H
