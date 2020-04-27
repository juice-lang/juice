// src/juice/Parser/FSM.cpp - finite state machine classes for lexing string and number literals
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/Parser/FSM.h"

#include <utility>

#include "juice/Basic/StringHelpers.h"

namespace juice {
    namespace parser {
        static const FSM::StateReturn acceptedState = {false, FSM::noNextState};
        static const FSM::StateReturn errorState = {true, FSM::noNextState};

        FSM::Return FSM::run(const char * start, const char * end, FSM::State initialState) {
            const char * current = start;
            const char * error = nullptr;
            State currentState = initialState;

            while (!((current == end && current[-1] == '\n') || current > end)) {
                StateReturn result = currentState((current == end && current[-1] != '\n') ? "\n" : current);
                if (result.error && error == nullptr) error = current;
                if (result.next == &FSM::noNextState) {
                    return {error, (size_t)(current - start), currentState};
                }

                current++;
                currentState = result.next;
            }

            return {current, (size_t)(current - start), currentState};
        }

        FSM::StateReturn FSM::noNextState(const char *) {
            return errorState;
        }


        FSM::Return NumberFSM::run(const char * start, const char * end) {
            return FSM::run(start, end, begin);
        }

        FSM::StateReturn NumberFSM::begin(const char * current) {
            if (basic::isDigit(*current)) return {false, integer};
            return errorState;
        }

        FSM::StateReturn NumberFSM::integer(const char * current) {
            if (basic::isDigit(*current)) return {false, integer};

            if (*current == '.') {
                if (*(current + 1) == '.') return acceptedState;
                return {false, beginDecimal};
            }

            if (basic::toLower(*current) == 'e') return {false, beginExponent};
            return acceptedState;
        }

        FSM::StateReturn NumberFSM::beginDecimal(const char * current) {
            if (basic::isDigit(*current)) return {false, decimal};
            return errorState;
        }

        FSM::StateReturn NumberFSM::decimal(const char * current) {
            if (basic::isDigit(*current)) return {false, decimal};

            if (basic::toLower(*current) == 'e') return {false, beginExponent};
            return acceptedState;
        }

        FSM::StateReturn NumberFSM::beginExponent(const char * current) {
            if (*current == '+' || *current == '-')  return {false, beginSignedExponent};

            if (basic::isDigit(*current)) return {false, decimalWithExponent};
            return errorState;
        }

        FSM::StateReturn NumberFSM::beginSignedExponent(const char * current) {
            if (basic::isDigit(*current)) return {false, decimalWithExponent};
            return errorState;
        }

        FSM::StateReturn NumberFSM::decimalWithExponent(const char * current) {
            if (basic::isDigit(*current)) return {false, decimalWithExponent};
            return acceptedState;
        }


        FSM::Return StringFSM::run(const char * start, const char * end) {
            return FSM::run(start, end, begin);
        }

        FSM::StateReturn StringFSM::begin(const char * current) {
            if (*current == '"') return {false, string};
            return errorState;
        }

        FSM::StateReturn StringFSM::string(const char * current) {
            if (*current == '\\') return {false, escape};
            if (*current == '\n') return errorState;
            if (*current == '"') return {false, end};
            return {false, string};
        }

        FSM::StateReturn StringFSM::escape(const char * current) {
            switch (*current) {
                case '0':
                case '\\':
                case 't':
                case 'n':
                case 'r':
                case '"':
                case '\'':
                    return {false, string};
                default:
                    return {true, invalidEscape};
            }
        }

        FSM::StateReturn StringFSM::invalidEscape(const char * current) {
            if (*current == '\n') return errorState;
            if (*current == '"') return {false, invalidEscapeEnd};
            return {false, invalidEscape};
        }

        FSM::StateReturn StringFSM::end(const char *) {
            return acceptedState;
        }

        FSM::StateReturn StringFSM::invalidEscapeEnd(const char * current) {
            return errorState;
        }
    }
}
