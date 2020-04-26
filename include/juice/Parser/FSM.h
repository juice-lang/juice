// include/juice/Parser/FSM.h - finite state machine classes for lexing string and number literals
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_PARSER_FSM_H
#define JUICE_PARSER_FSM_H

#include <cstddef>

namespace juice {
    namespace parser {
        class FSM {
        public:
            struct StateReturn;

            typedef StateReturn (*State)(const char *);

            struct StateReturn {
                bool error;
                State next;
            };

            struct Return {
                const char * error;
                size_t length;
                State state;
            };

            static Return run(const char * start, const char * end, State initialState);

            static StateReturn noNextState(const char *);
        };

        class NumberFSM: public FSM {
        public:
            static Return run(const char * start, const char * end);

            static StateReturn begin(const char * current);
            static StateReturn integer(const char * current);
            static StateReturn beginDecimal(const char * current);
            static StateReturn decimal(const char * current);
            static StateReturn beginExponent(const char * current);
            static StateReturn beginSignedExponent(const char * current);
            static StateReturn decimalWithExponent(const char * current);
        };

        class StringFSM: public FSM {
        public:
            static Return run(const char * start, const char * end);

            static StateReturn begin(const char * current);
            static StateReturn string(const char * current);
            static StateReturn escape(const char * current);
            static StateReturn invalidEscape(const char * current);
            static StateReturn end(const char *);
            static StateReturn invalidEscapeEnd(const char * current);
        };
    }
}

#endif //JUICE_PARSER_FSM_H
