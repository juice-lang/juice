// include/juice/IRGen/IRGen.h - class for generating LLVM IR
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_IRGEN_IRGEN_H
#define JUICE_IRGEN_IRGEN_H

#include <memory>
#include <vector>

#include "juice/Diagnostics/Diagnostics.h"
#include "juice/Sema/TypeChecker.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/raw_ostream.h"

namespace juice {
    namespace sema {
        class TypeCheckedModuleAST;
        class TypeCheckedBlockAST;
        class TypeCheckedControlFlowBodyAST;
        class TypeCheckedDeclarationAST;
        class TypeCheckedVariableDeclarationAST;
        class TypeCheckedExpressionAST;
        class TypeCheckedBinaryOperatorExpressionAST;
        class TypeCheckedNumberExpressionAST;
        class TypeCheckedVariableExpressionAST;
        class TypeCheckedGroupingExpressionAST;
        class TypeCheckedIfExpressionAST;
        class TypeCheckedStatementAST;
        class TypeCheckedBlockStatementAST;
        class TypeCheckedExpressionStatementAST;
        class TypeCheckedIfStatementAST;
        class TypeCheckedWhileStatementAST;
    }

    namespace irgen {
        class IRGen {
            std::unique_ptr<sema::TypeCheckedModuleAST> _ast;

            std::shared_ptr<diag::DiagnosticEngine> _diagnostics;

            llvm::LLVMContext _context;
            llvm::IRBuilder<> _builder;
            std::unique_ptr<llvm::Module> _module;

            std::vector<llvm::AllocaInst *> _allocas;

        public:
            IRGen() = delete;

            IRGen(sema::TypeChecker::Result typeCheckResult, std::shared_ptr<diag::DiagnosticEngine> diagnostics);

            bool generate();
            void dumpProgram(llvm::raw_ostream & os);

            bool emitObject(llvm::raw_pwrite_stream & os);

        private:
            llvm::Value * generateModule();

            llvm::Value * generateBlock(std::unique_ptr<sema::TypeCheckedBlockAST> block);

            llvm::Value * generateControlFlowBody(std::unique_ptr<sema::TypeCheckedControlFlowBodyAST> body);


            void generateDeclaration(std::unique_ptr<sema::TypeCheckedDeclarationAST> declaration);

            void generateVariableDeclaration(std::unique_ptr<sema::TypeCheckedVariableDeclarationAST> declaration);


            llvm::Value * generateExpression(std::unique_ptr<sema::TypeCheckedExpressionAST> expression);

            llvm::Value *
            generateBinaryOperatorExpression(std::unique_ptr<sema::TypeCheckedBinaryOperatorExpressionAST> expression);

            llvm::Value * generateNumberExpression(std::unique_ptr<sema::TypeCheckedNumberExpressionAST> expression);

            llvm::Value *
            generateVariableExpression(std::unique_ptr<sema::TypeCheckedVariableExpressionAST> expression);

            llvm::Value *
            generateGroupingExpression(std::unique_ptr<sema::TypeCheckedGroupingExpressionAST> expression);

            llvm::Value * generateIfExpression(std::unique_ptr<sema::TypeCheckedIfExpressionAST> expression);


            void generateStatement(std::unique_ptr<sema::TypeCheckedStatementAST> statement);

            llvm::Value * generateYieldingStatement(std::unique_ptr<sema::TypeCheckedStatementAST> statement);

            llvm::Value * generateBlockStatement(std::unique_ptr<sema::TypeCheckedBlockStatementAST> statement);

            llvm::Value *
            generateExpressionStatement(std::unique_ptr<sema::TypeCheckedExpressionStatementAST> statement);

            void generateIfStatement(std::unique_ptr<sema::TypeCheckedIfStatementAST> statement);

            void generateWhileStatement(std::unique_ptr<sema::TypeCheckedWhileStatementAST> statement);


            llvm::Function *
            createFunction(llvm::Type * returnType, const std::vector<llvm::Type *> & params, bool isVarArg,
                           llvm::StringRef name);
        };
    }
}

#endif //JUICE_IRGEN_IRGEN_H
