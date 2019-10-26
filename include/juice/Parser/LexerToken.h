// include/juice/Parser/LexerToken.h - LexerToken and ErrorToken structs
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_LEXERTOKEN_H
#define JUICE_LEXERTOKEN_H

#include <cstdint>

#include "llvm/ADT/StringRef.h"
#include "juice/Diagnostics/Diagnostics.h"

namespace juice {
    namespace parser {
        struct LexerToken {
            enum class Type: uint8_t {
            //Operators
                //Dispatch operators
                operatorDot,

                //Assignment operators
                operatorAsteriskEqual,
                operatorEqual,
                operatorMinusEqual,
                operatorPercentEqual,
                operatorPlusEqual,
                operatorSlashEqual,

                //Arithmetic operators
                operatorAsterisk,
                operatorMinus,
                operatorPercent,
                operatorPlus,
                operatorSlash,

                //Comparison operators
                operatorBangEqual,
                operatorEqualEqual,
                operatorGreater,
                operatorGreaterEqual,
                operatorLower,
                operatorLowerEqual,

                //Bitwise operators
                operatorAnd,
                operatorPipe,

                //Boolean and optional operators
                operatorAndAnd,
                operatorBang,
                operatorQuestion,
                operatorPipePipe,

                //Range operators
                operatorDotDotDot,
                operatorDotDotLower,


            //Delimiters
                delimiterAt,
                delimiterColon,
                delimiterComma,
                delimiterLeftBrace,
                delimiterLeftBracket,
                delimiterLeftParen,
                delimiterNewline,
                delimiterRightBrace,
                delimiterRightBracket,
                delimiterRightParen,
                delimiterSemicolon,


            //Keywords
                //Declaration keywords
                keywordBinary,
                keywordClass,
                keywordCompound,
                keywordFailable,
                keywordFunc,
                keywordInit,
                keywordLet,
                keywordOverride,
                keywordPrivate,
                keywordUnary,
                keywordVar,

                //Statement keywords
                keywordBreak,
                keywordCase,
                keywordContinue,
                keywordDo,
                keywordElif,
                keywordElse,
                keywordFor,
                keywordIf,
                keywordIn,
                keywordReturn,
                keywordSwitch,
                keywordWhile,

                //Expression keywords
                keywordAs,
                keywordFalse,
                keywordIs,
                keywordNil,
                keywordPrint,
                keywordSelf,
                keywordSuper,
                keywordTrue,


                //Identifiers and literals
                identifier,
                integerLiteral,
                decimalLiteral,
                stringLiteral,


                //special tokens
                error,
                eof
            };

            Type type;
            llvm::StringRef string;

            LexerToken(Type type, llvm::StringRef string): type(type), string(string) {}

            virtual void diagnoseInto(diag::DiagnosticEngine & diagnostics);
        };

        struct ErrorToken: LexerToken {
            diag::DiagnosticID id;
            const char * errorPosition;

            ErrorToken(llvm::StringRef string, diag::DiagnosticID id, const char * errorPosition):
                    LexerToken(Type::error, string), id(id), errorPosition(errorPosition) {}

            void diagnoseInto(diag::DiagnosticEngine & diagnostics) override;
        };
    }
}

#endif //JUICE_LEXERTOKEN_H
