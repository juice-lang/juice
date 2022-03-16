// src/juice/Parser/Parser.cpp - Parser class, parses source file into an AST
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/Parser/Parser.h"

#include <string>
#include <utility>

#include "juice/Diagnostics/DiagnosticError.h"
#include "juice/Basic/Error.h"
#include "juice/Basic/SourceLocation.h"

namespace juice {
    namespace parser {
        const char Parser::LexerError::ID = 0;

        template <typename... Args>
        llvm::Error Parser::createError(diag::DiagnosticID diagnosticID, Args &&... args) {
            basic::SourceLocation location(_currentToken->string.begin());
            return basic::createError<diag::DiagnosticError>(location, diagnosticID, std::forward<Args>(args)...);
        }

        bool Parser::isAtEnd() {
            return _currentToken == nullptr || _currentToken->type == LexerToken::Type::eof;
        }

        const std::unique_ptr<LexerToken> & Parser::getPreviousToken() {
            if (_previousToken) return _previousToken;
            return _matchedToken;
        }

        const std::unique_ptr<LexerToken> & Parser::getPreviousLookaheadToken() {
            switch (_lookaheadTokens.size()) {
                case 0: return getPreviousToken();
                case 1: return _currentToken;
                default: return *(_lookaheadTokens.end() - 1);
            }
        }

        const std::unique_ptr<LexerToken> & Parser::getCurrentLookaheadToken() {
            return _lookaheadTokens.empty() ? _currentToken : _lookaheadTokens.back();
        }

        std::unique_ptr<LexerToken> Parser::moveFirstLookaheadToken() {
            assert(!_lookaheadTokens.empty() && "There should be at least one lookahead token.");

            auto first = std::move(_lookaheadTokens.front());
            _lookaheadTokens.pop_front();

            return first;
        }

        bool Parser::check(LexerToken::Type type) {
            if (isAtEnd()) return false;
            return _currentToken->type == type;
        }

        template<typename... T, std::enable_if_t<basic::all_same_v<LexerToken::Type, T...>> *>
        bool Parser::check(LexerToken::Type type, T... types) {
            if (check(type)) return true;

            return check(types...);
        }

        template<size_t size>
        bool Parser::check(const std::array<LexerToken::Type, size> & types) {
            for (const auto & type: types) {
                if (check(type)) return true;
            }

            return false;
        }

        bool Parser::checkPrevious(LexerToken::Type type) {
            return getPreviousToken()->type == type;
        }

        llvm::Error Parser::advanceOne() {
            if (isAtEnd()) return createError(diag::DiagnosticID::unexpected_parser_error);
            _previousToken = std::move(_currentToken);
            if (_previousToken) _wasNewline = (_previousToken->type == LexerToken::Type::delimiterNewline);
            _currentToken = _lookaheadTokens.empty() ? _lexer->nextToken() : moveFirstLookaheadToken();
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
            if (auto error = advanceOne()) return error;
            auto token = std::move(_previousToken);

            if (auto error = skipNewlines()) return error;

            return token;
        }

        llvm::Expected<bool> Parser::match(LexerToken::Type type) {
            if (check(type)) {
                auto matchedToken = advance();
                if (auto error = matchedToken.takeError()) return error;

                _matchedToken = std::move(*matchedToken);
                return true;
            }
            return false;
        }

        template<typename... T, std::enable_if_t<basic::all_same_v<LexerToken::Type, T...>> *>
        llvm::Expected<bool> Parser::match(LexerToken::Type type, T... types) {
            auto matched = match(type);
            if (auto error = matched.takeError()) return error;

            if (*matched) return true;

            return match(types...);
        }

        template<size_t size>
        llvm::Expected<bool> Parser::match(const std::array<LexerToken::Type, size> & types) {
            for (const auto & type: types) {
                auto matched = match(LexerToken::Type::delimiterLeftParen);
                if (auto error = matched.takeError()) return error;

                if (*matched) return true;
            }

            return false;
        }

        template <typename... Args>
        llvm::Error Parser::consume(LexerToken::Type type, diag::DiagnosticID diagnosticID, Args &&... args) {
            if (check(type)) {
                auto matchedToken = advance();
                if (auto error = matchedToken.takeError()) return error;

                _matchedToken = std::move(*matchedToken);

                return llvm::Error::success();
            }

            return createError(diagnosticID, std::forward<Args>(args)...);
        }

