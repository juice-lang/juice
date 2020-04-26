// src/juice/Parser/Parser.cpp - Parser class, parses source file into an AST
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/Parser/Parser.h"

#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "juice/Basic/Error.h"
#include "juice/Basic/SourceLocation.h"

namespace juice {
    namespace parser {
        char Parser::LexerError::ID = 0;
        char Parser::Error::ID = 0;
        char Parser::ErrorWithString::ID = 0;

        bool Parser::isAtEnd() {
            return _currentToken == nullptr || _currentToken->type == LexerToken::Type::eof;
        }

        const std::unique_ptr<LexerToken> & Parser::getPreviousToken() {
            if (_previousToken) return _previousToken;
            return _matchedToken;
        }

        bool Parser::check(LexerToken::Type type) {
            if (isAtEnd()) return false;
            return _currentToken->type == type;
        }

        bool Parser::checkPrevious(LexerToken::Type type) {
            return getPreviousToken()->type == type;
        }

        llvm::Error Parser::advanceOne() {
            if (isAtEnd()) return llvm::make_error<Error>(diag::DiagnosticID::unexpected_parser_error);
            _previousToken = std::move(_currentToken);
            if (_previousToken) _wasNewline = (_previousToken->type == LexerToken::Type::delimiterNewline);
            _currentToken = _lexer->nextToken();
            if (check(LexerToken::Type::error)) return llvm::make_error<LexerError>();

            return llvm::Error::success();
        }

        llvm::Error Parser::skipNewlines() {
            while (check(LexerToken::Type::delimiterNewline)) {
                if (auto error = advanceOne()) return error;
            }

            return llvm::Error::success();
        }

        llvm::Expected<std::unique_ptr<LexerToken>> Parser::advance() {
            if (auto error = advanceOne()) return std::move(error);
            auto token = std::move(_previousToken);

            if (auto error = skipNewlines()) return std::move(error);

            return token;
        }

        llvm::Expected<bool> Parser::match(LexerToken::Type type) {
            if (check(type)) {
                auto matchedToken = advance();
                if (auto error = matchedToken.takeError()) return std::move(error);

                _matchedToken = std::move(*matchedToken);
                return true;
            }
            return false;
        }

        template<typename... T, std::enable_if_t<basic::all_same<LexerToken::Type, T...>::value> *>
        llvm::Expected<bool> Parser::match(LexerToken::Type type, T... types) {
            auto matched = match(type);
            if (auto error = matched.takeError()) return std::move(error);

            if (*matched) return true;

            return match(types...);
        }

        template<size_t size>
        llvm::Expected<bool> Parser::match(const std::array<LexerToken::Type, size> & types) {
            for (const auto & type: types) {
                auto matched = match(LexerToken::Type::delimiterLeftParen);
                if (auto error = matched.takeError()) return std::move(error);

                if (*matched) return true;
            }

            return false;
        }

        llvm::Error Parser::consume(LexerToken::Type type, llvm::Error errorToReturn) {
            if (check(type)) {
                auto matchedToken = advance();
                if (auto error = matchedToken.takeError()) return error;

                _matchedToken = std::move(*matchedToken);
                
                llvm::consumeError(std::move(errorToReturn));

                return llvm::Error::success();
            }

            return std::move(errorToReturn);
        }

        llvm::Error Parser::consume(LexerToken::Type type, diag::DiagnosticID errorID) {
            return consume(type, llvm::make_error<Error>(errorID));
        }

        llvm::Error Parser::consume(LexerToken::Type type, diag::DiagnosticID errorID, llvm::StringRef name) {
            return consume(type, llvm::make_error<ErrorWithString>(errorID, name));
        }


