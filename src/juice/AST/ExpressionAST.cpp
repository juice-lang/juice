// src/juice/Parser/ExpressionAST.cpp - AST nodes for expression parsing
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/AST/ExpressionAST.h"

#include <functional>
#include <map>

#include "juice/AST/Codegen.h"
#include "juice/AST/CodegenError.h"
#include "juice/Basic/SourceLocation.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/Casting.h"

namespace juice {
    namespace ast {
        ExpressionAST::ExpressionAST(Kind kind, std::unique_ptr<juice::parser::LexerToken> token):
            _kind(kind), _token(std::move(token)) {}

        BinaryOperatorExpressionAST::BinaryOperatorExpressionAST(std::unique_ptr<parser::LexerToken> token,
                                                                 std::unique_ptr<ExpressionAST> left,
                                                                 std::unique_ptr<ExpressionAST> right):
            ExpressionAST(Kind::binaryOperator, std::move(token)), _left(std::move(left)), _right(std::move(right)) {}

        void BinaryOperatorExpressionAST::diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const {
            basic::SourceLocation location(getLocation());

            diagnostics.diagnose(location, diag::DiagnosticID::binary_operator_expression_ast_0, getColor(level),
                                 level, _token.get());
            _left->diagnoseInto(diagnostics, level + 1);

            diagnostics
                .diagnose(location, diag::DiagnosticID::binary_operator_expression_ast_1, getColor(level), level);
            _right->diagnoseInto(diagnostics, level + 1);

            diagnostics.diagnose(location, diag::DiagnosticID::ast_end, getColor(level), level);
        }

        llvm::Expected<llvm::Value *> BinaryOperatorExpressionAST::codegen(Codegen & state) const {
            using namespace std::placeholders;
            using TokenType = parser::LexerToken::Type;
            using Builder = llvm::IRBuilder<>;

            Builder & builder = state.getBuilder();

            std::map<TokenType, std::function<llvm::Value *(llvm::Value *, llvm::Value *)>> assignmentOperators {
                {TokenType::operatorEqual, nullptr},
                {TokenType::operatorPlusEqual, std::bind(&Builder::CreateFAdd, builder, _1, _2, "addtmp", nullptr)},
                {TokenType::operatorMinusEqual, std::bind(&Builder::CreateFSub, builder, _1, _2, "subtmp", nullptr)},
                {TokenType::operatorAsteriskEqual, std::bind(&Builder::CreateFMul, builder, _1, _2, "multmp", nullptr)},
                {TokenType::operatorSlashEqual, std::bind(&Builder::CreateFDiv, builder, _1, _2, "divtmp", nullptr)}
            };

            auto function = assignmentOperators.find(_token->type);

            if (function != assignmentOperators.end()) {
                auto variable = llvm::dyn_cast_or_null<VariableExpressionAST>(_left.get());

                if (variable == nullptr) {
                    return llvm::make_error<CodegenError>(diag::DiagnosticID::expression_not_assignable, getLocation());
                }

                llvm::StringRef name = variable->name();

                auto right = _right->codegen(state);
                if (auto error = right.takeError()) return std::move(error);

                auto rightValue = *right;

                if (rightValue == nullptr) return nullptr;

                llvm::AllocaInst * alloca = state.getNamedValue(name);

                if (function->second != nullptr) {
                    llvm::Value * variableValue = builder.CreateLoad(alloca, name);

                    rightValue = function->second(variableValue, rightValue);
                }

                builder.CreateStore(rightValue, alloca);

                return rightValue;
            }

            auto left = _left->codegen(state);
            if (auto error = left.takeError()) return std::move(error);

            auto right = _right->codegen(state);
            if (auto error = right.takeError()) return std::move(error);

            if (*left == nullptr || *right == nullptr) return nullptr;

            switch (_token->type) {
                case parser::LexerToken::Type::operatorPlus:
                    return builder.CreateFAdd(*left, *right, "addtmp");
                case parser::LexerToken::Type::operatorMinus:
                    return builder.CreateFSub(*left, *right, "subtmp");
                case parser::LexerToken::Type::operatorAsterisk:
                    return builder.CreateFMul(*left, *right, "multmp");
                case parser::LexerToken::Type::operatorSlash:
                    return builder.CreateFDiv(*left, *right, "divtmp");
                default:
                    return nullptr;
            }
        }

        NumberExpressionAST::NumberExpressionAST(std::unique_ptr<parser::LexerToken> token, double value):
                ExpressionAST(Kind::number, std::move(token)), _value(value) {}

        void NumberExpressionAST::diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const {
            diagnostics.diagnose(getLocation(), diag::DiagnosticID::number_expression_ast, getColor(level), level,
                                 _token.get(), _value);
        }