        bool Parser::lookaheadIsAtEnd() {
            return getCurrentLookaheadToken() == nullptr || getCurrentLookaheadToken()->type == LexerToken::Type::eof;
        }

        bool Parser::checkLookahead(LexerToken::Type type) {
            if (lookaheadIsAtEnd()) return false;
            if (_lookaheadTokens.empty()) return check(type);
            return getCurrentLookaheadToken()->type == type;
        }

        template<typename... T, std::enable_if_t<basic::all_same_v<LexerToken::Type, T...>> *>
        bool Parser::checkLookahead(LexerToken::Type type, T... types) {
            if (checkLookahead(type)) return true;

            return checkLookahead(types...);
        }

        template<size_t size>
        bool Parser::checkLookahead(const std::array<LexerToken::Type, size> & types) {
            for (const auto & type: types) {
                if (checkLookahead(type)) return true;
            }

            return false;
        }

        bool Parser::checkPreviousLookahead(LexerToken::Type type) {
            return getPreviousLookaheadToken()->type == type;
        }

        llvm::Error Parser::advanceLookaheadOne() {
            if (lookaheadIsAtEnd()) return createError(diag::DiagnosticID::unexpected_parser_error);
            _lookaheadTokens.push_back(_lexer->nextToken());
            if (checkLookahead(LexerToken::Type::error)) return llvm::make_error<LexerError>();

            return llvm::Error::success();
        }

        llvm::Error Parser::lookaheadSkipNewlines() {
            while (checkLookahead(LexerToken::Type::delimiterNewline)) {
                if (auto error = advanceLookaheadOne()) return error;
            }

            return llvm::Error::success();
        }

        llvm::Expected<const std::unique_ptr<LexerToken> &> Parser::advanceLookahead() {
            if (auto error = advanceLookaheadOne()) return error;
            const auto & token = getPreviousLookaheadToken();

            if (auto error = lookaheadSkipNewlines()) return error;

            return token;
        }

        llvm::Expected<bool> Parser::matchLookahead(LexerToken::Type type) {
            if (checkLookahead(type)) {
                auto matchedToken = advanceLookahead();
                if (auto error = matchedToken.takeError()) return error;
                
                return true;
            }
            return false;
        }

        template<typename... T, std::enable_if_t<basic::all_same_v<LexerToken::Type, T...>> *>
        llvm::Expected<bool> Parser::matchLookahead(LexerToken::Type type, T... types) {
            auto matched = matchLookahead(type);
            if (auto error = matched.takeError()) return error;

            if (*matched) return true;

            return matchLookahead(types...);
        }

        template<size_t size>
        llvm::Expected<bool> Parser::matchLookahead(const std::array<LexerToken::Type, size> & types) {
            for (const auto & type: types) {
                auto matched = matchLookahead(LexerToken::Type::delimiterLeftParen);
                if (auto error = matched.takeError()) return error;

                if (*matched) return true;
            }

            return false;
        }


        llvm::Expected<std::unique_ptr<ast::BlockAST>> Parser::parseBlock(llvm::StringRef name) {
            if (auto error = consume(LexerToken::Type::delimiterLeftBrace,
                                     diag::DiagnosticID::expected_left_brace, name))
                return error;

            auto block = std::make_unique<ast::BlockAST>(std::move(_matchedToken));

            bool wasInBlock = _inBlock;
            _inBlock = true;

            if (auto error = parseContainer(*block, [](Parser * parser) {
                return parser->isAtEnd() || parser->check(LexerToken::Type::delimiterRightBrace);
            })) return error;

            if (auto error = consume(LexerToken::Type::delimiterRightBrace,
                                     diag::DiagnosticID::expected_right_brace, name))
                return error;

            _inBlock = wasInBlock;

            return block;
        }

        llvm::Expected<std::unique_ptr<ast::ControlFlowBodyAST>>
        Parser::parseControlFlowBody(std::unique_ptr<LexerToken> keyword) {
            if (check(LexerToken::Type::delimiterLeftBrace)) {
                auto block = parseBlock(keyword->string);
                if (auto error = block.takeError()) return error;

                return std::make_unique<ast::ControlFlowBodyAST>(std::move(keyword), std::move(*block));
            }

            if (auto error = consume(LexerToken::Type::delimiterColon,
                                     diag::DiagnosticID::expected_left_brace_or_colon, keyword->string))
                return error;

            auto expression = parseExpression();
            if (auto error = expression.takeError()) return error;

            return std::make_unique<ast::ControlFlowBodyAST>(std::move(keyword), std::move(*expression));
        }

