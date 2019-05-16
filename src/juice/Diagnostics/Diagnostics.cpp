// src/juice/Diagnostics/Diagnostics.cpp - DiagnosticEngine class, manages any diagnostics emitted by juice
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/Diagnostics/Diagnostics.h"

#include <iostream>
#include <string>
#include <tuple>
#include <utility>

#include "juice/Basic/StringHelpers.h"
#include "termcolor/termcolor.hpp"

namespace juice {
    namespace diag {
        static const constexpr DiagnosticKind diagnosticKinds[] {
            #define ERROR(ID, Text) DiagnosticKind::error,
            #define WARNING(ID, Text) DiagnosticKind::warning,
            #define OUTPUT(ID, Text) DiagnosticKind::output,
            #include "juice/Diagnostics/Diagnostics.def"
        };

        static constexpr const char * const diagnosticStrings[] {
            #define DIAG(KIND, ID, Text) Text,
            #include "juice/Diagnostics/Diagnostics.def"
        };

        DiagnosticEngine::DiagnosticEngine(std::shared_ptr<juice::basic::SourceBuffer> sourceBuffer):
                _sourceBuffer(std::move(sourceBuffer)), _output(std::cout), _errorOutput(std::cerr) {}

        DiagnosticEngine::DiagnosticEngine(std::shared_ptr<basic::SourceBuffer> sourceBuffer, std::ostream & output,
                                           std::ostream & errorOutput):
                _sourceBuffer(std::move(sourceBuffer)), _output(output), _errorOutput(errorOutput) {}

        void DiagnosticEngine::diagnose(basic::SourceLocation location, DiagnosticID id,
                                        const std::vector<DiagnosticArg> & args) {
            DiagnosticKind kind = diagnosticKindFor(id);
            basic::StringRef text(diagnosticStringFor(id));

            std::ostream & os = kind == DiagnosticKind::error ? _errorOutput : _output;

            os << termcolor::bold;

            switch (kind) {
                case DiagnosticKind::error: {
                    _hadError = true;
                    os << "juice: " << termcolor::red << "error";
                    break;
                }
                case DiagnosticKind::warning: {
                    os << "juice: " << termcolor::magenta << "warning";
                    break;
                }
                case DiagnosticKind::output: break;
            }

            if (kind != DiagnosticKind::output && location.isValid()) {
                unsigned line, column;
                std::tie(line, column) = _sourceBuffer->getLineAndColumn(location);

                os << " at " << line << ":" << column << ": " << termcolor::reset << termcolor::bold;

                formatDiagnosticTextInto(os, text, args);

                os << termcolor::reset << std::endl << _sourceBuffer->getLineString(location) << std::endl;

                os << std::string(column - 1, ' ') << termcolor::green << '^' << termcolor::reset;
            } else {
                if (kind != DiagnosticKind::output) os << ": " << termcolor::reset << termcolor::bold;

                formatDiagnosticTextInto(os, text, args);
            }

            os << termcolor::reset << std::endl;
        }

        void DiagnosticEngine::diagnose(DiagnosticID id, const std::vector<DiagnosticArg> & args) {
            DiagnosticKind kind = diagnosticKindFor(id);
            basic::StringRef text(diagnosticStringFor(id));

            std::ostream & os = kind == DiagnosticKind::error ? std::cerr : std::cout;

            os << termcolor::bold;

            switch (kind) {
                case DiagnosticKind::error: {
                    os << "juice: " << termcolor::red << "error" << ": " << termcolor::reset << termcolor::bold;
                    break;
                }
                case DiagnosticKind::warning: {
                    os << "juice: " << termcolor::magenta << "warning" << ": " << termcolor::reset << termcolor::bold;
                    break;
                }
                case DiagnosticKind::output: break;
            }

            formatDiagnosticTextInto(os, text, args);

            os << termcolor::reset << std::endl;
        }

