#include <stddef.h>
// include/juice/Sema/TypeCheckedAST.h - basic type-checked AST nodes
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_SEMA_TYPECHECKEDAST_H
#define JUICE_SEMA_TYPECHECKEDAST_H

#include <memory>
#include <vector>

#include "Type.h"
#include "TypeChecker.h"
#include "TypeHint.h"
#include "juice/AST/AST.h"
#include "juice/Basic/RawStreamHelpers.h"
#include "juice/Basic/SourceLocation.h"
#include "juice/Diagnostics/Diagnostics.h"
#include "juice/Parser/LexerToken.h"

namespace juice {
    namespace sema {
        class TypeCheckedStatementAST;
        class TypeCheckedExpressionAST;

        class TypeCheckedAST {
        protected:
            static basic::Color getColor(unsigned int level) {
                return basic::Color::rainbow[level % 6];
            }

        public:
            enum class Kind {
                container,
                module,
                block,
                container_last,
                controlFlowBody,
                expression,
                binaryOperatorExpression,
                numberExpression,
                variableExpression,
                groupingExpression,
                ifExpression,
                expression_last,
                statement,
                declaration,
                variableDeclaration,
                declaration_last,
                blockStatement,
                expressionStatement,
                ifStatement,
                whileStatement,
                statement_last
            };

        private:
            const Kind _kind;
            const Type * _type;

        protected:
            TypeCheckedAST(Kind kind, const Type * type): _kind(kind), _type(type) {}

        public:
            TypeCheckedAST() = delete;

            virtual ~TypeCheckedAST() = default;

            virtual basic::SourceLocation getLocation() const = 0;

            virtual void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const = 0;

            Kind getKind() const { return _kind; }
            const Type * getType() const { return _type; }
        };

        class TypeCheckedContainerAST: public TypeCheckedAST {
        public:
            typedef std::vector<std::unique_ptr<TypeCheckedStatementAST>> StatementVector;

        protected:
            StatementVector _statements;

            TypeCheckedContainerAST(Kind kind, const Type * type, StatementVector && statements);

        public:
            TypeCheckedContainerAST() = delete;

            ~TypeCheckedContainerAST() override = default;

            basic::SourceLocation getLocation() const override;

            static bool classof(const TypeCheckedAST * type) {
                return type->getKind() >= Kind::container
                    && type->getKind() <= Kind::container_last;
            }
        };

        class TypeCheckedModuleAST: public TypeCheckedContainerAST {
            TypeCheckedModuleAST(const Type * type, StatementVector && statements);

        public:
            TypeCheckedModuleAST() = delete;

            ~TypeCheckedModuleAST() override = default;

            void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const override;

            static std::unique_ptr<TypeCheckedModuleAST>
            createByTypeChecking(std::unique_ptr<ast::ModuleAST> ast, const TypeHint & hint, TypeChecker::State & state,
                                 diag::DiagnosticEngine & diagnostics);


            static bool classof(const TypeCheckedAST * type) {
                return type->getKind() == Kind::module;
            }
        };

        class TypeCheckedBlockAST: public TypeCheckedContainerAST {
            std::unique_ptr<parser::LexerToken> _start;

            TypeCheckedBlockAST(const Type * type, StatementVector && statements,
                                std::unique_ptr<parser::LexerToken> start);

        public:
            TypeCheckedBlockAST() = delete;

            ~TypeCheckedBlockAST() override = default;

            basic::SourceLocation getLocation() const override {
                return basic::SourceLocation(_start->string.begin());
            }

            void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const override;

            static std::unique_ptr<TypeCheckedBlockAST>
            createByTypeChecking(std::unique_ptr<ast::BlockAST> ast, const TypeHint & hint, TypeChecker::State & state,
                                 diag::DiagnosticEngine & diagnostics);


            static bool classof(const TypeCheckedAST * type) {
                return type->getKind() == Kind::block;
            }
        };


        class TypeCheckedControlFlowBodyAST: public TypeCheckedAST {
            enum class BodyKind {
                block,
                expression
            };

            std::unique_ptr<parser::LexerToken> _keyword;

            BodyKind _bodyKind;
            union {
                std::unique_ptr<TypeCheckedBlockAST> _block;
                std::unique_ptr<TypeCheckedExpressionAST> _expression;
            };

            TypeCheckedControlFlowBodyAST(const Type * type, std::unique_ptr<parser::LexerToken> keyword,
                                          std::unique_ptr<TypeCheckedBlockAST> block);
            TypeCheckedControlFlowBodyAST(const Type * type, std::unique_ptr<parser::LexerToken> keyword,
                                          std::unique_ptr<TypeCheckedExpressionAST> expression);

        public:
            TypeCheckedControlFlowBodyAST() = delete;

            ~TypeCheckedControlFlowBodyAST() override;

            basic::SourceLocation getLocation() const override {
                return basic::SourceLocation(_keyword->string.begin());
            }

            void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const override;

            static std::unique_ptr<TypeCheckedControlFlowBodyAST>
            createByTypeChecking(std::unique_ptr<ast::ControlFlowBodyAST> ast, const TypeHint & hint,
                                 TypeChecker::State & state, diag::DiagnosticEngine & diagnostics);


            const std::unique_ptr<parser::LexerToken> & getKeyword() const { return _keyword; }


            static bool classof(const TypeCheckedAST * type) {
                return type->getKind() == Kind::controlFlowBody;
            }
        };
    }
}

#endif //JUICE_SEMA_TYPECHECKEDAST_H