        llvm::Expected<llvm::Value *> NumberExpressionAST::codegen(Codegen & state) const {
            return llvm::ConstantFP::get(state.getContext(), llvm::APFloat(_value));
        }

        VariableExpressionAST::VariableExpressionAST(std::unique_ptr<parser::LexerToken> token):
                ExpressionAST(Kind::variable, std::move(token)) {}

        void VariableExpressionAST::diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const {
            diagnostics.diagnose(getLocation(), diag::DiagnosticID::variable_expression_ast, getColor(level),
                                 _token.get());
        }

        llvm::Expected<llvm::Value *> VariableExpressionAST::codegen(Codegen & state) const {
            if (state.namedValueExists(name())) {
                llvm::AllocaInst * alloca = state.getNamedValue(name());

                return state.getBuilder().CreateLoad(alloca, name());
            } else {
                return llvm::make_error<CodegenErrorWithString>(diag::DiagnosticID::unresolved_identifer,
                                                                getLocation(), name());
            }
        }

        GroupingExpressionAST::GroupingExpressionAST(std::unique_ptr<parser::LexerToken> token,
                                                     std::unique_ptr<ExpressionAST> expression):
                ExpressionAST(Kind::grouping, std::move(token)), _expression(std::move(expression)) {}

        void GroupingExpressionAST::diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const {
            _expression->diagnoseInto(diagnostics, level);
        }

        llvm::Expected<llvm::Value *> GroupingExpressionAST::codegen(Codegen & state) const {
            return _expression->codegen(state);
        }

        IfExpressionAST::IfExpressionAST(std::unique_ptr<ExpressionAST> ifCondition,
                                         std::unique_ptr<IfBodyAST> ifBody,
                                         std::vector<std::pair<std::unique_ptr<ExpressionAST>,
                                                               std::unique_ptr<IfBodyAST>>> && elifConditionsAndBodies,
                                         std::unique_ptr<IfBodyAST> elseBody, bool isStatement):
            ExpressionAST(Kind::_if, nullptr), _ifCondition(std::move(ifCondition)), _ifBody(std::move(ifBody)),
            _elifConditionsAndBodies(std::move(elifConditionsAndBodies)), _elseBody(std::move(elseBody)),
            _isStatement(isStatement) {}

        void IfExpressionAST::diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const {
            basic::SourceLocation location(getLocation());

            if (_isStatement) diagnostics.diagnose(location, diag::DiagnosticID::if_statement_ast_0, getColor(level),
                                                   level, _ifBody->getKeyword().get());
            else diagnostics.diagnose(location, diag::DiagnosticID::if_expression_ast_0, getColor(level), level,
                                 _ifBody->getKeyword().get());
            _ifCondition->diagnoseInto(diagnostics, level + 1);

            diagnostics.diagnose(location, diag::DiagnosticID::if_ast_1, getColor(level), level);
            _ifBody->diagnoseInto(diagnostics, level + 1);

            for (const auto & conditionAndBody: _elifConditionsAndBodies) {
                const auto & condition = std::get<0>(conditionAndBody);
                const auto & body = std::get<1>(conditionAndBody);

                diagnostics.diagnose(location, diag::DiagnosticID::if_ast_2, getColor(level), level);
                condition->diagnoseInto(diagnostics, level + 1);

                diagnostics.diagnose(location, diag::DiagnosticID::if_ast_3, getColor(level), level);
                body->diagnoseInto(diagnostics, level + 1);
            }

            if (!_isStatement || _elseBody) {
                diagnostics.diagnose(location, diag::DiagnosticID::if_ast_4, getColor(level), level);
                _elseBody->diagnoseInto(diagnostics, level + 1);
            }

            diagnostics.diagnose(location, diag::DiagnosticID::ast_end, getColor(level), level);
        }

