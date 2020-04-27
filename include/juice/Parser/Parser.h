// include/juice/Parser/Parser.h - Parser class, parses source file into an AST
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_PARSER_PARSER_H
#define JUICE_PARSER_PARSER_H

#include <array>
#include <functional>
#include <memory>
#include <tuple>
#include <type_traits>

#include "Lexer.h"
#include "LexerToken.h"
#include "juice/AST/AST.h"
#include "juice/AST/ExpressionAST.h"
#include "juice/AST/StatementAST.h"
#include "juice/AST/DeclarationAST.h"
#include "juice/Basic/SFINAE.h"
#include "juice/Diagnostics/Diagnostics.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/raw_ostream.h"

namespace juice {
    namespace parser {
        class Parser {
            class LexerError: public llvm::ErrorInfo<LexerError> {
            public:
                static char ID;

                void log(llvm::raw_ostream & os) const override {
                    os << "LexerError";
                }

                std::error_code convertToErrorCode() const override {
                    return llvm::inconvertibleErrorCode();
                }
            };

            class Error: public llvm::ErrorInfo<Error> {
                diag::DiagnosticID _diagnosticID;

            public:
                static char ID;

                Error() = delete;

                explicit Error(diag::DiagnosticID diagnosticID): _diagnosticID(diagnosticID) {}

                diag::DiagnosticID getDiagnosticID() const { return _diagnosticID; }

                void log(llvm::raw_ostream & os) const override {
                    os << "Error";
                }

                std::error_code convertToErrorCode() const override {
                    return llvm::inconvertibleErrorCode();
                }
            };

            class ErrorWithString: public llvm::ErrorInfo<ErrorWithString> {
                diag::DiagnosticID _diagnosticID;
                llvm::StringRef _name;

            public:
                static char ID;

                ErrorWithString() = delete;

                ErrorWithString(diag::DiagnosticID diagnosticID, llvm::StringRef name):
                    _diagnosticID(diagnosticID), _name(name) {}

                diag::DiagnosticID getDiagnosticID() const { return _diagnosticID; }
                llvm::StringRef getName() const { return _name; }

                void log(llvm::raw_ostream & os) const override {
                    os << "ErrorWithString";
                }

                std::error_code convertToErrorCode() const override {
                    return llvm::inconvertibleErrorCode();
                }
            };


            std::shared_ptr<diag::DiagnosticEngine> _diagnostics;
            std::unique_ptr<Lexer> _lexer;

            std::unique_ptr<LexerToken> _previousToken;
            std::unique_ptr<LexerToken> _currentToken;
            std::unique_ptr<LexerToken> _matchedToken;

            bool _inBlock;
            bool _wasNewline;

            bool isAtEnd();

            const std::unique_ptr<LexerToken> & getPreviousToken();

            bool check(LexerToken::Type type);

            template <typename... T, std::enable_if_t<basic::all_same<LexerToken::Type, T...>::value> * = nullptr>
            bool check(LexerToken::Type type, T... types);

            template <size_t size>
            bool check(const std::array<LexerToken::Type, size> & types);

            bool checkPrevious(LexerToken::Type type);

            llvm::Error advanceOne();
            llvm::Error skipNewlines();
            llvm::Expected<std::unique_ptr<LexerToken>> advance();

            llvm::Expected<bool> match(LexerToken::Type type);

            template <typename... T, std::enable_if_t<basic::all_same<LexerToken::Type, T...>::value> * = nullptr>
            llvm::Expected<bool> match(LexerToken::Type type, T... types);

            template <size_t size>
            llvm::Expected<bool> match(const std::array<LexerToken::Type, size> & types);

            llvm::Error consume(LexerToken::Type type, llvm::Error errorToReturn);
            llvm::Error consume(LexerToken::Type type, diag::DiagnosticID errorID);
            llvm::Error consume(LexerToken::Type type, diag::DiagnosticID errorID, llvm::StringRef name);

            llvm::Expected<std::unique_ptr<ast::BlockAST>> parseBlock(llvm::StringRef name);

            llvm::Expected<std::unique_ptr<ast::ControlFlowBodyAST>>
            parseControlFlowBody(std::unique_ptr<LexerToken> keyword);


            llvm::Expected<std::unique_ptr<ast::IfExpressionAST>> parseIfExpression(bool isStatement);
            llvm::Expected<std::unique_ptr<ast::ExpressionAST>> parseGroupedExpression();

            llvm::Expected<std::unique_ptr<ast::ExpressionAST>> parsePrimaryExpression();
            llvm::Expected<std::unique_ptr<ast::ExpressionAST>> parseMultiplicationPrecedenceExpression();
            llvm::Expected<std::unique_ptr<ast::ExpressionAST>> parseAdditionPrecedenceExpression();
            llvm::Expected<std::unique_ptr<ast::ExpressionAST>> parseComparisonPrecedenceExpression();
            llvm::Expected<std::unique_ptr<ast::ExpressionAST>> parseEqualityPrecedenceExpression();
            llvm::Expected<std::unique_ptr<ast::ExpressionAST>> parseLogicalAndPrecedenceExpression();
            llvm::Expected<std::unique_ptr<ast::ExpressionAST>> parseLogicalOrPrecedenceExpression();
            llvm::Expected<std::unique_ptr<ast::ExpressionAST>> parseAssignmentPrecedenceExpression();
            llvm::Expected<std::unique_ptr<ast::ExpressionAST>> parseExpression();


            llvm::Expected<std::unique_ptr<ast::ExpressionStatementAST>> parseExpressionStatement();
            llvm::Expected<std::unique_ptr<ast::WhileStatementAST>> parseWhileStatement();
            llvm::Expected<std::unique_ptr<ast::IfStatementAST>> parseIfStatement();
            llvm::Expected<std::unique_ptr<ast::BlockStatementAST>> parseBlockStatement();

            llvm::Expected<std::unique_ptr<ast::VariableDeclarationAST>> parseVariableDeclaration();

            llvm::Expected<std::unique_ptr<ast::StatementAST>> parseStatement();

            llvm::Error parseContainer(ast::ContainerAST & container,
                                       const std::function<bool(Parser *)> & endCondition = &Parser::isAtEnd);

        public:
            Parser() = delete;
            Parser(const Parser &) = delete;
            Parser & operator=(const Parser &) = delete;

            explicit Parser(std::shared_ptr<diag::DiagnosticEngine> diagnostics);

            std::unique_ptr<ast::ModuleAST> parseModule();
        };
    }
}

#endif //JUICE_PARSER_PARSER_H
