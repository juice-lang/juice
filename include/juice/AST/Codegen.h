// include/juice/AST/Codegen.h - class for first dummy generation of code
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_CODEGEN_H
#define JUICE_CODEGEN_H

#include <map>
#include <memory>
#include <vector>

#include "AST.h"
#include "StatementAST.h"
#include "juice/Diagnostics/Diagnostics.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

namespace juice {
    namespace ast {
        class Codegen: public std::enable_shared_from_this<Codegen> {
            std::unique_ptr<ModuleAST> _ast;

            std::shared_ptr<diag::DiagnosticEngine> _diagnostics;

            llvm::LLVMContext _context;
            llvm::IRBuilder<> _builder;
            std::unique_ptr<llvm::Module> _module;

            llvm::StringMap<llvm::AllocaInst *> _namedValues;

        public:
            Codegen(std::unique_ptr<ModuleAST> ast, std::shared_ptr<diag::DiagnosticEngine> diagnostics);

            bool generate();
            void dumpProgram();

            llvm::LLVMContext & getContext() { return _context; }
            const llvm::LLVMContext & getContext() const { return _context; }

            llvm::IRBuilder<> & getBuilder() { return _builder; }
            const llvm::IRBuilder<> & getBuilder() const { return _builder; }

            bool newNamedValue(llvm::StringRef name, llvm::AllocaInst * alloca);
            bool namedValueExists(llvm::StringRef name) const;
            llvm::AllocaInst * getNamedValue(llvm::StringRef name) const;

        private:
            llvm::Function *
            createFunction(llvm::Type * returnType, const std::vector<llvm::Type *> & params, bool isVarArg,
                           const std::string & name);
        };
    }
}

#endif //JUICE_CODEGEN_H
