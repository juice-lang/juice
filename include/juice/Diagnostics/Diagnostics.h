// include/juice/Diagnostics/Diagnostics.h - DiagnosticEngine class, manages any diagnostics emitted by juice
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_DIAGNOSTICS_H
#define JUICE_DIAGNOSTICS_H

#include <cstdint>
#include <memory>
#include <vector>

#include "juice/Basic/RawStreamHelpers.h"
#include "juice/Basic/SourceBuffer.h"
#include "juice/Basic/SourceLocation.h"
#include "juice/Basic/SourceManager.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/SourceMgr.h"

namespace juice {
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
            /* implicit */ constexpr DiagnosticKind(Kind kind) : _kind(kind) {}

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
                color
            };

        private:
            Kind _kind;
            union {
                int _integer = 0;
                double _double;
                bool _boolean;
                llvm::StringRef _string;
                const parser::LexerToken * _lexerToken;
                const basic::Color _color;
            };

        public:
            DiagnosticArg() = delete;

            explicit DiagnosticArg(int integer): _kind(Kind::integer), _integer(integer) {}
            explicit DiagnosticArg(double doubleValue): _kind(Kind::doubleValue), _double(doubleValue) {}
            explicit DiagnosticArg(bool boolean): _kind(Kind::boolean), _boolean(boolean) {}
            explicit DiagnosticArg(llvm::StringRef string): _kind(Kind::string), _string(string) {}
            explicit DiagnosticArg(const parser::LexerToken * lexerToken): _kind(Kind::lexerToken), _lexerToken(lexerToken) {}
            explicit DiagnosticArg(const basic::Color color): _kind(Kind::color), _color(color) {}

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

            int getAsInteger() const { return _integer; }
            double getAsDouble() const { return _double; }
            bool getAsBoolean() const { return _boolean; }
            llvm::StringRef getAsString() const { return _string; }
            const parser::LexerToken * getAsLexerToken() const { return _lexerToken; }
            basic::Color getAsColor() const { return _color; }
        };

        class DiagnosticEngine {
            std::unique_ptr<basic::SourceManager> _sourceManager;

            bool _hadError = false;

        public:
            explicit DiagnosticEngine(std::unique_ptr<basic::SourceManager> sourceManager);

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
                                               const std::vector<DiagnosticArg> & args, int selectedIndex);

            static void
            formatDiagnosticArgInto(llvm::raw_ostream & out, llvm::StringRef modifier, llvm::StringRef modifierArguments,
                                    const std::vector<DiagnosticArg> & args, unsigned argIndex,
                                    DiagnosticEngine * diagnostics = nullptr);

            static void
            formatDiagnosticTextInto(llvm::raw_ostream & out, llvm::StringRef text, const std::vector<DiagnosticArg> & args,
                                     DiagnosticEngine * diagnostics = nullptr);

        public:
            static DiagnosticKind diagnosticKindFor(DiagnosticID id);
            static const char * diagnosticStringFor(DiagnosticID id);
            static bool diagnosticNewlineFor(DiagnosticID id);
        };
    }
}

#endif //JUICE_DIAGNOSTICS_H