        llvm::Expected<std::unique_ptr<ast::IfExpressionAST>> Parser::parseIfExpression(bool isStatement) {
            auto ifKeyword = std::move(_matchedToken);

            auto ifCondition = parseExpression();
            if (auto error = ifCondition.takeError()) return error;

            auto ifBody = parseControlFlowBody(std::move(ifKeyword));
            if (auto error = ifBody.takeError()) return error;

            ast::IfExpressionAST::ElifVector elifConditionsAndBodies;

            auto matchedElif = match(LexerToken::Type::keywordElif);
            if (auto error = matchedElif.takeError()) return error;

            while (*matchedElif) {
                auto elifKeyword = std::move(_matchedToken);

                auto elifCondition = parseExpression();
                if (auto error = elifCondition.takeError()) return error;

                auto elifBody = parseControlFlowBody(std::move(elifKeyword));
                if (auto error = elifBody.takeError()) return error;

                elifConditionsAndBodies.emplace_back(std::move(*elifCondition), std::move(*elifBody));

                matchedElif = match(LexerToken::Type::keywordElif);
                if (auto error = matchedElif.takeError()) return error;
            }


            if (isStatement) {
                auto matchedElse = match(LexerToken::Type::keywordElse);
                if (auto error = matchedElse.takeError()) return error;

                std::unique_ptr<ast::ControlFlowBodyAST> elseBody = nullptr;

                if (*matchedElse) {
                    auto elseKeyword = std::move(_matchedToken);

                    auto expectedElseBody = parseControlFlowBody(std::move(elseKeyword));
                    if (auto error = expectedElseBody.takeError()) return error;

                    elseBody = std::move(*expectedElseBody);
                }

                return std::make_unique<ast::IfExpressionAST>(std::move(*ifCondition), std::move(*ifBody),
                                                              std::move(elifConditionsAndBodies), std::move(elseBody),
                                                              true);
            }


            if (auto error = consume(LexerToken::Type::keywordElse, diag::DiagnosticID::expected_else))
                return error;

            auto elseKeyword = std::move(_matchedToken);

            auto elseBody = parseControlFlowBody(std::move(elseKeyword));
            if (auto error = elseBody.takeError()) return error;

            return std::make_unique<ast::IfExpressionAST>(std::move(*ifCondition), std::move(*ifBody),
                                                          std::move(elifConditionsAndBodies), std::move(*elseBody),
                                                          false);
        }

        llvm::Expected<std::unique_ptr<ast::ExpressionAST>> Parser::parseGroupedExpression() {
            auto matched = match(LexerToken::Type::delimiterLeftParen);
            if (auto error = matched.takeError()) return error;

            if (*matched) {
                auto token = std::move(_matchedToken);

                auto expression = parseExpression();
                if (auto error = expression.takeError()) return error;

                if (auto error = consume(LexerToken::Type::delimiterRightParen,
                                         diag::DiagnosticID::expected_right_paren))
                    return error;

                return std::make_unique<ast::GroupingExpressionAST>(std::move(token), std::move(*expression));
            }

            return createError(diag::DiagnosticID::expected_expression);
        }

