// src/juice/IRGen/GenStatement.cpp - generate LLVM IR from statement AST nodes
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/IRGen/IRGen.h"

#include <utility>

#include "juice/Sema/TypeCheckedDeclarationAST.h"
#include "juice/Sema/TypeCheckedStatementAST.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/Support/Casting.h"

namespace juice {
    namespace irgen {
        void IRGen::generateStatement(std::unique_ptr<sema::TypeCheckedStatementAST> statement) {
            if (llvm::isa<sema::TypeCheckedDeclarationAST>(statement.get())) {
                auto declaration = std::unique_ptr<sema::TypeCheckedDeclarationAST>(
                    llvm::cast<sema::TypeCheckedDeclarationAST>(statement.release()));
                generateDeclaration(std::move(declaration));
            } else {
                switch (statement->_kind) {
                    case sema::TypeCheckedAST::Kind::blockStatement: {
                        auto block = std::unique_ptr<sema::TypeCheckedBlockStatementAST>(
                            llvm::cast<sema::TypeCheckedBlockStatementAST>(statement.release()));
                        generateBlockStatement(std::move(block));
                        break;
                    }
                    case sema::TypeCheckedAST::Kind::expressionStatement: {
                        auto expression = std::unique_ptr<sema::TypeCheckedExpressionStatementAST>(
                            llvm::cast<sema::TypeCheckedExpressionStatementAST>(statement.release()));
                        generateExpressionStatement(std::move(expression));
                        break;
                    }
                    case sema::TypeCheckedAST::Kind::ifStatement: {
                        auto _if = std::unique_ptr<sema::TypeCheckedIfStatementAST>(
                            llvm::cast<sema::TypeCheckedIfStatementAST>(statement.release()));
                        generateIfStatement(std::move(_if));
                        break;
                    }
                    case sema::TypeCheckedAST::Kind::whileStatement: {
                        auto _while = std::unique_ptr<sema::TypeCheckedWhileStatementAST>(
                            llvm::cast<sema::TypeCheckedWhileStatementAST>(statement.release()));
                        generateWhileStatement(std::move(_while));
                        break;
                    }
                    default:
                        llvm_unreachable("All statement AST nodes should be handled here");
                }
            }
        }

        llvm::Value * IRGen::generateYieldingStatement(std::unique_ptr<sema::TypeCheckedStatementAST> statement) {
            switch (statement->_kind) {
                case sema::TypeCheckedAST::Kind::blockStatement: {
                    auto block = std::unique_ptr<sema::TypeCheckedBlockStatementAST>(
                        llvm::cast<sema::TypeCheckedBlockStatementAST>(statement.release()));
                    return generateBlockStatement(std::move(block));
                }
                case sema::TypeCheckedAST::Kind::expressionStatement: {
                    auto expression = std::unique_ptr<sema::TypeCheckedExpressionStatementAST>(
                        llvm::cast<sema::TypeCheckedExpressionStatementAST>(statement.release()));
                    return generateExpressionStatement(std::move(expression));
                }
                default:
                    llvm_unreachable("All yielding statement AST nodes should be handled here");
            }
        }

        llvm::Value * IRGen::generateBlockStatement(std::unique_ptr<sema::TypeCheckedBlockStatementAST> statement) {
            return generateBlock(std::move(statement->_block));
        }

        llvm::Value *
        IRGen::generateExpressionStatement(std::unique_ptr<sema::TypeCheckedExpressionStatementAST> statement) {
            return generateExpression(std::move(statement->_expression));
        }

        void IRGen::generateIfStatement(std::unique_ptr<sema::TypeCheckedIfStatementAST> statement) {
            bool hasElse = (bool)statement->_ifExpression->_elseBody;

            auto ifCondition = generateExpression(std::move(statement->_ifExpression->_ifCondition));

            llvm::Function * function = _builder.GetInsertBlock()->getParent();

            llvm::BasicBlock * ifBlock = llvm::BasicBlock::Create(_context, "if", function);

            std::vector<llvm::BasicBlock *> elifBlocks(statement->_ifExpression->_elifConditionsAndBodies.size() * 2);

            for (int i = 0; i < elifBlocks.size(); ++i) {
                elifBlocks[i] = llvm::BasicBlock::Create(_context, i % 2 == 0 ? "elifcmp" : "elif");
            }

            llvm::BasicBlock * elseBlock = hasElse ? llvm::BasicBlock::Create(_context, "else") : nullptr;
            llvm::BasicBlock * mergeBlock = llvm::BasicBlock::Create(_context, "ifcont");

            if (elifBlocks.empty()) _builder.CreateCondBr(ifCondition, ifBlock, hasElse ? elseBlock : mergeBlock);
            else _builder.CreateCondBr(ifCondition, ifBlock, elifBlocks.front());

            _builder.SetInsertPoint(ifBlock);

            generateControlFlowBody(std::move(statement->_ifExpression->_ifBody));

            _builder.CreateBr(mergeBlock);


            auto elifBlocksIt = elifBlocks.begin();
            auto elifConditionsAndBodiesIt = statement->_ifExpression->_elifConditionsAndBodies.begin();

            while (elifBlocksIt != elifBlocks.end()) {
                auto & compareBlock = *elifBlocksIt;
                auto & block = *(elifBlocksIt + 1);
                auto condition = std::move(std::get<0>(*elifConditionsAndBodiesIt));
                auto body = std::move(std::get<1>(*elifConditionsAndBodiesIt));

                auto nextBlockIt = elifBlocksIt + 2;

                function->getBasicBlockList().push_back(compareBlock);
                _builder.SetInsertPoint(compareBlock);

                auto elifCondition = generateExpression(std::move(condition));

                if (nextBlockIt != elifBlocks.end()) _builder.CreateCondBr(elifCondition, block, *(nextBlockIt));
                else _builder.CreateCondBr(elifCondition, block, hasElse ? elseBlock : mergeBlock);

                compareBlock = _builder.GetInsertBlock();


                function->getBasicBlockList().push_back(block);
                _builder.SetInsertPoint(block);

                generateControlFlowBody(std::move(body));

                _builder.CreateBr(mergeBlock);


                elifBlocksIt += 2;
                ++elifConditionsAndBodiesIt;
            }

            if (hasElse) {
                function->getBasicBlockList().push_back(elseBlock);
                _builder.SetInsertPoint(elseBlock);

                generateControlFlowBody(std::move(statement->_ifExpression->_elseBody));

                _builder.CreateBr(mergeBlock);
            }

            function->getBasicBlockList().push_back(mergeBlock);
            _builder.SetInsertPoint(mergeBlock);
        }

        void IRGen::generateWhileStatement(std::unique_ptr<sema::TypeCheckedWhileStatementAST> statement) {
            llvm::Function * function = _builder.GetInsertBlock()->getParent();

            llvm::BasicBlock * conditionBlock = llvm::BasicBlock::Create(_context, "whilecmp", function);
            llvm::BasicBlock * block = llvm::BasicBlock::Create(_context, "while");
            llvm::BasicBlock * mergeBlock = llvm::BasicBlock::Create(_context, "whilecont");

            _builder.CreateBr(conditionBlock);


            _builder.SetInsertPoint(conditionBlock);

            auto condition = generateExpression(std::move(statement->_condition));

            _builder.CreateCondBr(condition, block, mergeBlock);


            function->getBasicBlockList().push_back(block);
            _builder.SetInsertPoint(block);

            generateControlFlowBody(std::move(statement->_body));

            _builder.CreateBr(conditionBlock);


            function->getBasicBlockList().push_back(mergeBlock);
            _builder.SetInsertPoint(mergeBlock);
        }
    }
}

