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
#include "llvm/ADT/StringRef.h"

namespace juice {
    namespace sema {
        class TypeCheckedModuleAST;

        class TypeChecker {
        public:
            class State {
                typedef std::pair<llvm::StringRef, const Type *> DeclarationPair;
                typedef std::vector<DeclarationPair> DeclarationVector;

                struct Scope {
                    DeclarationVector & declarations;
                    size_t currentIndex;

                    std::unique_ptr<Scope> parent;

                    Scope() = delete;

                    Scope(DeclarationVector & declarations, size_t currentIndex, std::unique_ptr<Scope> parent):
                        declarations(declarations), currentIndex(currentIndex), parent(std::move(parent)) {}

                    llvm::Optional<std::pair<size_t, const Type *>> getDeclaration(llvm::StringRef name) const;
                    llvm::Optional<size_t> addDeclaration(llvm::StringRef name, const Type * type);
                };

                DeclarationVector _declarations;
                std::unique_ptr<Scope> _currentScope;

            public:
                State();

                void newScope();
                void endScope();

                size_t declarationVectorSize() const { return _declarations.size(); }

                llvm::Optional<std::pair<size_t, const Type *>> getDeclaration(llvm::StringRef name) const;
                llvm::Optional<size_t> addDeclaration(llvm::StringRef name, const Type * type);
            };

            struct Result {
                std::unique_ptr<TypeCheckedModuleAST> ast;
                size_t declarationVectorSize;

                Result() = delete;

                Result(std::unique_ptr<TypeCheckedModuleAST> ast, size_t declarationVectorSize);
            };

        private:
            std::unique_ptr<ast::ModuleAST> _ast;

            std::shared_ptr<diag::DiagnosticEngine> _diagnostics;

        public:
            TypeChecker(std::unique_ptr<ast::ModuleAST> ast, std::shared_ptr<diag::DiagnosticEngine> diagnostics);

            Result typeCheck();
        };
    }
}

#endif //JUICE_SEMA_TYPECHECKER_H
