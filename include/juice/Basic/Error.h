// include/juice/Basic/Error.h - Helper classes for working with llvm Errors
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors

#ifndef JUICE_BASIC_ERROR_H_
#define JUICE_BASIC_ERROR_H_

#include "llvm/Support/Error.h"
#include "llvm/Support/raw_ostream.h"

namespace juice {
    namespace basic {
        class ReturningError: public llvm::ErrorInfo<ReturningError> {
        public:
            static char ID;

            void log(llvm::raw_ostream & os) const override {
                os << "ReturningError";
            }

            std::error_code convertToErrorCode() const override {
                return llvm::inconvertibleErrorCode();
            }
        };
    }
}

#endif //JUICE_BASIC_ERROR_H_