        llvm::Expected<std::unique_ptr<ast::IfBodyAST>> Parser::parseIfBody(std::unique_ptr<LexerToken> keyword) {
            if (check(LexerToken::Type::delimiterLeftBrace)) {
                auto block = parseBlock(keyword->string);
                if (auto error = block.takeError()) return std::move(error);

                return std::make_unique<ast::IfBodyAST>(std::move(keyword), std::move(*block));
            }

            if (auto error = consume(LexerToken::Type::delimiterColon,
                                     diag::DiagnosticID::expected_left_brace_or_colon, keyword->string))
                return std::move(error);

            auto expression = parseExpression();
            if (auto error = expression.takeError()) return std::move(error);

            return std::make_unique<ast::IfBodyAST>(std::move(keyword), std::move(*expression));
        }

        llvm::Expected<std::unique_ptr<ast::ExpressionAST>> Parser::parseIfExpression() {
            auto ifKeyword = std::move(_matchedToken);

            auto ifCondition = parseExpression();
            if (auto error = ifCondition.takeError()) return std::move(error);

            auto ifBody = parseIfBody(std::move(ifKeyword));
            if (auto error = ifBody.takeError()) return std::move(error);

            std::vector<std::pair<std::unique_ptr<ast::ExpressionAST>, std::unique_ptr<ast::IfBodyAST>>>
                elifConditionsAndBodies;

            auto matchedElif = match(LexerToken::Type::keywordElif);
            if (auto error = matchedElif.takeError()) return std::move(error);

            while (*matchedElif) {
                auto elifKeyword = std::move(_matchedToken);

                auto elifCondition = parseExpression();
                if (auto error = elifCondition.takeError()) return std::move(error);

                auto elifBody = parseIfBody(std::move(elifKeyword));
                if (auto error = elifBody.takeError()) return std::move(error);

                elifConditionsAndBodies.push_back({ std::move(*elifCondition), std::move(*elifBody) });

                matchedElif = match(LexerToken::Type::keywordElif);
                if (auto error = matchedElif.takeError()) return std::move(error);
            }


            if (auto error = consume(LexerToken::Type::keywordElse, diag::DiagnosticID::expected_else))
                return std::move(error);

            auto elseKeyword = std::move(_matchedToken);

            auto elseBody = parseIfBody(std::move(elseKeyword));
            if (auto error = elseBody.takeError()) return std::move(error);

            return std::make_unique<ast::IfExpressionAST>(std::move(*ifCondition), std::move(*ifBody),
                                                          std::move(*elseBody), std::move(elifConditionsAndBodies));
        }

        llvm::Expected<std::unique_ptr<ast::ExpressionAST>> Parser::parseGroupedExpression() {
            auto matched = match(LexerToken::Type::delimiterLeftParen);
            if (auto error = matched.takeError()) return std::move(error);

            if (*matched) {
                auto token = std::move(_matchedToken);

                auto expression = parseExpression();
                if (auto error = expression.takeError()) return std::move(error);

                if (auto error = consume(LexerToken::Type::delimiterRightParen,
                                         diag::DiagnosticID::expected_right_paren))
                    return std::move(error);

                return std::make_unique<ast::GroupingExpressionAST>(std::move(token), std::move(*expression));
            }

            return llvm::make_error<Error>(diag::DiagnosticID::expected_expression);
        }

        llvm::Expected<std::unique_ptr<ast::ExpressionAST>> Parser::parsePrimaryExpression() {
            auto matchedNumber = match(LexerToken::Type::integerLiteral, LexerToken::Type::decimalLiteral);
            if (auto error = matchedNumber.takeError()) return std::move(error);

            if (*matchedNumber) {
                auto token = std::move(_matchedToken);
                double number = std::stod(token->string.str());
                return std::make_unique<ast::NumberExpressionAST>(std::move(token), number);
            }

            auto matchedIdentifier = match(LexerToken::Type::identifier);
            if (auto error = matchedIdentifier.takeError()) return std::move(error);

            if (*matchedIdentifier) {
                auto token = std::move(_matchedToken);
                return std::make_unique<ast::VariableExpressionAST>(std::move(token));
            }

            auto matchedIf = match(LexerToken::Type::keywordIf);
            if (auto error = matchedIf.takeError()) return std::move(error);

            if (*matchedIf) return parseIfExpression();

            return parseGroupedExpression();
        }

