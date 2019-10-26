// src/juice/Parser/LexerToken.cpp - LexerToken and ErrorToken structs
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/Parser/LexerToken.h"

#include "juice/Basic/SourceLocation.h"

namespace juice {
    namespace parser {
        static const char * tokenTypeName(const LexerToken * token) {
            switch (token->type) {
                case LexerToken::Type::operatorDot:             return "OPERATOR_DOT";
                case LexerToken::Type::operatorAsteriskEqual:   return "OPERATOR_ASTERISK_EQUAL";
                case LexerToken::Type::operatorEqual:           return "OPERATOR_EQUAL";
                case LexerToken::Type::operatorMinusEqual:      return "OPERATOR_MINUS_EQUAL";
                case LexerToken::Type::operatorPercentEqual:    return "OPERATOR_PERCENT_EQUAL";
                case LexerToken::Type::operatorPlusEqual:       return "OPERATOR_PLUS_EQUAL";
                case LexerToken::Type::operatorSlashEqual:      return "OPERATOR_SLASH_EQUAL";
                case LexerToken::Type::operatorAsterisk:        return "OPERATOR_ASTERISK";
                case LexerToken::Type::operatorMinus:           return "OPERATOR_MINUS";
                case LexerToken::Type::operatorPercent:         return "OPERATOR_PERCENT";
                case LexerToken::Type::operatorPlus:            return "OPERATOR_PLUS";
                case LexerToken::Type::operatorSlash:           return "OPERATOR_SLASH";
                case LexerToken::Type::operatorBangEqual:       return "OPERATOR_BANG_EQUAL";
                case LexerToken::Type::operatorEqualEqual:      return "OPERATOR_EQUAL_EQUAL";
                case LexerToken::Type::operatorGreater:         return "OPERATOR_GREATER";
                case LexerToken::Type::operatorGreaterEqual:    return "OPERATOR_GREATER_EQUAL";
                case LexerToken::Type::operatorLower:           return "OPERATOR_LOWER";
                case LexerToken::Type::operatorLowerEqual:      return "OPERATOR_LOWER_EQUAL";
                case LexerToken::Type::operatorAnd:             return "OPERATOR_AND";
                case LexerToken::Type::operatorPipe:            return "OPERATOR_PIPE";
                case LexerToken::Type::operatorAndAnd:          return "OPERATOR_AND_AND";
                case LexerToken::Type::operatorBang:            return "OPERATOR_BANG";
                case LexerToken::Type::operatorQuestion:        return "OPERATOR_QUESTION";
                case LexerToken::Type::operatorPipePipe:        return "OPERATOR_PIPE_PIPE";
                case LexerToken::Type::operatorDotDotDot:       return "OPERATOR_DOT_DOT_DOT";
                case LexerToken::Type::operatorDotDotLower:     return "OPERATOR_DOT_DOT_LOWER";
                case LexerToken::Type::delimiterAt:             return "DELIMITER_AT";
                case LexerToken::Type::delimiterColon:          return "DELIMITER_COLON";
                case LexerToken::Type::delimiterComma:          return "DELIMITER_COMMA";
                case LexerToken::Type::delimiterLeftBrace:      return "DELIMITER_LEFT_BRACE";
                case LexerToken::Type::delimiterLeftBracket:    return "DELIMITER_LEFT_BRACKET";
                case LexerToken::Type::delimiterLeftParen:      return "DELIMITER_LEFT_PARENTHESIS";
                case LexerToken::Type::delimiterNewline:        return "DELIMITER_NEWLINE";
                case LexerToken::Type::delimiterRightBrace:     return "DELIMITER_RIGHT_BRACE";
                case LexerToken::Type::delimiterRightBracket:   return "DELIMITER_RIGHT_BRACKET";
                case LexerToken::Type::delimiterRightParen:     return "DELIMITER_RIGHT_PARENTHESIS";
                case LexerToken::Type::delimiterSemicolon:      return "DELIMITER_SEMICOLON";
                case LexerToken::Type::keywordBinary:           return "KEYWORD_BINARY";
                case LexerToken::Type::keywordClass:            return "KEYWORD_CLASS";
                case LexerToken::Type::keywordCompound:         return "KEYWORD_COMPOUND";
                case LexerToken::Type::keywordFailable:         return "KEYWORD_FAILABLE";
                case LexerToken::Type::keywordFunc:             return "KEYWORD_FUNC";
                case LexerToken::Type::keywordInit:             return "KEYWORD_INIT";
                case LexerToken::Type::keywordLet:              return "KEYWORD_LET";
                case LexerToken::Type::keywordOverride:         return "KEYWORD_OVERRIDE";
                case LexerToken::Type::keywordPrivate:          return "KEYWORD_PRIVATE";
                case LexerToken::Type::keywordUnary:            return "KEYWORD_UNARY";
                case LexerToken::Type::keywordVar:              return "KEYWORD_VAR";
                case LexerToken::Type::keywordBreak:            return "KEYWORD_BREAK";
                case LexerToken::Type::keywordCase:             return "KEYWORD_CASE";
                case LexerToken::Type::keywordContinue:         return "KEYWORD_CONTINUE";
                case LexerToken::Type::keywordDo:               return "KEYWORD_DO";
                case LexerToken::Type::keywordElif:             return "KEYWORD_ELIF";
                case LexerToken::Type::keywordElse:             return "KEYWORD_ELSE";
                case LexerToken::Type::keywordFor:              return "KEYWORD_FOR";
                case LexerToken::Type::keywordIf:               return "KEYWORD_IF";
                case LexerToken::Type::keywordIn:               return "KEYWORD_IN";
                case LexerToken::Type::keywordReturn:           return "KEYWORD_RETURN";
                case LexerToken::Type::keywordSwitch:           return "KEYWORD_SWITCH";
                case LexerToken::Type::keywordWhile:            return "KEYWORD_WHILE";
                case LexerToken::Type::keywordAs:               return "KEYWORD_AS";
                case LexerToken::Type::keywordFalse:            return "KEYWORD_FALSE";
                case LexerToken::Type::keywordIs:               return "KEYWORD_IS";
                case LexerToken::Type::keywordNil:              return "KEYWORD_NIL";
                case LexerToken::Type::keywordPrint:            return "KEYWORD_PRINT";
                case LexerToken::Type::keywordSelf:             return "KEYWORD_SELF";
                case LexerToken::Type::keywordSuper:            return "KEYWORD_SUPER";
                case LexerToken::Type::keywordTrue:             return "KEYWORD_TRUE";
                case LexerToken::Type::identifier:              return "IDENTIFIER";
                case LexerToken::Type::integerLiteral:          return "INTEGER_LITERAL";
                case LexerToken::Type::decimalLiteral:          return "DECIMAL_LITERAL";
                case LexerToken::Type::stringLiteral:           return "STRING_LITERAL";
                case LexerToken::Type::error:                   return "ERROR";
                case LexerToken::Type::eof:                     return "EOF";
            }
        }
        
