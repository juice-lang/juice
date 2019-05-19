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
#include <ostream>
#include <vector>

#include "juice/Basic/SourceBuffer.h"
#include "juice/Basic/SourceLocation.h"
#include "juice/Basic/StringRef.h"

namespace juice {
    namespace parser {
        struct LexerToken;

        class LexerTokenStream {
            std::ostream & _os;
            const LexerToken * _token;

        public:
            LexerTokenStream(std::ostream & os, const LexerToken * token): _os(os), _token(token) {}

            std::ostream & getOS() const { return _os; }
            const LexerToken * getToken() const { return _token; }
        };

        std::unique_ptr<LexerTokenStream> operator<<(std::ostream & os, const LexerToken * token);
        std::ostream & operator<<(std::unique_ptr<LexerTokenStream> tokenStream,
                                  const std::shared_ptr<basic::SourceBuffer> & sourceBuffer);
    }

    namespace diag {
        enum class DiagnosticID: uint32_t {
            #define DIAG(KIND, ID, Text, Newline) ID,
            #include "Diagnostics.def"
        };

        enum class DiagnosticKind {
            error,
            warning,
            output
        };

        typedef std::ostream & (*Color)(std::ostream &);

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
                basic::StringRef _string;
                const parser::LexerToken * _lexerToken;
                const Color _color;
            };

        public:
            DiagnosticArg(int integer): _kind(Kind::integer), _integer(integer) {}
            DiagnosticArg(double doubleValue): _kind(Kind::doubleValue), _double(doubleValue) {}
            DiagnosticArg(bool boolean): _kind(Kind::boolean), _boolean(boolean) {}
            DiagnosticArg(basic::StringRef string): _kind(Kind::string), _string(string) {}
            DiagnosticArg(const parser::LexerToken * lexerToken): _kind(Kind::lexerToken), _lexerToken(lexerToken) {}
            DiagnosticArg(const Color color): _kind(Kind::color), _color(color) {}

            static void getAllInto(std::vector<DiagnosticArg> & vector) {}

            template<typename T>
            static void getAllInto(std::vector<DiagnosticArg> & vector, T first) {
                vector.push_back(first);
            }

            template<typename T, typename... Args>
            static void getAllInto(std::vector<DiagnosticArg> & vector, T first, Args... args) {
                vector.push_back(first);
                getAllInto(vector, args...);
            }

            Kind getKind() const { return _kind; }

            int getAsInteger() const { return _integer; }
            double getAsDouble() const { return _double; }
            bool getAsBoolean() const { return _boolean; }
            basic::StringRef getAsString() const { return _string; }
            const parser::LexerToken * getAsLexerToken() const { return _lexerToken; }
            const Color getAsColor() const { return _color; }
        };

        class DiagnosticEngine {
            std::shared_ptr<basic::SourceBuffer> _sourceBuffer;

            std::ostream & _output;
            std::ostream & _errorOutput;

            bool _hadError = false;

        public:
            explicit DiagnosticEngine(std::shared_ptr<basic::SourceBuffer> sourceBuffer);

            DiagnosticEngine(std::shared_ptr<basic::SourceBuffer> sourceBuffer, std::ostream & output,
                             std::ostream & errorOutput);

            bool hadError() const { return _hadError; }
            std::shared_ptr<basic::SourceBuffer> getBuffer() const { return _sourceBuffer; }

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
            static basic::StringRef
            skipToDelimiter(basic::StringRef & text, char delimiter, bool * foundDelimiter = nullptr);

            static void formatSelectionArgInto(std::ostream & out, basic::StringRef modifierArguments,
                                               const std::vector<DiagnosticArg> & args, int selectedIndex);

            static void
            formatDiagnosticArgInto(std::ostream & out, basic::StringRef modifier, basic::StringRef modifierArguments,
                                    const std::vector<DiagnosticArg> & args, int argIndex,
                                    DiagnosticEngine * diagnostics = nullptr);

            static void
            formatDiagnosticTextInto(std::ostream & out, basic::StringRef text, const std::vector<DiagnosticArg> & args,
                                     DiagnosticEngine * diagnostics = nullptr);

        public:
            static DiagnosticKind diagnosticKindFor(DiagnosticID id);
            static const char * diagnosticStringFor(DiagnosticID id);
            static bool diagnosticNewlineFor(DiagnosticID id);
        };
    }
}

#endif //JUICE_DIAGNOSTICS_H