        llvm::Expected<std::unique_ptr<ast::ExpressionAST>> Parser::parseMultiplicationPrecedenceExpression() {
            auto node = parsePrimaryExpression();
            if (auto error = node.takeError()) return std::move(error);

            auto matched = match(LexerToken::Type::operatorAsterisk, LexerToken::Type::operatorSlash);
            if (auto error = matched.takeError()) return std::move(error);

            while (*matched) {
                auto token = std::move(_matchedToken);

                auto right = parsePrimaryExpression();
                if (auto error = right.takeError()) return std::move(error);

                node = std::make_unique<ast::BinaryOperatorExpressionAST>(std::move(token), std::move(*node),
                                                                          std::move(*right));

                matched = match(LexerToken::Type::operatorAsterisk, LexerToken::Type::operatorSlash);
                if (auto error = matched.takeError()) return std::move(error);
            }

            return node;
        }

        llvm::Expected<std::unique_ptr<ast::ExpressionAST>> Parser::parseAdditionPrecedenceExpression() {
            auto node = parseMultiplicationPrecedenceExpression();
            if (auto error = node.takeError()) return std::move(error);

            auto matched = match(LexerToken::Type::operatorPlus, LexerToken::Type::operatorMinus);
            if (auto error = matched.takeError()) return std::move(error);

            while (*matched) {
                auto token = std::move(_matchedToken);

                auto right = parseMultiplicationPrecedenceExpression();
                if (auto error = right.takeError()) return std::move(error);

                node = std::make_unique<ast::BinaryOperatorExpressionAST>(std::move(token), std::move(*node),
                                                                          std::move(*right));

                matched = match(LexerToken::Type::operatorPlus, LexerToken::Type::operatorMinus);
                if (auto error = matched.takeError()) return std::move(error);
            }

            return node;
        }

        llvm::Expected<std::unique_ptr<ast::ExpressionAST>> Parser::parseAssignmentPrecedenceExpression() {
            auto node = parseAdditionPrecedenceExpression();
            if (auto error = node.takeError()) return std::move(error);

            auto matched = match(LexerToken::Type::operatorEqual, LexerToken::Type::operatorPlusEqual,
                                 LexerToken::Type::operatorMinusEqual, LexerToken::Type::operatorAsteriskEqual,
                                 LexerToken::Type::operatorSlashEqual);
            if (auto error = matched.takeError()) return std::move(error);

            while (*matched) {
                auto token = std::move(_matchedToken);

                auto right = parseAssignmentPrecedenceExpression();
                if (auto error = right.takeError()) return std::move(error);

                node = std::make_unique<ast::BinaryOperatorExpressionAST>(std::move(token), std::move(*node),
                                                                          std::move(*right));

                matched = match(LexerToken::Type::operatorEqual, LexerToken::Type::operatorPlusEqual,
                                LexerToken::Type::operatorMinusEqual, LexerToken::Type::operatorAsteriskEqual,
                                LexerToken::Type::operatorSlashEqual);
                if (auto error = matched.takeError()) return std::move(error);
            }

            return node;
        }

        llvm::Expected<std::unique_ptr<ast::ExpressionAST>> Parser::parseExpression() {
            return parseAssignmentPrecedenceExpression();
        }

        llvm::Expected<std::unique_ptr<ast::ExpressionStatementAST>> Parser::parseExpressionStatement() {
            auto expression = parseExpression();
            if (auto error = expression.takeError()) return std::move(error);

            if (!_wasNewline && !(_inBlock && check(LexerToken::Type::delimiterRightBrace))) {
                if (auto error = consume(LexerToken::Type::delimiterSemicolon,
                                         diag::DiagnosticID::expected_newline_or_semicolon, "expression"))
                    return std::move(error);
            }

            return std::make_unique<ast::ExpressionStatementAST>(std::move(*expression));
        }

