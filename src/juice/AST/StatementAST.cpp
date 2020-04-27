// src/juice/AST/StatementAST.cpp - AST nodes for statement parsing
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors

#include "juice/AST/StatementAST.h"

#include <utility>

#include "juice/AST/Codegen.h"
#include "juice/AST/CodegenError.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"

namespace juice {
    namespace ast {
        BlockStatementAST::BlockStatementAST(std::unique_ptr<BlockAST> block): _block(std::move(block)) {}

        void BlockStatementAST::diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const {
            _block->diagnoseInto(diagnostics, level);
        }

        llvm::Expected<llvm::Value *> BlockStatementAST::codegen(Codegen & state) const {
            return _block->codegen(state);
        }

        ExpressionStatementAST::ExpressionStatementAST(std::unique_ptr<ExpressionAST> expression):
                _expression(std::move(expression)) {}

        void ExpressionStatementAST::diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const {
            _expression->diagnoseInto(diagnostics, level);
        }

        llvm::Expected<llvm::Value *> ExpressionStatementAST::codegen(Codegen & state) const {
            return _expression->codegen(state);
        }

        IfStatementAST::IfStatementAST(std::unique_ptr<IfExpressionAST> ifExpression):
            _ifExpression(std::move(ifExpression)) {}

        void IfStatementAST::diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const {
            _ifExpression->diagnoseInto(diagnostics, level);
        }

        llvm::Expected<llvm::Value *> IfStatementAST::codegen(Codegen & state) const {
            llvm::IRBuilder<> & builder = state.getBuilder();

            bool hasElse = static_cast<bool>(_ifExpression->_elseBody);

            auto ifCondition = _ifExpression->_ifCondition->codegen(state);
            if (auto error = ifCondition.takeError()) return std::move(error);

            auto ifConditionValue = *ifCondition;
            assert(ifConditionValue && "The if condition should be a valid expression");

            ifConditionValue = builder.CreateFCmpONE(ifConditionValue,
                                                     llvm::ConstantFP::get(state.getContext(), llvm::APFloat(0.0)),
                                                     "ifcond");

            llvm::Function * function = builder.GetInsertBlock()->getParent();

            llvm::BasicBlock * ifBlock = llvm::BasicBlock::Create(state.getContext(), "if", function);

            std::vector<llvm::BasicBlock *> elifBlocks(_ifExpression->_elifConditionsAndBodies.size() * 2);

            for (int i = 0; i < elifBlocks.size(); ++i) {
                elifBlocks[i] = llvm::BasicBlock::Create(state.getContext(), i % 2 == 0 ? "elifcmp" : "elif");
            }

            llvm::BasicBlock * elseBlock = hasElse ? llvm::BasicBlock::Create(state.getContext(), "else") : nullptr;
            llvm::BasicBlock * mergeBlock = llvm::BasicBlock::Create(state.getContext(), "ifcont");

            if (elifBlocks.empty())
                builder.CreateCondBr(ifConditionValue, ifBlock, hasElse ? elseBlock : mergeBlock);
            else builder.CreateCondBr(ifConditionValue, ifBlock, elifBlocks.front());

            builder.SetInsertPoint(ifBlock);

            auto ifValue = _ifExpression->_ifBody->codegen(state);
            if (auto error = ifValue.takeError()) return std::move(error);

            builder.CreateBr(mergeBlock);

            ifBlock = builder.GetInsertBlock();


            auto elifBlocksIt = elifBlocks.begin();
            auto elifConditionsAndBodiesIt = _ifExpression->_elifConditionsAndBodies.begin();

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
                assert(elifConditionValue && "The elif condition should be a valid expression");

                elifConditionValue = builder.CreateFCmpONE(elifConditionValue,
                                                           llvm::ConstantFP::get(state.getContext(),
                                                                                 llvm::APFloat(0.0)),
                                                           "elifcond");

                if (nextBlockIt != elifBlocks.end()) builder.CreateCondBr(elifConditionValue, block, *(nextBlockIt));
                else builder.CreateCondBr(elifConditionValue, block, hasElse ? elseBlock : mergeBlock);

                compareBlock = builder.GetInsertBlock();


                function->getBasicBlockList().push_back(block);
                builder.SetInsertPoint(block);

                auto elifValue = body->codegen(state);
                if (auto error = elifValue.takeError()) return std::move(error);

                builder.CreateBr(mergeBlock);

                block = builder.GetInsertBlock();


                elifBlocksIt += 2;
                ++elifConditionsAndBodiesIt;
            }

            if (hasElse) {
                function->getBasicBlockList().push_back(elseBlock);
                builder.SetInsertPoint(elseBlock);

                auto elseValue = _ifExpression->_elseBody->codegen(state);
                if (auto error = elseValue.takeError()) return std::move(error);

                builder.CreateBr(mergeBlock);

                elseBlock = builder.GetInsertBlock();
            }

            function->getBasicBlockList().push_back(mergeBlock);
            builder.SetInsertPoint(mergeBlock);


            return nullptr;
        }

        WhileStatementAST::WhileStatementAST(std::unique_ptr<ExpressionAST> condition,
                                             std::unique_ptr<ControlFlowBodyAST> body):
            _condition(std::move(condition)), _body(std::move(body)) {}

        void WhileStatementAST::diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const {
            basic::SourceLocation location(getLocation());

            diagnostics.diagnose(location, diag::DiagnosticID::while_statement_ast_0, getColor(level), level,
                                 _body->getKeyword().get());
            _condition->diagnoseInto(diagnostics, level + 1);

            diagnostics.diagnose(location, diag::DiagnosticID::while_statement_ast_1, getColor(level), level);
            _body->diagnoseInto(diagnostics, level + 1);

            diagnostics.diagnose(location, diag::DiagnosticID::ast_end, getColor(level), level);
        }

        llvm::Expected<llvm::Value *> WhileStatementAST::codegen(Codegen & state) const {
            llvm::IRBuilder<> & builder = state.getBuilder();

            llvm::Function * function = builder.GetInsertBlock()->getParent();

            llvm::BasicBlock * conditionBlock = llvm::BasicBlock::Create(state.getContext(), "whilecmp", function);
            llvm::BasicBlock * block = llvm::BasicBlock::Create(state.getContext(), "while");
            llvm::BasicBlock * mergeBlock = llvm::BasicBlock::Create(state.getContext(), "whilecont");

            builder.CreateBr(conditionBlock);


            builder.SetInsertPoint(conditionBlock);

            auto condition = _condition->codegen(state);
            if (auto error = condition.takeError()) return std::move(error);

            auto conditionValue = *condition;
            assert(conditionValue && "The while condition should be a valid expression");

            conditionValue = builder.CreateFCmpONE(conditionValue,
                                                   llvm::ConstantFP::get(state.getContext(), llvm::APFloat(0.0)),
                                                   "whilecond");

            builder.CreateCondBr(conditionValue, block, mergeBlock);

            conditionBlock = builder.GetInsertBlock();


            function->getBasicBlockList().push_back(block);
            builder.SetInsertPoint(block);

            auto value = _body->codegen(state);
            if (auto error = value.takeError()) return std::move(error);

            builder.CreateBr(conditionBlock);

            block = builder.GetInsertBlock();


            function->getBasicBlockList().push_back(mergeBlock);
            builder.SetInsertPoint(mergeBlock);

            return nullptr;
        }
    }
}
