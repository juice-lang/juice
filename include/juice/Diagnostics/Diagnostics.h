// include/juice/Diagnostics/Diagnostics.h - DiagnosticEngine class, manages any diagnostics emitted by juice
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_DIAG_DIAGNOSTICS_H
#define JUICE_DIAG_DIAGNOSTICS_H

#include <cstdint>
#include <memory>
#include <string>
#include <system_error>
#include <vector>

#include "juice/Basic/RawStreamHelpers.h"
#include "juice/Basic/SourceBuffer.h"
#include "juice/Basic/SourceLocation.h"
#include "juice/Basic/SourceManager.h"
#include "juice/Sema/Type.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/SourceMgr.h"

namespace juice {
    namespace ast {
        class TypeRepr;

        llvm::raw_ostream & operator<<(llvm::raw_ostream & os, const TypeRepr * typeRepr);
    }

    namespace parser {
        struct LexerToken;

        class LexerTokenStream {
            llvm::raw_ostream & _os;
            const LexerToken * _token;

        public:
            LexerTokenStream(llvm::raw_ostream & os, const LexerToken * token): _os(os), _token(token) {}

            llvm::raw_ostream & getOS() const { return _os; }
            const LexerToken * getToken() const { return _token; }
        };

        std::unique_ptr<LexerTokenStream> operator<<(llvm::raw_ostream & os, const LexerToken * token);
        llvm::raw_ostream & operator<<(std::unique_ptr<LexerTokenStream> tokenStream,
                                  const basic::SourceManager * sourceManager);
    }

    namespace diag {
        enum class DiagnosticID: uint32_t {
            #define DIAG(KIND, ID, Text, Newline) ID,
            #include "Diagnostics.def"
        };

        class DiagnosticKind {
        public:
            enum Kind : uint8_t {
                error,
                warning,
                output
            };

            DiagnosticKind() = default;
            // NOLINTNEXTLINE(google-explicit-constructor)
            /* implicit */ constexpr DiagnosticKind(Kind kind) : _kind(kind) {}

            // NOLINTNEXTLINE(google-explicit-constructor)
            /* implicit */ constexpr operator Kind() const { return _kind; }

            explicit operator bool() = delete;
            constexpr bool operator==(DiagnosticKind other) const { return _kind == other._kind; }
            constexpr bool operator==(Kind other) const { return _kind == other; }
            constexpr bool operator!=(DiagnosticKind other) const { return _kind != other._kind; }
            constexpr bool operator!=(Kind other) const { return _kind != other; }

            llvm::SourceMgr::DiagKind llvm() const {
                switch (_kind) {
                    case error:   return llvm::SourceMgr::DK_Error;
                    case warning: return llvm::SourceMgr::DK_Warning;
                    case output:  return llvm::SourceMgr::DK_Error;
                }
            }

        private:
            Kind _kind;
        };

        class DiagnosticArg {
        public:
            enum class Kind {
                integer,
                doubleValue,
                boolean,
                string,
                lexerToken,
                color,
                type,
                types,
                typeRepr,
                errorCode
            };

        private:
            Kind _kind;
            union {
                u_int64_t _integer = 0;
                double _double;
                bool _boolean;
                llvm::StringRef _string;
                const parser::LexerToken * _lexerToken;
                const basic::Color _color;
                sema::Type _type;
                const std::vector<sema::Type> * _types;
                const ast::TypeRepr * _typeRepr;
                std::error_code _errorCode;
            };

        public:
            DiagnosticArg() = delete;

            // NOLINTBEGIN(cppcoreguidelines-pro-type-member-init)
            explicit DiagnosticArg(u_int8_t integer): _kind(Kind::integer), _integer(integer) {}
            explicit DiagnosticArg(int8_t integer): _kind(Kind::integer), _integer(integer) {}
            explicit DiagnosticArg(u_int16_t integer): _kind(Kind::integer), _integer(integer) {}
            explicit DiagnosticArg(int16_t integer): _kind(Kind::integer), _integer(integer) {}
            explicit DiagnosticArg(u_int32_t integer): _kind(Kind::integer), _integer(integer) {}
            explicit DiagnosticArg(int32_t integer): _kind(Kind::integer), _integer(integer) {}
            explicit DiagnosticArg(u_int64_t integer): _kind(Kind::integer), _integer(integer) {}
            explicit DiagnosticArg(int64_t integer): _kind(Kind::integer), _integer(integer) {}
            explicit DiagnosticArg(double doubleValue): _kind(Kind::doubleValue), _double(doubleValue) {}
            explicit DiagnosticArg(bool boolean): _kind(Kind::boolean), _boolean(boolean) {}
            explicit DiagnosticArg(const char * cString): _kind(Kind::string), _string(cString) {}
            explicit DiagnosticArg(char * cString): _kind(Kind::string), _string(cString) {}
            explicit DiagnosticArg(llvm::StringRef string): _kind(Kind::string), _string(string) {}
            explicit DiagnosticArg(const parser::LexerToken * lexerToken):
                _kind(Kind::lexerToken), _lexerToken(lexerToken) {}
            explicit DiagnosticArg(const basic::Color color): _kind(Kind::color), _color(color) {}
            explicit DiagnosticArg(sema::Type type): _kind(Kind::type), _type(type) {}
            explicit DiagnosticArg(const std::vector<sema::Type> * types): _kind(Kind::types), _types(types) {}
            explicit DiagnosticArg(const ast::TypeRepr * typeRepr): _kind(Kind::typeRepr), _typeRepr(typeRepr) {}
            explicit DiagnosticArg(std::error_code errorCode): _kind(Kind::errorCode), _errorCode(errorCode) {}
            // NOLINTEND(cppcoreguidelines-pro-type-member-init)

