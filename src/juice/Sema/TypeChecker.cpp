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
        bool TypeChecker::State::Scope::hasTypeDeclaration(llvm::StringRef name) const {
            auto result = typeDeclarations.find(name);

            if (result == typeDeclarations.end()) {
                return parent && parent->hasTypeDeclaration(name);
            }

            return true;
        }

        llvm::Optional<Type> TypeChecker::State::Scope::getTypeDeclaration(llvm::StringRef name) const {
            auto result = typeDeclarations.find(name);

            if (result == typeDeclarations.end()) {
                if (parent) return parent->getTypeDeclaration(name);

                return llvm::None;
            }

            return result->getValue();
        }

        bool TypeChecker::State::Scope::addTypeDeclaration(llvm::StringRef name, Type type) {
            if (!(hasVariableDeclaration(name) || (parent && parent->hasTypeDeclaration(name)))) {
                return typeDeclarations.try_emplace(name, type).second;
            }

            return false;
        }

        bool TypeChecker::State::Scope::hasVariableDeclaration(llvm::StringRef name) const {
            auto begin = variableDeclarations.begin();
            auto result = std::find_if(begin, begin + currentVariableIndex, [&](const auto & declaration) {
                return declaration.name.equals(name);
            });

            return result != begin + currentVariableIndex;
        }

        llvm::Optional<VariableDeclaration>
        TypeChecker::State::Scope::getVariableDeclaration(llvm::StringRef name) const {
            auto begin = variableDeclarations.begin();
            auto result = std::find_if(begin, begin + currentVariableIndex, [&](const auto & declaration) {
                return declaration.name.equals(name);
            });

            if (result == begin + currentVariableIndex) return llvm::None;

            return *result;
        }

        llvm::Optional<size_t>
        TypeChecker::State::Scope::addVariableDeclaration(llvm::StringRef name, Type type, bool isMutable) {
            if (!(hasVariableDeclaration(name) || hasTypeDeclaration(name))) {
                auto begin = variableDeclarations.begin();
                auto currentIter = begin + currentVariableIndex;
                if (currentIter == variableDeclarations.end()) {
                    variableDeclarations.emplace_back(name, type, currentVariableIndex, isMutable);
                } else variableDeclarations.emplace(currentIter, name, type, currentVariableIndex, isMutable);

                return currentVariableIndex++;
            }

            return llvm::None;
        }

        TypeChecker::State::State():
            _currentScope(std::make_unique<Scope>(_variableDeclarations, 0, nullptr)) {}

        void TypeChecker::State::newScope() {
            _currentScope = std::make_unique<Scope>(_variableDeclarations, _currentScope->currentVariableIndex,
                                                    std::move(_currentScope));
        }

        void TypeChecker::State::endScope() {
            _currentScope = std::move(_currentScope->parent);
        }

        bool TypeChecker::State::hasTypeDeclaration(llvm::StringRef name) const {
            return _currentScope->hasTypeDeclaration(name);
        }

        llvm::Optional<Type> TypeChecker::State::getTypeDeclaration(llvm::StringRef name) const {
            return _currentScope->getTypeDeclaration(name);
        }

        bool TypeChecker::State::addTypeDeclaration(llvm::StringRef name, Type type) {
            return _currentScope->addTypeDeclaration(name, type);
        }

        bool TypeChecker::State::hasVariableDeclaration(llvm::StringRef name) const {
            return _currentScope->hasVariableDeclaration(name);
        }

        llvm::Optional<VariableDeclaration> TypeChecker::State::getVariableDeclaration(llvm::StringRef name) const {
            return _currentScope->getVariableDeclaration(name);
        }

        llvm::Optional<size_t>
        TypeChecker::State::addVariableDeclaration(llvm::StringRef name, Type type, bool isMutable) {
            return _currentScope->addVariableDeclaration(name, type, isMutable);
        }

        TypeChecker::Result::Result(std::unique_ptr<TypeCheckedModuleAST> ast, size_t allocaVectorSize):
            ast(std::move(ast)), allocaVectorSize(allocaVectorSize) {}

        TypeChecker::TypeChecker(std::unique_ptr<ast::ModuleAST> ast,
                                 std::shared_ptr<diag::DiagnosticEngine> diagnostics):
            _ast(std::move(ast)), _diagnostics(std::move(diagnostics)) {}

        TypeChecker::Result TypeChecker::typeCheck() {
            State state;

            declareBuiltinTypes(state);

            const TypeHint & hint = ExpectedEitherTypeHint({
                #define BUILTIN_TYPE(Name, Initialization) Initialization,
                #include "juice/Sema/BuiltinTypes.def"
            });

            auto ast = TypeCheckedModuleAST::createByTypeChecking(std::move(_ast), hint, state, *_diagnostics);

            return { std::move(ast), state.getAllocaVectorSize() };
        }

        void TypeChecker::declareBuiltinTypes(State & state) {
            #define BUILTIN_TYPE(Name, Initialization) state.addTypeDeclaration(Name, Initialization);
            #include "juice/Sema/BuiltinTypes.def"
        }
    }
}
