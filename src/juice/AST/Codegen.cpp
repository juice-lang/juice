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

#include "juice/AST/CodegenException.h"
#include "juice/Basic/RawStreamHelpers.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/raw_ostream.h"

namespace juice {
    namespace ast {
        Codegen::Codegen(std::unique_ptr<ModuleAST> ast, std::shared_ptr<diag::DiagnosticEngine> diagnostics):
                _ast(std::move(ast)), _diagnostics(std::move(diagnostics)), _builder(_context) {
            _module = std::make_unique<llvm::Module>("expression", _context);
        }

        bool Codegen::generate() {
            std::string string = "%f\n";

            llvm::Function * printfFunction = createFunction(llvm::Type::getInt32Ty(_context),
                                                             {llvm::Type::getInt8PtrTy(_context)}, true, "printf");

            llvm::Function * mainFunction = createFunction(llvm::Type::getInt32Ty(_context), {}, false, "main");
            llvm::BasicBlock * mainEntryBlock = llvm::BasicBlock::Create(_context, "entry", mainFunction);
            _builder.SetInsertPoint(mainEntryBlock);

            try {
                llvm::Value * value = _ast->codegen(*this);

                auto * globalString = _builder.CreateGlobalString(string, ".str");
                llvm::Value * stringValue = _builder.CreateBitCast(globalString, llvm::Type::getInt8PtrTy(_context),
                                                                   "cast");

                _builder.CreateCall(printfFunction, {stringValue, value}, "printfCall");

                llvm::Value * zero = llvm::ConstantInt::get(_context, llvm::APInt(32, 0, true));

                _builder.CreateRet(zero);

                std::string error;
                llvm::raw_string_ostream os(error);

                if (llvm::verifyFunction(*mainFunction, &os)) {
                    os.flush();
                    _diagnostics->diagnose(diag::DiagnosticID::function_verification_error, error);
                } else return true;
            } catch (const CodegenException & e) {
                e.diagnoseInto(_diagnostics);
            }

            return false;
        }

        void Codegen::dumpProgram() {
            llvm::raw_ostream & os = llvm::outs();

            os << basic::Color::bold;
            _module->print(os, nullptr);
            os << basic::Color::reset;
        }

        bool Codegen::newNamedValue(llvm::StringRef name, llvm::AllocaInst * alloca) {
            if (namedValueExists(name)) return false;

            _namedValues[name] = alloca;
            return true;
        }

        bool Codegen::namedValueExists(llvm::StringRef name) const {
            return _namedValues.find(name) != _namedValues.end();
        }

        llvm::AllocaInst * Codegen::getNamedValue(llvm::StringRef name) const {
            return _namedValues.find(name)->second;
        }

        llvm::Function *
        Codegen::createFunction(llvm::Type * returnType, const std::vector<llvm::Type *> & params, bool isVarArg,
                                const std::string & name) {
            llvm::FunctionType * type = llvm::FunctionType::get(returnType, params, isVarArg);
            return llvm::Function::Create(type, llvm::Function::ExternalLinkage, name, _module.get());
        }
    }
}
