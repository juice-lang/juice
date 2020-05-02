// src/juice/IRGen/GenExpression.cpp -
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/IRGen/IRGen.h"

#include <functional>
#include <map>
#include <utility>

#include "juice/Sema/TypeCheckedExpressionAST.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/Support/Casting.h"

namespace juice {
    namespace irgen {
        llvm::Value * IRGen::generateExpression(std::unique_ptr<sema::TypeCheckedExpressionAST> expression) {
            switch (expression->_kind) {
                case sema::TypeCheckedAST::Kind::binaryOperatorExpression: {
                    auto binaryOperator = std::unique_ptr<sema::TypeCheckedBinaryOperatorExpressionAST>(
                        llvm::cast<sema::TypeCheckedBinaryOperatorExpressionAST>(expression.release()));
                    return generateBinaryOperatorExpression(std::move(binaryOperator));
                }
                case sema::TypeCheckedAST::Kind::numberExpression: {
                    auto number = std::unique_ptr<sema::TypeCheckedNumberExpressionAST>(
                        llvm::cast<sema::TypeCheckedNumberExpressionAST>(expression.release()));
                    return generateNumberExpression(std::move(number));
                }
                case sema::TypeCheckedAST::Kind::variableExpression: {
                    auto variable = std::unique_ptr<sema::TypeCheckedVariableExpressionAST>(
                        llvm::cast<sema::TypeCheckedVariableExpressionAST>(expression.release()));
                    return generateVariableExpression(std::move(variable));
                }
                case sema::TypeCheckedAST::Kind::groupingExpression: {
                    auto grouping = std::unique_ptr<sema::TypeCheckedGroupingExpressionAST>(
                        llvm::cast<sema::TypeCheckedGroupingExpressionAST>(expression.release()));
                    return generateGroupingExpression(std::move(grouping));
                }
                case sema::TypeCheckedAST::Kind::ifExpression: {
                    auto _if = std::unique_ptr<sema::TypeCheckedIfExpressionAST>(
                        llvm::cast<sema::TypeCheckedIfExpressionAST>(expression.release()));
                    return generateIfExpression(std::move(_if));
                }
                default:
                    llvm_unreachable("All expression AST nodes should be handled here");
            }
        }

        llvm::Value * IRGen::generateBinaryOperatorExpression(
            std::unique_ptr<sema::TypeCheckedBinaryOperatorExpressionAST> expression) {
            using namespace std::placeholders;
            using TokenType = parser::LexerToken::Type;
            using Builder = llvm::IRBuilder<>;

            std::map<TokenType, std::function<llvm::Value *(llvm::Value *, llvm::Value *)>> assignmentOperators {
                {TokenType::operatorEqual, nullptr},
                {TokenType::operatorPlusEqual, [&](auto l, auto r) { return _builder.CreateFAdd(l, r, "addtmp"); }},
                {TokenType::operatorMinusEqual, [&](auto l, auto r) { return _builder.CreateFSub(l, r, "subtmp"); }},
                {TokenType::operatorAsteriskEqual, [&](auto l, auto r) { return _builder.CreateFMul(l, r, "multmp"); }},
                {TokenType::operatorSlashEqual, [&](auto l, auto r) { return _builder.CreateFDiv(l, r, "divtmp"); }}
            };

            auto instruction = assignmentOperators.find(expression->_token->type);

            if (instruction != assignmentOperators.end()) {
                const auto & variable = llvm::cast<sema::TypeCheckedVariableExpressionAST>(*expression->_left);

                llvm::StringRef name = variable.name();

                auto right = generateExpression(std::move(expression->_right));

                llvm::AllocaInst * alloca = _allocas.at(variable._index);

                if (instruction->second != nullptr) {
                    llvm::Value * variableValue = _builder.CreateLoad(alloca, name);

                    right = instruction->second(variableValue, right);
                }

                _builder.CreateStore(right, alloca);

                return right;
            }

            auto left = generateExpression(std::move(expression->_left));

            if (expression->_token->type == TokenType::operatorAndAnd
                || expression->_token->type == TokenType::operatorPipePipe) {

                llvm::Function * function = _builder.GetInsertBlock()->getParent();

                llvm::BasicBlock * rightBlock = llvm::BasicBlock::Create(_context, "logical", function);
                llvm::BasicBlock * mergeBlock = llvm::BasicBlock::Create(_context, "logicalcont");

                if (expression->_token->type == TokenType::operatorAndAnd)
                    _builder.CreateCondBr(left, rightBlock, mergeBlock);
                else _builder.CreateCondBr(left, mergeBlock, rightBlock);

                llvm::BasicBlock * leftBlock = _builder.GetInsertBlock();


                _builder.SetInsertPoint(rightBlock);

                auto right = generateExpression(std::move(expression->_right));

                _builder.CreateBr(mergeBlock);

                rightBlock = _builder.GetInsertBlock();


                function->getBasicBlockList().push_back(mergeBlock);
                _builder.SetInsertPoint(mergeBlock);

                llvm::PHINode * phi = _builder.CreatePHI(llvm::Type::getInt1Ty(_context), 2, "logicaltmp");
                phi->addIncoming(left, leftBlock);
                phi->addIncoming(right, rightBlock);

                return phi;
            }

            auto right = generateExpression(std::move(expression->_right));

            switch (expression->_token->type) {
                case TokenType::operatorPlus:
                    return _builder.CreateFAdd(left, right, "addtmp");
                case TokenType::operatorMinus:
                    return _builder.CreateFSub(left, right, "subtmp");
                case TokenType::operatorAsterisk:
                    return _builder.CreateFMul(left, right, "multmp");
                case TokenType::operatorSlash:
                    return _builder.CreateFDiv(left, right, "divtmp");
                case TokenType::operatorEqualEqual:
                    return _builder.CreateFCmpOEQ(left, right, "eqtmp");
                case TokenType::operatorBangEqual:
                    return _builder.CreateFCmpONE(left, right, "netmp");
                case TokenType::operatorLower:
                    return _builder.CreateFCmpOLT(left, right, "lttmp");
                case TokenType::operatorLowerEqual:
                    return _builder.CreateFCmpOLE(left, right, "letmp");
                case TokenType::operatorGreater:
                    return _builder.CreateFCmpOGT(left, right, "gttmp");
                case TokenType::operatorGreaterEqual:
                    return _builder.CreateFCmpOGE(left, right, "getmp");
                default:
                    llvm_unreachable("All possible parsed operators should be handled here");
            }
        }