        llvm::Expected<std::unique_ptr<ast::VariableDeclarationAST>> Parser::parseVariableDeclaration() {
            if (auto error = consume(LexerToken::Type::identifier, diag::DiagnosticID::expected_variable_name))
                return std::move(error);

            auto name = std::move(_matchedToken);

            if (auto error =
                consume(LexerToken::Type::operatorEqual, diag::DiagnosticID::expected_variable_initialization))
                return std::move(error);

            auto initialization = parseExpression();
            if (auto error = initialization.takeError()) return std::move(error);

            if (!_wasNewline && !(_inBlock && check(LexerToken::Type::delimiterRightBrace))) {
                if (auto error =
                    consume(LexerToken::Type::delimiterSemicolon,
                            diag::DiagnosticID::expected_newline_or_semicolon, "variable declaration"))
                    return std::move(error);
            }

            return std::make_unique<ast::VariableDeclarationAST>(std::move(name), std::move(*initialization));
        }

        llvm::Expected<std::unique_ptr<ast::BlockAST>> Parser::parseBlock(llvm::StringRef name) {
            if (auto error = consume(LexerToken::Type::delimiterLeftBrace,
                                     diag::DiagnosticID::expected_left_brace, name))
                return std::move(error);

            auto block = std::make_unique<ast::BlockAST>(std::move(_matchedToken));

            bool wasInBlock = _inBlock;
            _inBlock = true;

            if (auto error = parseContainer(*block, [](Parser * parser) {
                return parser->isAtEnd() || parser->check(LexerToken::Type::delimiterRightBrace);
            })) return std::move(error);

            if (auto error =
                consume(LexerToken::Type::delimiterRightBrace,
                        llvm::make_error<ErrorWithString>(diag::DiagnosticID::expected_right_brace, name)))
                return std::move(error);

            _inBlock = wasInBlock;

            return block;
        }

        llvm::Expected<std::unique_ptr<ast::StatementAST>> Parser::parseStatement() {
            auto matchedDo = match(LexerToken::Type::keywordDo);
            if (auto error = matchedDo.takeError()) return std::move(error);

            if (*matchedDo) {
                auto block = parseBlock("do");
                if (auto error = block.takeError()) return std::move(error);

                return std::make_unique<ast::BlockStatementAST>(std::move(*block));
            }

            auto matchedVar = match(LexerToken::Type::keywordVar);
            if (auto error = matchedVar.takeError()) return std::move(error);

            if (*matchedVar) return parseVariableDeclaration();

            return parseExpressionStatement();
        }

        llvm::Error Parser::parseContainer(ast::ContainerAST & container,
                                           const std::function<bool(Parser *)> & endCondition) {
            if (auto error = skipNewlines()) return error;

            while (!endCondition(this)) {
                auto statement = parseStatement();
                if (auto error = statement.takeError()) return error;

                container.appendStatement(std::move(*statement));
            }

            return llvm::Error::success();
        }

        Parser::Parser(std::shared_ptr<diag::DiagnosticEngine> diagnostics):
            _diagnostics(std::move(diagnostics)), _previousToken(nullptr), _matchedToken(nullptr), _inBlock(false),
            _wasNewline(false) {
            _lexer = std::make_unique<Lexer>(_diagnostics->getBuffer());
            _currentToken = _lexer->nextToken();
        }

        std::unique_ptr<ast::ModuleAST> Parser::parseModule() {
            auto module = std::make_unique<ast::ModuleAST>();

            if (auto error = llvm::handleErrors(parseContainer(*module), [this](const ErrorWithString & error) {
                    basic::SourceLocation location(_currentToken->string.begin());
                    _diagnostics->diagnose(location, error.getDiagnosticID(), error.getName());
                    return llvm::make_error<basic::ReturningError>();
                }, [this](const Error & error) {
                    basic::SourceLocation location(_currentToken->string.begin());
                    _diagnostics->diagnose(location, error.getDiagnosticID());
                    return llvm::make_error<basic::ReturningError>();
                }, [this](const LexerError & error) {
                    _currentToken->diagnoseInto(*_diagnostics);
                    return llvm::make_error<basic::ReturningError>();
                })) {
                llvm::consumeError(std::move(error));
                return nullptr;
            }

            return module;
        }
    }
}
