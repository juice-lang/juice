// src/juice/AST/Codegen.cpp - class for first dummy generation of code
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors

#include "juice/AST/Codegen.h"

#include <string>

#include "juice/AST/CodegenError.h"
#include "juice/Basic/Error.h"
#include "juice/Basic/RawStreamHelpers.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/raw_ostream.h"

namespace juice {
    namespace ast {
        Scope::Scope(std::unique_ptr<Scope> parent): parent(std::move(parent)) {}

        bool Scope::newNamedValue(llvm::StringRef name, llvm::AllocaInst * alloca) {
            if (namedValues.find(name) != namedValues.end()) return false;

            namedValues[name] = alloca;
            return true;
        }

        bool Scope::namedValueExists(llvm::StringRef name) const {
            return namedValues.find(name) != namedValues.end() ||
                   (parent != nullptr ? parent->namedValueExists(name) : false);
        }

        llvm::AllocaInst * Scope::getNamedValue(llvm::StringRef name) const {
            return namedValues.find(name) != namedValues.end() ? namedValues.find(name)->second :
                   (parent != nullptr ? parent->getNamedValue(name) : nullptr);
        }

        Codegen::Codegen(std::unique_ptr<ModuleAST> ast, std::shared_ptr<diag::DiagnosticEngine> diagnostics):
                _ast(std::move(ast)), _diagnostics(std::move(diagnostics)), _builder(_context) {
            _module = std::make_unique<llvm::Module>("expression", _context);
            _currentScope = std::make_unique<Scope>();
        }

        bool Codegen::generate() {
            std::string string = "%f\n";

            llvm::Function * printfFunction = createFunction(llvm::Type::getInt32Ty(_context),
                                                             {llvm::Type::getInt8PtrTy(_context)}, true, "printf");

            llvm::Function * mainFunction = createFunction(llvm::Type::getInt32Ty(_context), {}, false, "main");
            llvm::BasicBlock * mainEntryBlock = llvm::BasicBlock::Create(_context, "entry", mainFunction);
            _builder.SetInsertPoint(mainEntryBlock);

            auto value = _ast->codegen(*this);

            if (auto error = llvm::handleErrors(value.takeError(), [this](const CodegenError & error) {
                    error.diagnoseInto(_diagnostics);
                    return llvm::make_error<basic::ReturningError>();
                })) {
                llvm::consumeError(std::move(error));
                return false;
            }

            auto * globalString = _builder.CreateGlobalString(string, ".str");
            llvm::Value * stringValue = _builder.CreateBitCast(globalString, llvm::Type::getInt8PtrTy(_context),
                                                               "cast");

            _builder.CreateCall(printfFunction, {stringValue, *value}, "printfCall");

            llvm::Value * zero = llvm::ConstantInt::get(_context, llvm::APInt(32, 0, true));

            _builder.CreateRet(zero);

            std::string error;
            llvm::raw_string_ostream os(error);

            if (llvm::verifyFunction(*mainFunction, &os)) {
                os.flush();
                _diagnostics->diagnose(diag::DiagnosticID::function_verification_error, error);
                return false;
            }

            return true;
        }

        void Codegen::dumpProgram() {
            llvm::raw_ostream & os = llvm::outs();

            os << basic::Color::bold;
            _module->print(os, nullptr);
            os << basic::Color::reset;
        }

        void Codegen::newScope() {
            _currentScope = std::make_unique<Scope>(std::move(_currentScope));
        }

        void Codegen::endScope() {
            _currentScope = std::move(_currentScope->parent);
        }

        bool Codegen::newNamedValue(llvm::StringRef name, llvm::AllocaInst * alloca) {
            return _currentScope->newNamedValue(name, alloca);
        }

        bool Codegen::namedValueExists(llvm::StringRef name) const {
            return _currentScope->namedValueExists(name);
        }

        llvm::AllocaInst * Codegen::getNamedValue(llvm::StringRef name) const {
            return _currentScope->getNamedValue(name);
        }

        llvm::Function *
        Codegen::createFunction(llvm::Type * returnType, const std::vector<llvm::Type *> & params, bool isVarArg,
                                const std::string & name) {
            llvm::FunctionType * type = llvm::FunctionType::get(returnType, params, isVarArg);
            return llvm::Function::Create(type, llvm::Function::ExternalLinkage, name, _module.get());
        }
    }
}
