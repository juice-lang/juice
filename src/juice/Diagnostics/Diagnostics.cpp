// src/juice/Diagnostics/Diagnostics.cpp - DiagnosticEngine class, manages any diagnostics emitted by juice
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/Diagnostics/Diagnostics.h"

#include <string>
#include <utility>

#include "juice/Basic/StringHelpers.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/Support/FormatAdapters.h"

namespace juice {
    namespace diag {
        static const constexpr DiagnosticKind diagnosticKinds[] {
            #define ERROR(ID, Text, Newline) DiagnosticKind::error,
            #define WARNING(ID, Text, Newline) DiagnosticKind::warning,
            #define OUTPUT(ID, Text, Newline) DiagnosticKind::output,
            #include "juice/Diagnostics/Diagnostics.def"
        };

        static constexpr const char * const diagnosticStrings[] {
            #define DIAG(KIND, ID, Text, Newline) Text,
            #include "juice/Diagnostics/Diagnostics.def"
        };

        static const constexpr bool diagnosticNewlines[] {
            #define DIAG(KIND, ID, Text, Newline) Newline,
            #include "juice/Diagnostics/Diagnostics.def"
        };

        DiagnosticEngine::DiagnosticEngine(std::unique_ptr<basic::SourceManager> sourceManager):
                _sourceManager(std::move(sourceManager)) {}

        void DiagnosticEngine::diagnose(basic::SourceLocation location, DiagnosticID id,
                                        const std::vector<DiagnosticArg> & args) {
            DiagnosticKind kind = diagnosticKindFor(id);
            llvm::StringRef text(diagnosticStringFor(id));
            bool newline = diagnosticNewlineFor(id);

            if (kind == DiagnosticKind::error) _hadError = true;


            std::string message;
            llvm::raw_string_ostream os(message);

            os << basic::Color::bold;

            formatDiagnosticTextInto(os, text, args, this);

            os << basic::Color::reset;

            if (newline) os << '\n';

            os.flush();

            if (kind == DiagnosticKind::output) llvm::outs() << message;
            else {
                _sourceManager->printDiagnostic(message, kind, location);
            }
        }

        void DiagnosticEngine::diagnose(DiagnosticID id, const std::vector<DiagnosticArg> & args) {
            DiagnosticKind kind = diagnosticKindFor(id);
            llvm::StringRef text(diagnosticStringFor(id));
            bool newline = diagnosticNewlineFor(id);

            std::string message;

            llvm::raw_string_ostream os(message);

            os << basic::Color::bold;

            switch (kind) {
                case DiagnosticKind::error: {
                    os << basic::Color::yellow << "juice: " << basic::Color::red << "error: " << basic::Color::reset
                       << basic::Color::bold;
                    break;
                }
                case DiagnosticKind::warning: {
                    os << basic::Color::yellow << "juice: " << basic::Color::magenta << "error: " << basic::Color::reset
                       << basic::Color::bold;
                    break;
                }
                case DiagnosticKind::output:
                    break;
            }

            formatDiagnosticTextInto(os, text, args, nullptr);

            os << basic::Color::reset;

            if (newline) os << '\n';

            os.flush();

            (kind == DiagnosticKind::error ? llvm::errs() : llvm::outs()) << message;
        }