        llvm::Value *
        IRGen::generateNumberExpression(std::unique_ptr<sema::TypeCheckedNumberExpressionAST> expression) {
            return llvm::ConstantFP::get(_context, llvm::APFloat(expression->_value));
        }

        llvm::Value *
        IRGen::generateVariableExpression(std::unique_ptr<sema::TypeCheckedVariableExpressionAST> expression) {
            llvm::AllocaInst * alloca = _allocas.at(expression->_index);

            return _builder.CreateLoad(alloca, expression->name());
        }

        llvm::Value *
        IRGen::generateGroupingExpression(std::unique_ptr<sema::TypeCheckedGroupingExpressionAST> expression) {
            return generateExpression(std::move(expression->_expression));
        }

        llvm::Value * IRGen::generateIfExpression(std::unique_ptr<sema::TypeCheckedIfExpressionAST> expression) {
            auto ifCondition = generateExpression(std::move(expression->_ifCondition));

            llvm::Function * function = _builder.GetInsertBlock()->getParent();

            llvm::BasicBlock * ifBlock = llvm::BasicBlock::Create(_context, "if", function);

            std::vector<llvm::BasicBlock *> elifBlocks(expression->_elifConditionsAndBodies.size() * 2);

            for (int i = 0; i < elifBlocks.size(); ++i) {
                elifBlocks[i] = llvm::BasicBlock::Create(_context, i % 2 == 0 ? "elifcmp" : "elif");
            }

            llvm::BasicBlock * elseBlock = llvm::BasicBlock::Create(_context, "else");
            llvm::BasicBlock * mergeBlock = llvm::BasicBlock::Create(_context, "ifcont");

            if (elifBlocks.empty()) _builder.CreateCondBr(ifCondition, ifBlock, elseBlock);
            else _builder.CreateCondBr(ifCondition, ifBlock, elifBlocks.front());

            _builder.SetInsertPoint(ifBlock);

            auto ifValue = generateControlFlowBody(std::move(expression->_ifBody));

            _builder.CreateBr(mergeBlock);

            ifBlock = _builder.GetInsertBlock();


            std::vector<llvm::Value *> elifValues(expression->_elifConditionsAndBodies.size());

            auto elifBlocksIt = elifBlocks.begin();
            auto elifConditionsAndBodiesIt = expression->_elifConditionsAndBodies.begin();
            auto elifValuesIt = elifValues.begin();

            while (elifBlocksIt != elifBlocks.end()) {
                auto & compareBlock = *elifBlocksIt;
                auto & block = *(elifBlocksIt + 1);
                auto condition = std::move(std::get<0>(*elifConditionsAndBodiesIt));
                auto body = std::move(std::get<1>(*elifConditionsAndBodiesIt));

                auto nextBlockIt = elifBlocksIt + 2;

                function->getBasicBlockList().push_back(compareBlock);
                _builder.SetInsertPoint(compareBlock);

                auto elifCondition = generateExpression(std::move(condition));

                if (nextBlockIt != elifBlocks.end()) _builder.CreateCondBr(elifCondition, block, *nextBlockIt);
                else _builder.CreateCondBr(elifCondition, block, elseBlock);

                compareBlock = _builder.GetInsertBlock();


                function->getBasicBlockList().push_back(block);
                _builder.SetInsertPoint(block);

                auto elifValue = generateControlFlowBody(std::move(body));

                *elifValuesIt = elifValue;

                _builder.CreateBr(mergeBlock);

                block = _builder.GetInsertBlock();


                elifBlocksIt += 2;
                ++elifConditionsAndBodiesIt;
                ++elifValuesIt;
            }


            function->getBasicBlockList().push_back(elseBlock);
            _builder.SetInsertPoint(elseBlock);

            auto elseValue = generateControlFlowBody(std::move(expression->_elseBody));

            _builder.CreateBr(mergeBlock);

            elseBlock = _builder.GetInsertBlock();

            function->getBasicBlockList().push_back(mergeBlock);
            _builder.SetInsertPoint(mergeBlock);

            llvm::PHINode * phi = _builder.CreatePHI(llvm::Type::getDoubleTy(_context),
                                                     2 + expression->_elifConditionsAndBodies.size(), "iftmp");
            phi->addIncoming(ifValue, ifBlock);

            elifBlocksIt = elifBlocks.begin();
            elifValuesIt = elifValues.begin();

            while (elifBlocksIt != elifBlocks.end()) {
                const auto & block = *(elifBlocksIt + 1);

                phi->addIncoming(*elifValuesIt, block);

                elifBlocksIt += 2;
                ++elifValuesIt;
            }

            phi->addIncoming(elseValue, elseBlock);

            return phi;
        }
    }
}
