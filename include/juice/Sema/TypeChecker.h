// include/juice/Sema/TypeChecker.h - type checking class
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_SEMA_TYPECHECKER_H
#define JUICE_SEMA_TYPECHECKER_H

#include <memory>
#include <vector>
#include <utility>

#include "Type.h"
#include "juice/AST/AST.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"

namespace juice {
    namespace sema {
        class TypeCheckedModuleAST;

        class TypeChecker {
        public:
            class State {
                typedef std::pair<llvm::StringRef, Type> VariableDeclarationPair;
                typedef std::vector<VariableDeclarationPair> VariableDeclarationVector;

                struct Scope {
                    llvm::StringMap<Type> typeDeclarations;

                    VariableDeclarationVector & variableDeclarations;
                    size_t currentVariableIndex;

                    std::unique_ptr<Scope> parent;

                    Scope() = delete;

                    Scope(VariableDeclarationVector & variableDeclarations, size_t currentVariableIndex,
                          std::unique_ptr<Scope> parent):
                        variableDeclarations(variableDeclarations), currentVariableIndex(currentVariableIndex),
                        parent(std::move(parent)) {}

                    bool hasTypeDeclaration(llvm::StringRef name) const;
                    llvm::Optional<Type> getTypeDeclaration(llvm::StringRef name) const;
                    bool addTypeDeclaration(llvm::StringRef name, Type type);

                    bool hasVariableDeclaration(llvm::StringRef name) const;
                    llvm::Optional<std::pair<size_t, Type>> getVariableDeclaration(llvm::StringRef name) const;
                    llvm::Optional<size_t> addVariableDeclaration(llvm::StringRef name, Type type);
                };

                VariableDeclarationVector _variableDeclarations;
                std::unique_ptr<Scope> _currentScope;

            public:
                State();

                void newScope();
                void endScope();

                size_t getAllocaVectorSize() const { return _variableDeclarations.size(); }

                bool hasTypeDeclaration(llvm::StringRef name) const;
                llvm::Optional<Type> getTypeDeclaration(llvm::StringRef name) const;
                bool addTypeDeclaration(llvm::StringRef name, Type type);

                bool hasVariableDeclaration(llvm::StringRef name) const;
                llvm::Optional<std::pair<size_t, Type>> getVariableDeclaration(llvm::StringRef name) const;
                llvm::Optional<size_t> addVariableDeclaration(llvm::StringRef name, Type type);
            };

            struct Result {
                std::unique_ptr<TypeCheckedModuleAST> ast;
                size_t allocaVectorSize;

                Result() = delete;

                Result(std::unique_ptr<TypeCheckedModuleAST> ast, size_t allocaVectorSize);
            };

        private:
            std::unique_ptr<ast::ModuleAST> _ast;

            std::shared_ptr<diag::DiagnosticEngine> _diagnostics;

        public:
            TypeChecker(std::unique_ptr<ast::ModuleAST> ast, std::shared_ptr<diag::DiagnosticEngine> diagnostics);

            Result typeCheck();

        private:
            static void declareBuiltinTypes(State & state);
        };
    }
}

#endif //JUICE_SEMA_TYPECHECKER_H