        llvm::Expected<std::unique_ptr<ast::ExpressionAST>> Parser::parsePrimaryExpression() {
            auto matchedInteger = match(LexerToken::Type::integerLiteral);
            if (auto error = matchedInteger.takeError()) return error;

            if (*matchedInteger) {
                auto token = std::move(_matchedToken);
                int64_t value = std::stoll(token->string.str());
                return std::make_unique<ast::IntegerLiteralExpressionAST>(std::move(token), value);
            }

            auto matchedFloatingPoint = match(LexerToken::Type::floatingPointLiteral);
            if (auto error = matchedFloatingPoint.takeError()) return error;

            if (*matchedFloatingPoint) {
                auto token = std::move(_matchedToken);
                double value = std::stod(token->string.str());
                return std::make_unique<ast::FloatingPointLiteralExpressionAST>(std::move(token), value);
            }
            
            auto matchedBooleanLiteral = match(LexerToken::Type::keywordTrue, LexerToken::Type::keywordFalse);
            if (auto error = matchedBooleanLiteral.takeError()) return error;

            if (*matchedBooleanLiteral) {
                auto token = std::move(_matchedToken);
                bool value = token->type == LexerToken::Type::keywordTrue;
                return std::make_unique<ast::BooleanLiteralExpressionAST>(std::move(token), value);
            }

            auto matchedIdentifier = match(LexerToken::Type::identifier);
            if (auto error = matchedIdentifier.takeError()) return error;

            if (*matchedIdentifier) {
                auto token = std::move(_matchedToken);
                return std::make_unique<ast::VariableExpressionAST>(std::move(token));
            }

            auto matchedIf = match(LexerToken::Type::keywordIf);
            if (auto error = matchedIf.takeError()) return error;

            if (*matchedIf) return parseIfExpression(false);

            return parseGroupedExpression();
        }

        llvm::Expected<std::unique_ptr<ast::ExpressionAST>> Parser::parseMultiplicationPrecedenceExpression() {
            auto node = parsePrimaryExpression();
            if (auto error = node.takeError()) return error;

            auto matched = match(LexerToken::Type::operatorAsterisk, LexerToken::Type::operatorSlash);
            if (auto error = matched.takeError()) return error;

            while (*matched) {
                auto token = std::move(_matchedToken);

                auto right = parsePrimaryExpression();
                if (auto error = right.takeError()) return error;

                node = std::make_unique<ast::BinaryOperatorExpressionAST>(std::move(token), std::move(*node),
                                                                          std::move(*right));
                if (auto error = node.takeError()) return error;

                matched = match(LexerToken::Type::operatorAsterisk, LexerToken::Type::operatorSlash);
                if (auto error = matched.takeError()) return error;
            }

            return node;
        }

        llvm::Expected<std::unique_ptr<ast::ExpressionAST>> Parser::parseAdditionPrecedenceExpression() {
            auto node = parseMultiplicationPrecedenceExpression();
            if (auto error = node.takeError()) return error;

            auto matched = match(LexerToken::Type::operatorPlus, LexerToken::Type::operatorMinus);
            if (auto error = matched.takeError()) return error;

            while (*matched) {
                auto token = std::move(_matchedToken);

                auto right = parseMultiplicationPrecedenceExpression();
                if (auto error = right.takeError()) return error;

                node = std::make_unique<ast::BinaryOperatorExpressionAST>(std::move(token), std::move(*node),
                                                                          std::move(*right));
                if (auto error = node.takeError()) return error;

                matched = match(LexerToken::Type::operatorPlus, LexerToken::Type::operatorMinus);
                if (auto error = matched.takeError()) return error;
            }

            return node;
        }

        llvm::Expected<std::unique_ptr<ast::ExpressionAST>> Parser::parseComparisonPrecedenceExpression() {
            auto node = parseAdditionPrecedenceExpression();
            if (auto error = node.takeError()) return error;

            auto matched = match(LexerToken::Type::operatorLower, LexerToken::Type::operatorLowerEqual,
                                 LexerToken::Type::operatorGreater, LexerToken::Type::operatorGreaterEqual);
            if (auto error = matched.takeError()) return error;

            if (*matched) {
                auto token = std::move(_matchedToken);

                auto right = parseAdditionPrecedenceExpression();
                if (auto error = right.takeError()) return error;

                node = std::make_unique<ast::BinaryOperatorExpressionAST>(std::move(token), std::move(*node),
                                                                          std::move(*right));
                if (auto error = node.takeError()) return error;

                if (check(LexerToken::Type::operatorLower, LexerToken::Type::operatorLowerEqual,
                          LexerToken::Type::operatorGreater, LexerToken::Type::operatorGreaterEqual))
                    return createError(diag::DiagnosticID::unexpected_operator, "comparison");
            }

            return node;
        }