            explicit DiagnosticArg(const std::string & string) = delete;


            static void getAllInto(std::vector<DiagnosticArg> & vector) {}

            template<typename T>
            static void getAllInto(std::vector<DiagnosticArg> & vector, T first) {
                vector.push_back(DiagnosticArg(first));
            }

            template<typename T, typename... Args>
            static void getAllInto(std::vector<DiagnosticArg> & vector, T first, Args... args) {
                vector.push_back(DiagnosticArg(first));
                getAllInto(vector, args...);
            }

            Kind getKind() const { return _kind; }

            u_int64_t getAsInteger() const { return _integer; }
            double getAsDouble() const { return _double; }
            bool getAsBoolean() const { return _boolean; }
            llvm::StringRef getAsString() const { return _string; }
            const parser::LexerToken * getAsLexerToken() const { return _lexerToken; }
            basic::Color getAsColor() const { return _color; }
            sema::Type getAsType() const { return _type; }
            const std::vector<sema::Type> & getAsTypes() const { return *_types; }
            const ast::TypeRepr * getAsTypeRepr() const { return _typeRepr; }
            std::error_code getAsErrorCode() const { return _errorCode; }
        };

        class DiagnosticEngine {
            std::unique_ptr<basic::SourceManager> _sourceManager;

            llvm::raw_ostream & _outputOS;

            bool _hadError = false;

        public:
            DiagnosticEngine(std::unique_ptr<basic::SourceManager> sourceManager, llvm::raw_ostream & outputOS);

            bool hadError() const { return _hadError; }
            std::shared_ptr<basic::SourceBuffer> getBuffer() const { return _sourceManager->getMainBuffer(); }

            template<typename... Args>
            void diagnose(basic::SourceLocation location, DiagnosticID id, Args... args) {
                std::vector<DiagnosticArg> vector;
                DiagnosticArg::getAllInto(vector, args...);

                diagnose(location, id, vector);
            }

            void diagnose(basic::SourceLocation location, DiagnosticID id, const std::vector<DiagnosticArg> & args);

            template<typename... Args>
            static void diagnose(DiagnosticID id, Args... args) {
                std::vector<DiagnosticArg> vector;
                DiagnosticArg::getAllInto(vector, args...);

                diagnose(id, vector);
            }

            static void diagnose(DiagnosticID id, const std::vector<DiagnosticArg> & args);

        private:
            static llvm::StringRef
            skipToDelimiter(llvm::StringRef & text, char delimiter, bool * foundDelimiter = nullptr);

            static void formatSelectionArgInto(llvm::raw_ostream & out, llvm::StringRef modifierArguments,
                                               const std::vector<DiagnosticArg> & args, unsigned int selectedIndex);

            static void
            formatDiagnosticArgInto(llvm::raw_ostream & out, llvm::StringRef modifier, llvm::StringRef modifierArguments,
                                    const std::vector<DiagnosticArg> & args, unsigned int argIndex,
                                    DiagnosticEngine * diagnostics = nullptr);

            static void
            formatDiagnosticTextInto(llvm::raw_ostream & out, llvm::StringRef text, const std::vector<DiagnosticArg> & args,
                                     DiagnosticEngine * diagnostics = nullptr);

        public:
            static constexpr DiagnosticKind diagnosticKindFor(DiagnosticID id);
            static constexpr const char * diagnosticStringFor(DiagnosticID id);
            static constexpr bool diagnosticNewlineFor(DiagnosticID id);
        };
    }
}

#endif //JUICE_DIAG_DIAGNOSTICS_H