        basic::StringRef
        DiagnosticEngine::skipToDelimiter(basic::StringRef & text, char delimiter, bool * foundDelimiter) {
            unsigned depth = 0;
            if (foundDelimiter) *foundDelimiter = false;

            unsigned i = 0;
            for (unsigned n = text.size(); i != n; ++i) {
                if (text[i] == '{') {
                    ++depth;
                    continue;
                }
                if (depth > 0) {
                    if (text[i] == '}')
                        --depth;
                    continue;
                }

                if (text[i] == delimiter) {
                    if (foundDelimiter) *foundDelimiter = true;
                    break;
                }
            }

            assert(depth == 0 && "Unbalanced {} set in diagnostic text");
            basic::StringRef result = text.substr(0, i);
            text = text.substr(i + 1);
            return result;
        }

        void DiagnosticEngine::formatSelectionArgInto(std::ostream & out, basic::StringRef modifierArguments,
                                                      const std::vector<DiagnosticArg> & args, int selectedIndex) {
            bool foundPipe = false;
            do {
                assert((modifierArguments.isNotEmpty() || foundPipe) && "Index beyond bounds in %select modifier");
                basic::StringRef text = skipToDelimiter(modifierArguments, '|', &foundPipe);
                if (selectedIndex == 0) {
                    formatDiagnosticTextInto(out, text, args);
                    break;
                }
                --selectedIndex;
            } while (true);
        }

        void DiagnosticEngine::formatDiagnosticArgInto(std::ostream & out, basic::StringRef modifier,
                                                       basic::StringRef modifierArguments,
                                                       const std::vector<DiagnosticArg> & args, int argIndex) {
            DiagnosticArg arg = args[argIndex];
            switch (arg.getKind()) {
                case DiagnosticArg::Kind::integer: {
                    if (modifier == "select") {
                        assert(arg.getAsInteger() >= 0 && "Negative selection index");
                        formatSelectionArgInto(out, modifierArguments, args, arg.getAsInteger());
                    } else if (modifier == "s") {
                        if (arg.getAsInteger() != 1)
                            out << 's';
                    } else {
                        assert(modifier.isEmpty() && "Improper modifier for integer argument");
                        out << arg.getAsInteger();
                    }
                    break;
                }
                case DiagnosticArg::Kind::boolean: {
                    if (modifier == "if") {
                        if (arg.getAsBoolean()) {
                            formatDiagnosticTextInto(out, modifierArguments, args);
                        }
                    } else {
                        assert(modifier.isEmpty() && "Improper modifier for boolean argument");
                        out << (arg.getAsBoolean() ? "true" : "false");
                    }
                    break;
                }
                case DiagnosticArg::Kind::string: {
                    assert(modifier.isEmpty() && "Improper modifier for string argument");
                    out << arg.getAsString();
                    break;
                }
                case DiagnosticArg::Kind::lexerToken: {
                    assert(modifier.isEmpty() && "Improper modifier for LexerToken argument");
                    out << *(arg.getAsLexerToken());
                    break;
                }
            }
        }

        void DiagnosticEngine::formatDiagnosticTextInto(std::ostream & out, basic::StringRef text,
                                                        const std::vector<DiagnosticArg> & args) {
            while (text.isNotEmpty()) {
                size_t percent = text.indexOf('%');
                if (percent == basic::StringRef::npos) {
                    out << text;
                    break;
                }

                out << text.prefix(percent);
                text = text.substr(percent + 1);

                if (text[0] == '%') {
                    out << '%';
                    text = text.dropFirst();
                    continue;
                }

                basic::StringRef modifier;
                {
                    size_t length = text.indexWhereNot(basic::isAlpha);
                    modifier = text.substr(0, length);
                    text = text.substr(length);
                }

                basic::StringRef modifierArguments;
                if (text[0] == '{') {
                    text = text.substr(1);
                    modifierArguments = skipToDelimiter(text, '}');
                }

                size_t length = text.indexWhereNot(basic::isDigit);
                assert(length > 0 && "Unparseable argument index value");

                int argIndex = std::stoi(text.prefix(length).str());
                assert(argIndex < args.size() && "Out-of-range argument index");

                text = text.substr(length);

                formatDiagnosticArgInto(out, modifier, modifierArguments, args, argIndex);
            }
        }

        DiagnosticKind DiagnosticEngine::diagnosticKindFor(const DiagnosticID id) {
            return diagnosticKinds[(unsigned)id];
        }

        const char * DiagnosticEngine::diagnosticStringFor(const DiagnosticID id) {
            return diagnosticStrings[(unsigned)id];
        }


    }
}
