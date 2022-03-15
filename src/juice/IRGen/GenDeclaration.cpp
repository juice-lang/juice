// src/juice/IRGen/GenDeclaration.cpp - generate LLVM IR from declaration AST nodes
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/IRGen/IRGen.h"

#include "juice/Sema/TypeCheckedDeclarationAST.h"
#include "llvm/Support/Casting.h"

namespace juice {
    namespace irgen {
        void IRGen::generateDeclaration(std::unique_ptr<sema::TypeCheckedDeclarationAST> declaration) {
            switch (declaration->_kind) {
                case sema::TypeCheckedAST::Kind::variableDeclaration: {
                    auto variable = std::unique_ptr<sema::TypeCheckedVariableDeclarationAST>(
                        llvm::cast<sema::TypeCheckedVariableDeclarationAST>(declaration.release()));
                    generateVariableDeclaration(std::move(variable));
                    break;
                }
                default:
                    llvm_unreachable("All declaration AST nodes should be handled here");
            }
        }

        void IRGen::generateVariableDeclaration(std::unique_ptr<sema::TypeCheckedVariableDeclarationAST> declaration) {
            auto value = generateExpression(std::move(declaration->_initialization));

            llvm::AllocaInst * alloca = _builder.CreateAlloca(declaration->_variableType->toLLVM(_context), nullptr,
                                                              declaration->_name->string);
            _builder.CreateStore(value, alloca);

            _allocas.at(declaration->_index) = alloca;
        }
    }
}
