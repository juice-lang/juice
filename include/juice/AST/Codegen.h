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

#include <memory>
#include <vector>

#include "AST.h"
#include "StatementAST.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

namespace juice {
    namespace ast {
        class Codegen {
            std::unique_ptr<ModuleAST> _ast;

            llvm::LLVMContext _context;
            llvm::IRBuilder<> _builder;
            std::unique_ptr<llvm::Module> _module;

        public:
            explicit Codegen(std::unique_ptr<ModuleAST> ast);

            bool generate(llvm::raw_string_ostream & os);
            void dumpProgram(llvm::raw_ostream & os);

        private:
            llvm::Function *
            createFunction(llvm::Type * returnType, const std::vector<llvm::Type *> & params, bool isVarArg,
                           const std::string & name);
        };
    }
}

#endif //JUICE_CODEGEN_H
