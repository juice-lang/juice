// src/juice/Sema/TypeChecker.cpp - type checking class
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/Sema/TypeChecker.h"

#include "juice/Sema/BuiltinType.h"
#include "juice/Sema/TypeCheckedAST.h"
#include "juice/Sema/TypeCheckedStatementAST.h"
#include "juice/Sema/TypeHint.h"

namespace juice {
    namespace sema {
        llvm::Optional<std::pair<size_t, const Type *>>
        TypeChecker::State::Scope::getDeclaration(llvm::StringRef name) const {
            auto begin = declarations.begin();
            auto result = std::find_if(begin, begin + currentIndex, [&name](const DeclarationPair & pair) {
                return pair.first.equals(name);
            });

            if (result == begin + currentIndex) return llvm::None;

            return std::make_pair((size_t)(result - begin), (*result).second);
        }

        llvm::Optional<size_t> TypeChecker::State::Scope::addDeclaration(llvm::StringRef name, const Type * type) {
            if (!getDeclaration(name)) {
                auto begin = declarations.begin();
                if (begin + currentIndex == declarations.end()) {
                    declarations.emplace_back(name, type);
                } else declarations.at(currentIndex) = std::make_pair(name, type);

                return currentIndex++;
            }
            return llvm::None;
        }

        TypeChecker::State::State():
            _currentScope(std::make_unique<Scope>(_declarations, 0, nullptr)) {}

        void TypeChecker::State::newScope() {
            _currentScope = std::make_unique<Scope>(_declarations, _currentScope->currentIndex,
                                                    std::move(_currentScope));
        }

        void TypeChecker::State::endScope() {
            _currentScope = std::move(_currentScope->parent);
        }

        TypeChecker::Result::Result(std::unique_ptr<TypeCheckedModuleAST> ast, size_t declarationVectorSize):
            ast(std::move(ast)), declarationVectorSize(declarationVectorSize) {}

        llvm::Optional<std::pair<size_t, const Type *>> TypeChecker::State::getDeclaration(llvm::StringRef name) const {
            return _currentScope->getDeclaration(name);
        }

        llvm::Optional<size_t> TypeChecker::State::addDeclaration(llvm::StringRef name, const Type * type) {
            return _currentScope->addDeclaration(name, type);
        }

        TypeChecker::TypeChecker(std::unique_ptr<ast::ModuleAST> ast,
                                 std::shared_ptr<diag::DiagnosticEngine> diagnostics):
            _ast(std::move(ast)), _diagnostics(std::move(diagnostics)) {}

        TypeChecker::Result TypeChecker::typeCheck() {
            State state;

            auto ast =
                TypeCheckedModuleAST::createByTypeChecking(std::move(_ast),
                                                           ExpectedTypeHint(BuiltinFloatingPointType::getDouble()),
                                                           state, *_diagnostics);

            return { std::move(ast), state.declarationVectorSize() };
        }
    }
}
