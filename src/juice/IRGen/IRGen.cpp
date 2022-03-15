// src/juice/IRGen/IRGen.cpp - class for generating LLVM IR
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

#include "juice/Sema/TypeCheckedAST.h"
#include "juice/Sema/TypeCheckedExpressionAST.h"
#include "juice/Sema/TypeCheckedStatementAST.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/CodeGen.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

namespace juice {
    namespace irgen {
        IRGen::IRGen(sema::TypeChecker::Result typeCheckResult,
                     std::shared_ptr<diag::DiagnosticEngine> diagnostics):
            _ast(std::move(typeCheckResult.ast)), _diagnostics(std::move(diagnostics)), _builder(_context) {
            _module = std::make_unique<llvm::Module>("expression", _context);
            _allocas.resize(typeCheckResult.allocaVectorSize);
        }

        bool IRGen::generate() {
            llvm::Function * printfFunction = createFunction(llvm::Type::getInt32Ty(_context),
                                                             {llvm::Type::getInt8PtrTy(_context)}, true, "printf");

            llvm::Function * mainFunction = createFunction(llvm::Type::getInt32Ty(_context), {}, false, "main");
            llvm::BasicBlock * mainEntryBlock = llvm::BasicBlock::Create(_context, "entry", mainFunction);
            _builder.SetInsertPoint(mainEntryBlock);

            llvm::Value * value = generateModule();

            llvm::GlobalVariable * globalString;
            if (_ast->_type.isBuiltinDouble()) {
                globalString = _builder.CreateGlobalString("%f\n", ".str");
            } else if (_ast->_type.isBuiltinBool()) {
                globalString = _builder.CreateGlobalString("%s\n", ".str");

                auto * trueString = _builder.CreateGlobalString("true", "true.str");
                auto * falseString = _builder.CreateGlobalString("false", "false.str");

                llvm::BasicBlock * falseBlock = llvm::BasicBlock::Create(_context, "false", mainFunction);
                llvm::BasicBlock * mergeBlock = llvm::BasicBlock::Create(_context, "booleancont");

                _builder.CreateCondBr(value, mergeBlock, falseBlock);

                llvm::BasicBlock * trueBlock = _builder.GetInsertBlock();

                _builder.SetInsertPoint(falseBlock);
                _builder.CreateBr(mergeBlock);

                falseBlock = _builder.GetInsertBlock();


                mainFunction->getBasicBlockList().push_back(mergeBlock);
                _builder.SetInsertPoint(mergeBlock);

                llvm::Value * trueStringValue = _builder.CreateBitCast(trueString, llvm::Type::getInt8PtrTy(_context),
                                                                       "truecast");
                llvm::Value * falseStringValue = _builder.CreateBitCast(falseString, llvm::Type::getInt8PtrTy(_context),
                                                                       "falsecast");

                llvm::PHINode * phi = _builder.CreatePHI(llvm::Type::getInt8PtrTy(_context), 2, "booleanstr");
                phi->addIncoming(trueStringValue, trueBlock);
                phi->addIncoming(falseStringValue, falseBlock);

                value = phi;
            } else {
                llvm_unreachable("All possible yield types kinds should be handled here");
            }

            llvm::Value * stringValue = _builder.CreateBitCast(globalString, llvm::Type::getInt8PtrTy(_context),
                                                               "cast");

            _builder.CreateCall(printfFunction, {stringValue, value}, "printfCall");

            _builder.CreateRet(_builder.getInt32(0));

            std::string error;
            llvm::raw_string_ostream os(error);

            if (llvm::verifyFunction(*mainFunction, &os)) {
                os.flush();
                _diagnostics->diagnose(diag::DiagnosticID::function_verification_error, error.c_str());
                return false;
            }

            if (llvm::verifyModule(*_module, &os)) {
                os.flush();
                _diagnostics->diagnose(diag::DiagnosticID::module_verification_error, error.c_str());
                return false;
            }

            return true;
        }

        void IRGen::dumpProgram(llvm::raw_ostream & os) {
            os << basic::Color::bold;
            _module->print(os, nullptr);
            os << basic::Color::reset;
        }

        bool IRGen::emitObject(llvm::raw_pwrite_stream & os) {
            llvm::InitializeAllTargetInfos();
            llvm::InitializeAllTargets();
            llvm::InitializeAllTargetMCs();
            llvm::InitializeAllAsmParsers();
            llvm::InitializeAllAsmPrinters();

            std::string targetTriple = llvm::sys::getDefaultTargetTriple();
            _module->setTargetTriple(targetTriple);

            std::string errorString;
            const llvm::Target * target = llvm::TargetRegistry::lookupTarget(targetTriple, errorString);

            if (!target) {
                _diagnostics->diagnose(diag::DiagnosticID::target_lookup_error,
                                       targetTriple.c_str(), errorString.c_str());
                return false;
            }

            llvm::TargetOptions options;
            auto relocationModel = llvm::Optional<llvm::Reloc::Model>();
            auto targetMachine = target->createTargetMachine(targetTriple, "generic", "", options, relocationModel);

            _module->setDataLayout(targetMachine->createDataLayout());


            llvm::legacy::PassManager outputPassManager;
            auto outputFileType = llvm::CGFT_ObjectFile;

            if (targetMachine->addPassesToEmitFile(outputPassManager, os, nullptr, outputFileType)) {
                llvm::errs() << "Target machine cannot emit a file of this type";
                return false;
            }

            outputPassManager.run(*_module);
            os.flush();

            return true;
        }

        llvm::Value * IRGen::generateModule() {
            switch (_ast->_statements.size()) {
                case 0:
                    llvm_unreachable("Module has to return a value at the moment");
                case 1:
                    return generateYieldingStatement(std::move(_ast->_statements.front()));
                default: {
                    auto last = _ast->_statements.end() - 1;
                    for (auto it = _ast->_statements.begin(); it < last; ++it) {
                        generateStatement(std::move(*it));
                    }
                    return generateYieldingStatement(std::move(*last));
                }
            }
        }

        llvm::Value * IRGen::generateBlock(std::unique_ptr<sema::TypeCheckedBlockAST> block) {
            switch (block->_statements.size()) {
                case 0:
                    return nullptr;
                case 1:
                    return generateYieldingStatement(std::move(block->_statements.front()));
                default: {
                    auto last = block->_statements.end() - 1;
                    for (auto it = block->_statements.begin(); it < last; ++it) {
                        generateStatement(std::move(*it));
                    }
                    return generateYieldingStatement(std::move(*last));
                }
            }
        }

        llvm::Value * IRGen::generateControlFlowBody(std::unique_ptr<sema::TypeCheckedControlFlowBodyAST> body) {
            switch (body->_bodyKind) {
                case sema::TypeCheckedControlFlowBodyAST::BodyKind::block:
                    return generateBlock(std::move(body->_block));
                case sema::TypeCheckedControlFlowBodyAST::BodyKind::expression:
                    return generateExpression(std::move(body->_expression));
            }
        }

        llvm::Function * IRGen::createFunction(llvm::Type * returnType, const std::vector<llvm::Type *> & params,
                                               bool isVarArg, llvm::StringRef name) {
            llvm::FunctionType * type = llvm::FunctionType::get(returnType, params, isVarArg);
            return llvm::Function::Create(type, llvm::Function::ExternalLinkage, name, _module.get());
        }
    }
}