        llvm::Expected<std::unique_ptr<ast::ExpressionAST>> Parser::parseEqualityPrecedenceExpression() {
            auto node = parseComparisonPrecedenceExpression();
            if (auto error = node.takeError()) return error;

            auto matched = match(LexerToken::Type::operatorEqualEqual, LexerToken::Type::operatorBangEqual);
            if (auto error = matched.takeError()) return error;

            if (*matched) {
                auto token = std::move(_matchedToken);

                auto right = parseComparisonPrecedenceExpression();
                if (auto error = right.takeError()) return error;

                node = std::make_unique<ast::BinaryOperatorExpressionAST>(std::move(token), std::move(*node),
                                                                          std::move(*right));
                if (auto error = node.takeError()) return error;

                if (check(LexerToken::Type::operatorEqualEqual, LexerToken::Type::operatorBangEqual))
                    return createError(diag::DiagnosticID::unexpected_operator, "equality");
            }

            return node;
        }

        llvm::Expected<std::unique_ptr<ast::ExpressionAST>> Parser::parseLogicalAndPrecedenceExpression() {
            auto node = parseEqualityPrecedenceExpression();
            if (auto error = node.takeError()) return error;

            auto matched = match(LexerToken::Type::operatorAndAnd);
            if (auto error = matched.takeError()) return error;

            while (*matched) {
                auto token = std::move(_matchedToken);

                auto right = parseEqualityPrecedenceExpression();
                if (auto error = right.takeError()) return error;

                node = std::make_unique<ast::BinaryOperatorExpressionAST>(std::move(token), std::move(*node),
                                                                          std::move(*right));
                if (auto error = node.takeError()) return error;

                matched = match(LexerToken::Type::operatorAndAnd);
                if (auto error = matched.takeError()) return error;
            }

            return node;
        }

        llvm::Expected<std::unique_ptr<ast::ExpressionAST>> Parser::parseLogicalOrPrecedenceExpression() {
            auto node = parseLogicalAndPrecedenceExpression();
            if (auto error = node.takeError()) return error;

            auto matched = match(LexerToken::Type::operatorPipePipe);
            if (auto error = matched.takeError()) return error;

            while (*matched) {
                auto token = std::move(_matchedToken);

                auto right = parseLogicalAndPrecedenceExpression();
                if (auto error = right.takeError()) return error;

                node = std::make_unique<ast::BinaryOperatorExpressionAST>(std::move(token), std::move(*node),
                                                                          std::move(*right));
                if (auto error = node.takeError()) return error;

                matched = match(LexerToken::Type::operatorPipePipe);
                if (auto error = matched.takeError()) return error;
            }

            return node;
        }

        llvm::Expected<std::unique_ptr<ast::ExpressionAST>> Parser::parseAssignmentPrecedenceExpression() {
            auto node = parseLogicalOrPrecedenceExpression();
            if (auto error = node.takeError()) return error;

            auto matched = match(LexerToken::Type::operatorEqual, LexerToken::Type::operatorPlusEqual,
                                 LexerToken::Type::operatorMinusEqual, LexerToken::Type::operatorAsteriskEqual,
                                 LexerToken::Type::operatorSlashEqual);
            if (auto error = matched.takeError()) return error;

            while (*matched) {
                auto token = std::move(_matchedToken);

                auto right = parseAssignmentPrecedenceExpression();
                if (auto error = right.takeError()) return error;

                node = std::make_unique<ast::BinaryOperatorExpressionAST>(std::move(token), std::move(*node),
                                                                          std::move(*right));
                if (auto error = node.takeError()) return error;

                matched = match(LexerToken::Type::operatorEqual, LexerToken::Type::operatorPlusEqual,
                                LexerToken::Type::operatorMinusEqual, LexerToken::Type::operatorAsteriskEqual,
                                LexerToken::Type::operatorSlashEqual);
                if (auto error = matched.takeError()) return error;
            }

            return node;
        }

        llvm::Expected<std::unique_ptr<ast::ExpressionAST>> Parser::parseExpression() {
            return parseAssignmentPrecedenceExpression();
        }

        llvm::Expected<std::unique_ptr<ast::ExpressionStatementAST>> Parser::parseExpressionStatement() {
            auto expression = parseExpression();
            if (auto error = expression.takeError()) return error;

            if (!_wasNewline && !(_inBlock && check(LexerToken::Type::delimiterRightBrace))) {
                if (auto error = consume(LexerToken::Type::delimiterSemicolon,
                                         diag::DiagnosticID::expected_newline_or_semicolon, "expression"))
                    return error;
            }

            return std::make_unique<ast::ExpressionStatementAST>(std::move(*expression));
        }

