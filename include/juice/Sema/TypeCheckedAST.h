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

        private:
            const Type * _type;

        protected:
            explicit TypeCheckedAST(const Type * type): _type(type) {}

        public:
            TypeCheckedAST() = delete;

            virtual ~TypeCheckedAST() = default;

            virtual basic::SourceLocation getLocation() const = 0;

            virtual void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const = 0;

            const Type * getType() const { return _type; }
        };

        class TypeCheckedContainerAST: public TypeCheckedAST {
        public:
            typedef std::vector<std::unique_ptr<TypeCheckedStatementAST>> StatementVector;

        protected:
            StatementVector _statements;

            TypeCheckedContainerAST(const Type * type, StatementVector && statements);

        public:
            TypeCheckedContainerAST() = delete;

            ~TypeCheckedContainerAST() override = default;

            basic::SourceLocation getLocation() const override;
        };

        class TypeCheckedModuleAST: public TypeCheckedContainerAST {
            TypeCheckedModuleAST(const Type * type, StatementVector && statements);

        public:
            TypeCheckedModuleAST() = delete;

            ~TypeCheckedModuleAST() override = default;

            void diagnoseInto(diag::DiagnosticEngine & diagnostics, unsigned int level) const override;

            static std::unique_ptr<TypeCheckedModuleAST>
            createByTypeChecking(std::unique_ptr<ast::ModuleAST> ast, const TypeHint & hint,
                                 diag::DiagnosticEngine & diagnostics);
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
            createByTypeChecking(std::unique_ptr<ast::BlockAST> ast, const TypeHint & hint,
                                 diag::DiagnosticEngine & diagnostics);
        };


        class TypeCheckedControlFlowBodyAST: public TypeCheckedAST {
            enum class Kind {
                block,
                expression
            };

            std::unique_ptr<parser::LexerToken> _keyword;

            Kind _kind;
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
                                 diag::DiagnosticEngine & diagnostics);


            const std::unique_ptr<parser::LexerToken> & getKeyword() const { return _keyword; }
        };
    }
}

#endif //JUICE_SEMA_TYPECHECKEDAST_H