        llvm::Expected<llvm::Value *> IfExpressionAST::codegen(Codegen & state) const {
            llvm::IRBuilder<> & builder = state.getBuilder();

            auto ifCondition = _ifCondition->codegen(state);
            if (auto error = ifCondition.takeError()) return std::move(error);

            auto ifConditionValue = *ifCondition;

            if (!ifConditionValue) return nullptr;

            ifConditionValue = builder.CreateFCmpONE(ifConditionValue,
                                                     llvm::ConstantFP::get(state.getContext(), llvm::APFloat(0.0)),
                                                     "ifcond");

            llvm::Function * function = builder.GetInsertBlock()->getParent();

            llvm::BasicBlock * ifBlock = llvm::BasicBlock::Create(state.getContext(), "then", function);

            std::vector<llvm::BasicBlock *> elifBlocks(_elifConditionsAndBodies.size() * 2);

            for (int i = 0; i < elifBlocks.size(); ++i) {
                elifBlocks[i] = llvm::BasicBlock::Create(state.getContext(), i % 2 == 0 ? "elifcmp" : "elif");
            }

            llvm::BasicBlock * elseBlock = llvm::BasicBlock::Create(state.getContext(), "else");
            llvm::BasicBlock * mergeBlock = llvm::BasicBlock::Create(state.getContext(), "ifcont");

            if (elifBlocks.empty()) builder.CreateCondBr(ifConditionValue, ifBlock, elseBlock);
            else builder.CreateCondBr(ifConditionValue, ifBlock, elifBlocks.front());

            builder.SetInsertPoint(ifBlock);

            auto ifValue = _ifBody->codegen(state);
            if (auto error = ifValue.takeError()) return std::move(error);

            if (!*ifValue) {
                return llvm::make_error<CodegenErrorWithString>(diag::DiagnosticID::expected_block_yield,
                                                                _ifBody->getLocation(), "then");
            }

            builder.CreateBr(mergeBlock);

            ifBlock = builder.GetInsertBlock();


            std::vector<llvm::Value *> elifValues(_elifConditionsAndBodies.size());

            auto elifBlocksIt = elifBlocks.begin();
            auto elifConditionsAndBodiesIt = _elifConditionsAndBodies.begin();
            auto elifValuesIt = elifValues.begin();

            while (elifBlocksIt != elifBlocks.end()) {
                auto & compareBlock = *elifBlocksIt;
                auto & block = *(elifBlocksIt + 1);
                const auto & condition = std::get<0>(*elifConditionsAndBodiesIt);
                const auto & body = std::get<1>(*elifConditionsAndBodiesIt);

                auto nextBlockIt = elifBlocksIt + 2;

                function->getBasicBlockList().push_back(compareBlock);
                builder.SetInsertPoint(compareBlock);

                auto elifCondition = condition->codegen(state);
                if (auto error = elifCondition.takeError()) return std::move(error);

                auto elifConditionValue = *elifCondition;

                if (!elifConditionValue) return nullptr;

                elifConditionValue = builder.CreateFCmpONE(elifConditionValue,
                                                           llvm::ConstantFP::get(state.getContext(),
                                                                                 llvm::APFloat(0.0)),
                                                           "elifcond");

                if (nextBlockIt != elifBlocks.end()) builder.CreateCondBr(elifConditionValue, block, *(nextBlockIt));
                else builder.CreateCondBr(elifConditionValue, block, elseBlock);

                compareBlock = builder.GetInsertBlock();


                function->getBasicBlockList().push_back(block);
                builder.SetInsertPoint(block);

                auto elifValue = body->codegen(state);
                if (auto error = elifValue.takeError()) return std::move(error);

                if (!*elifValue) {
                    return llvm::make_error<CodegenErrorWithString>(diag::DiagnosticID::expected_block_yield,
                                                                    body->getLocation(), "elif");
                }

                *elifValuesIt = *elifValue;

                builder.CreateBr(mergeBlock);

                block = builder.GetInsertBlock();


                elifBlocksIt += 2;
                ++elifConditionsAndBodiesIt;
                ++elifValuesIt;
            }


            function->getBasicBlockList().push_back(elseBlock);
            builder.SetInsertPoint(elseBlock);

            auto elseValue = _elseBody->codegen(state);
            if (auto error = elseValue.takeError()) return std::move(error);

            if (!*elseValue) {
                return llvm::make_error<CodegenErrorWithString>(diag::DiagnosticID::expected_block_yield,
                                                                _elseBody->getLocation(), "else");
            }

            builder.CreateBr(mergeBlock);

            elseBlock = builder.GetInsertBlock();

            function->getBasicBlockList().push_back(mergeBlock);
            builder.SetInsertPoint(mergeBlock);

            llvm::PHINode * phi = builder.CreatePHI(llvm::Type::getDoubleTy(state.getContext()),
                                                    2 + _elifConditionsAndBodies.size(), "iftmp");
            phi->addIncoming(*ifValue, ifBlock);

            elifBlocksIt = elifBlocks.begin();
            elifValuesIt = elifValues.begin();

            while (elifBlocksIt != elifBlocks.end()) {
                const auto & block = *(elifBlocksIt + 1);

                phi->addIncoming(*elifValuesIt, block);

                elifBlocksIt += 2;
                ++elifValuesIt;
            }

            phi->addIncoming(*elseValue, elseBlock);

            return phi;
        }
    }
}