        llvm::StringRef
        DiagnosticEngine::skipToDelimiter(llvm::StringRef & text, char delimiter, bool * foundDelimiter) {
            unsigned int depth = 0;
            if (foundDelimiter) *foundDelimiter = false;

            unsigned int i = 0;
            for (unsigned int n = text.size(); i != n; ++i) {
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
            llvm::StringRef result = text.substr(0, i);
            text = text.substr(i + 1);
            return result;
        }

        void DiagnosticEngine::formatSelectionArgInto(llvm::raw_ostream & out, llvm::StringRef modifierArguments,
                                                      const std::vector<DiagnosticArg> & args,
                                                      unsigned int selectedIndex) {
            bool foundPipe = false;
            do {
                assert((!modifierArguments.empty() || foundPipe) && "Index beyond bounds in %select modifier");
                llvm::StringRef text = skipToDelimiter(modifierArguments, '|', &foundPipe);
                if (selectedIndex == 0) {
                    formatDiagnosticTextInto(out, text, args, nullptr);
                    break;
                }
                --selectedIndex;
            } while (true);
        }

        void DiagnosticEngine::formatDiagnosticArgInto(llvm::raw_ostream & out, llvm::StringRef modifier,
                                                       llvm::StringRef modifierArguments,
                                                       const std::vector<DiagnosticArg> & args, unsigned int argIndex,
                                                       DiagnosticEngine * diagnostics) {
            if (modifier == "reset") {
                out << basic::Color::reset << basic::Color::bold;
                return;
            }
            DiagnosticArg arg = args[argIndex];
            switch (arg.getKind()) {
                case DiagnosticArg::Kind::integer: {
                    if (modifier == "indent") {
                        out << llvm::formatv("{0}", llvm::fmt_repeat("    ", arg.getAsInteger()));
                    } else if (modifier == "select") {
                        formatSelectionArgInto(out, modifierArguments, args, arg.getAsInteger());
                    } else if (modifier == "s") {
                        if (arg.getAsInteger() != 1)
                            out << 's';
                    } else {
                        assert(modifier.empty() && "Improper modifier for integer argument");
                        out << arg.getAsInteger();
                    }
                    break;
                }
                case DiagnosticArg::Kind::doubleValue: {
                    assert(modifier.empty() && "Improper modifier for double argument");
                    out << llvm::formatv("{0}", arg.getAsDouble());
                    break;
                }
                case DiagnosticArg::Kind::boolean: {
                    if (modifier == "if") {
                        if (arg.getAsBoolean()) {
                            formatDiagnosticTextInto(out, modifierArguments, args, diagnostics);
                        }
                    } else {
                        assert(modifier.empty() && "Improper modifier for boolean argument");
                        out << (arg.getAsBoolean() ? "true" : "false");
                    }
                    break;
                }
                case DiagnosticArg::Kind::string: {
                    assert(modifier.empty() && "Improper modifier for string argument");
                    out << arg.getAsString();
                    break;
                }
                case DiagnosticArg::Kind::lexerToken: {
                    assert(modifier.empty() && "Improper modifier for LexerToken argument");
                    out << arg.getAsLexerToken()
                        << (diagnostics != nullptr ? diagnostics->_sourceManager.get() : nullptr);
                    break;
                }
                case DiagnosticArg::Kind::color: {
                    assert(modifier.empty() && "Improper modifier for Color argument");
                    out << basic::Color::bold << arg.getAsColor();
                    break;
                }
                case DiagnosticArg::Kind::type: {
                    assert(modifier.empty() && "Improper modifier for Type argument");
                    out << arg.getAsType();
                    break;
                }
            }
        }

        void DiagnosticEngine::formatDiagnosticTextInto(llvm::raw_ostream & out, llvm::StringRef text,
                                                        const std::vector<DiagnosticArg> & args,
                                                        DiagnosticEngine * diagnostics) {
            while (!text.empty()) {
                size_t percent = text.find('%');
                if (percent == llvm::StringRef::npos) {
                    out << text;
                    break;
                }

                out << text.take_front(percent);
                text = text.substr(percent + 1);

                if (text[0] == '%') {
                    out << '%';
                    text = text.drop_front();
                    continue;
                }

                llvm::StringRef modifier;
                {
                    size_t length = text.find_if_not(basic::isAlpha);
                    modifier = text.substr(0, length);
                    text = text.substr(length);
                }

                llvm::StringRef modifierArguments;
                if (text[0] == '{') {
                    text = text.substr(1);
                    modifierArguments = skipToDelimiter(text, '}');
                }

                unsigned int argIndex;

                if (modifier != "reset") {
                    size_t length = text.find_if_not(basic::isDigit);
                    bool Result = text.substr(0, length).getAsInteger(10, argIndex);
                    assert(!Result && "Unparseable argument index value?");
                    (void)Result;
                    assert(argIndex < args.size() && "Out-of-range argument index");

                    text = text.substr(length);
                } else argIndex = 0;

                formatDiagnosticArgInto(out, modifier, modifierArguments, args, argIndex, diagnostics);
            }
        }

        constexpr DiagnosticKind DiagnosticEngine::diagnosticKindFor(DiagnosticID id) {
            return diagnosticKinds[(unsigned int)id];
        }

        constexpr const char * DiagnosticEngine::diagnosticStringFor(const DiagnosticID id) {
            return diagnosticStrings[(unsigned int)id];
        }

        constexpr bool DiagnosticEngine::diagnosticNewlineFor(DiagnosticID id) {
            return diagnosticNewlines[(unsigned int)id];
        }
    }
}
