// src/juice/AST/Codegen.cpp - class for first dummy generation of code
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors

#include "juice/AST/Codegen.h"

#include <string>

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/raw_ostream.h"

namespace juice {
    namespace ast {
        Codegen::Codegen(std::unique_ptr<ModuleAST> ast):
                _ast(std::move(ast)), _builder(_context) {
            _module = std::make_unique<llvm::Module>("expression", _context);
        }

        bool Codegen::generate(llvm::raw_string_ostream & os) {
            std::string string = "%f\n";

            llvm::Function * printfFunction = createFunction(llvm::Type::getInt32Ty(_context), {llvm::Type::getInt8PtrTy(_context)}, true, "printf");

            llvm::Function * mainFunction = createFunction(llvm::Type::getInt32Ty(_context), {}, false, "main");
            llvm::BasicBlock * mainEntryBlock = llvm::BasicBlock::Create(_context, "entry", mainFunction);
            _builder.SetInsertPoint(mainEntryBlock);

            if (llvm::Value * value = _ast->codegen(_context, _builder)) {
                auto * global = _builder.CreateGlobalString(string, ".str");

                llvm::Value * stringValue = _builder.CreateBitCast(global, llvm::Type::getInt8PtrTy(_context), "cast");

                _builder.CreateCall(printfFunction, {stringValue, value}, "printfCall");

                llvm::Value * zero = llvm::ConstantInt::get(_context, llvm::APInt(32, 0, true));

                _builder.CreateRet(zero);

                return !llvm::verifyFunction(*mainFunction, &os);
            } else {
                os << "could not generate expression code";

                return false;
            }
        }

        void Codegen::dumpProgram(llvm::raw_ostream & os) {
            _module->print(os, nullptr);
        }

        llvm::Function *
        Codegen::createFunction(llvm::Type * returnType, const std::vector<llvm::Type *> & params, bool isVarArg,
                                const std::string & name) {
            llvm::FunctionType * type = llvm::FunctionType::get(returnType, params, isVarArg);
            return llvm::Function::Create(type, llvm::Function::ExternalLinkage, name, _module.get());
        }
    }
}