        void LexerToken::diagnoseInto(diag::DiagnosticEngine & diagnostics) {
            basic::SourceLocation location(string.begin());
            diagnostics.diagnose(location, diag::DiagnosticID::lexer_token, this);
        }

        std::unique_ptr<LexerTokenStream> operator<<(llvm::raw_ostream & os, const LexerToken * token) {
            return std::make_unique<LexerTokenStream>(os, token);
        }

        llvm::raw_ostream & operator<<(std::unique_ptr<LexerTokenStream> tokenStream,
                                  const basic::SourceManager * sourceManager) {
            llvm::raw_ostream & os = tokenStream->getOS();
            const LexerToken * token = tokenStream->getToken();

            os << "<" << tokenTypeName(token);

            if (sourceManager != nullptr) {
                basic::SourceLocation location(token->string.begin());

                unsigned line, column;
                std::tie(line, column) = sourceManager->getLineAndColumn(location);

                os << " " << line << ":" << column;
            }

            os << " \"";

            for (char c: token->string) {
                switch (c) {
                    case '\0': os << "\\0"; break;
                    case '\t': os << "\\t"; break;
                    case '\n': os << "\\n"; break;
                    case '\r': os << "\\r"; break;
                    case '\"': os << "\\\""; break;
                    case '\'': os << "\\\'"; break;
                    default: os << c;
                }
            }

            return os << "\">";
        }
        
        void ErrorToken::diagnoseInto(diag::DiagnosticEngine & diagnostics) {
            basic::SourceLocation location(errorPosition);
            diagnostics.diagnose(location, id);
        }
    }
}