        llvm::Expected<std::unique_ptr<ast::WhileStatementAST>> Parser::parseWhileStatement() {
            auto keyword = std::move(_matchedToken);

            auto condition = parseExpression();
            if (auto error = condition.takeError()) return error;

            auto body = parseControlFlowBody(std::move(keyword));
            if (auto error = body.takeError()) return error;

            return std::make_unique<ast::WhileStatementAST>(std::move(*condition), std::move(*body));
        }

        llvm::Expected<std::unique_ptr<ast::IfStatementAST>> Parser::parseIfStatement() {
            auto ifExpression = parseIfExpression(true);
            if (auto error = ifExpression.takeError()) return error;

            return std::make_unique<ast::IfStatementAST>(std::move(*ifExpression));
        }

        llvm::Expected<std::unique_ptr<ast::BlockStatementAST>> Parser::parseBlockStatement() {
            auto block = parseBlock("do");
            if (auto error = block.takeError()) return error;

            return std::make_unique<ast::BlockStatementAST>(std::move(*block));
        }

        llvm::Expected<std::unique_ptr<ast::TypeRepr>> Parser::parseIdentifierType() {
            return std::make_unique<ast::IdentifierTypeRepr>(std::move(_matchedToken));
        }

        llvm::Expected<std::unique_ptr<ast::TypeRepr>> Parser::parseType() {
            auto matchedIdentifier = match(LexerToken::Type::identifier);
            if (auto error = matchedIdentifier.takeError()) return error;

            if (*matchedIdentifier) {
                return parseIdentifierType();
            }

            return createError(diag::DiagnosticID::expected_type);
        }

        llvm::Expected<std::unique_ptr<ast::TypeRepr>> Parser::parseTypeAnnotation() {
            return parseType();
        }

        llvm::Expected<std::unique_ptr<ast::VariableDeclarationAST>> Parser::parseVariableDeclaration() {
            auto keyword = std::move(_matchedToken);

            if (auto error = consume(LexerToken::Type::identifier, diag::DiagnosticID::expected_variable_name))
                return error;

            auto name = std::move(_matchedToken);

            auto matchedColon = match(LexerToken::Type::delimiterColon);
            if (auto error = matchedColon.takeError()) return error;

            std::unique_ptr<ast::TypeRepr> typeAnnotation;
            if (*matchedColon) {
                auto type = parseTypeAnnotation();
                if (auto error = type.takeError()) return error;

                typeAnnotation = std::move(*type);
            }

            if (auto error = consume(LexerToken::Type::operatorEqual,
                                     diag::DiagnosticID::expected_variable_initialization))
                return error;

            auto initialization = parseExpression();
            if (auto error = initialization.takeError()) return error;

            if (!_wasNewline && !(_inBlock && check(LexerToken::Type::delimiterRightBrace))) {
                if (auto error = consume(LexerToken::Type::delimiterSemicolon,
                                         diag::DiagnosticID::expected_newline_or_semicolon, "variable declaration"))
                    return error;
            }

            return std::make_unique<ast::VariableDeclarationAST>(std::move(keyword), std::move(name),
                                                                 std::move(typeAnnotation), std::move(*initialization));
        }

        llvm::Expected<std::unique_ptr<ast::StatementAST>> Parser::parseStatement() {
            auto matchedVar = match(LexerToken::Type::keywordVar);
            if (auto error = matchedVar.takeError()) return error;

            if (*matchedVar) return parseVariableDeclaration();


            auto matchedDo = match(LexerToken::Type::keywordDo);
            if (auto error = matchedDo.takeError()) return error;

            if (*matchedDo) return parseBlockStatement();


            auto matchedIf = match(LexerToken::Type::keywordIf);
            if (auto error = matchedIf.takeError()) return error;

            if (*matchedIf) return parseIfStatement();


            auto matchedWhile = match(LexerToken::Type::keywordWhile);
            if (auto error = matchedWhile.takeError()) return error;

            if (*matchedWhile) return parseWhileStatement();


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

            if (basic::handleAllErrors(parseContainer(*module), [this](const diag::DiagnosticError & error) {
                error.diagnoseInto(*_diagnostics);
            }, [this](const LexerError &) {
                _currentToken->diagnoseInto(*_diagnostics);
            })) {
                return nullptr;
            }

            return module;
        }
    }
}
