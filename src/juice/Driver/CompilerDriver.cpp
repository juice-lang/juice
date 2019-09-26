// src/juice/Driver/CompilerDriver.cpp - Driver subclass for compiling a file
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/Driver/CompilerDriver.h"

#include <memory>
#include <vector>
#include <utility>

#include "juice/Basic/SourceBuffer.h"
#include "juice/Basic/StringRef.h"
#include "juice/Diagnostics/Diagnostics.h"
#include "juice/Parser/Lexer.h"
#include "juice/Parser/LexerToken.h"
#include "juice/Parser/Parser.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Verifier.h"

namespace juice {
    namespace driver {
        static llvm::Function * createFunction(llvm::Type * returnType, const std::vector<llvm::Type *> & params, bool isVarArg, const std::string & name, llvm::Module * module) {
            llvm::FunctionType * type = llvm::FunctionType::get(returnType, params, isVarArg);
            return llvm::Function::Create(type, llvm::Function::ExternalLinkage, name, module);
        }

        static void createExpressionPrintingProgram(std::unique_ptr<parser::ExpressionAST> expression) {
            llvm::LLVMContext context;
            llvm::IRBuilder<> builder(context);

            std::string string = "%f\n";

            auto module = std::make_unique<llvm::Module>("expression", context);

            auto * global = new llvm::GlobalVariable(*module, llvm::ArrayType::get(llvm::Type::getInt8Ty(context), string.size() + 1), true, llvm::GlobalValue::PrivateLinkage, llvm::ConstantDataArray::getString(context, string), ".str");

            llvm::Function * expressionFunction = createFunction(llvm::Type::getDoubleTy(context), {}, false, "expression", module.get());
            llvm::BasicBlock * expressionEntryBlock = llvm::BasicBlock::Create(context, "entry", expressionFunction);
            builder.SetInsertPoint(expressionEntryBlock);

            if (llvm::Value * value = expression->codegen(context, builder)) {
                builder.CreateRet(value);

                llvm::verifyFunction(*expressionFunction);

                llvm::Function * printfFunction = createFunction(llvm::Type::getInt32Ty(context), {llvm::Type::getInt8PtrTy(context)}, true, "printf", module.get());

                llvm::Function * mainFunction = createFunction(llvm::Type::getInt32Ty(context), {}, false, "main", module.get());
                llvm::BasicBlock * mainEntryBlock = llvm::BasicBlock::Create(context, "entry", mainFunction);
                builder.SetInsertPoint(mainEntryBlock);

                llvm::Value * expressionValue = builder.CreateCall(expressionFunction, {}, "expressionCall");
                llvm::Value * stringValue = builder.CreateBitCast(global, llvm::Type::getInt8PtrTy(context), "cast");

                builder.CreateCall(printfFunction, {stringValue, expressionValue}, "printfCall");

                llvm::Value * zero = llvm::ConstantInt::get(context, llvm::APInt(32, 0, true));

                builder.CreateRet(zero);

                llvm::verifyFunction(*mainFunction);

                module->print(llvm::outs(), nullptr);
            } else {
                expressionFunction->eraseFromParent();
            }
        }
        
        int CompilerDriver::execute() {
            basic::StringRef filename(_filename);
            auto buffer = basic::SourceBuffer::getFile(filename);
            if (buffer == nullptr) {
                diag::DiagnosticEngine::diagnose(diag::DiagnosticID::file_not_found, filename);
                return 1;
            }

            auto diagnostics = std::make_shared<diag::DiagnosticEngine>(buffer);

            parser::Parser juiceParser(diagnostics);

            auto expression = juiceParser.parseProgram();

            if (expression != nullptr) {
                createExpressionPrintingProgram(std::move(expression));
            }

            return diagnostics->hadError() ? 1 : 0;
        }
    }
}
