// include/juice/Basic/Error.h - Helpers for working with llvm Errors
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

#include <memory>
#include <type_traits>

#include "llvm/Support/Error.h"
#include "llvm/Support/raw_ostream.h"

namespace juice {
    namespace basic {
        class DummyError: public llvm::ErrorInfo<DummyError> {
        public:
            static const char ID;

            void log(llvm::raw_ostream & os) const override {
                os << "DummyError";
            }

            std::error_code convertToErrorCode() const override {
                return llvm::inconvertibleErrorCode();
            }
        };

        class AlreadyHandledError: public llvm::ErrorInfo<AlreadyHandledError> {
        public:
            static const char ID;

            void log(llvm::raw_ostream & os) const override {
                os << "AlreadyHandledError";
            }

            std::error_code convertToErrorCode() const override {
                return llvm::inconvertibleErrorCode();
            }
        };


        template <typename Error, typename... Args>
        llvm::Error createError(Args &&... args) {
            return llvm::Error(Error::create(std::forward<Args>(args)...));
        }


        template <typename Handler>
        class ErrorHandlerTraits:
            public ErrorHandlerTraits<decltype(&std::remove_reference<Handler>::type::operator())> {};

        template <typename Error>
        class ErrorHandlerTraits<void (&)(Error &)> {
        public:
            typedef Error ErrorType;
        };

        template <typename Error>
        class ErrorHandlerTraits<void (&)(std::unique_ptr<Error>)> {
        public:
            typedef Error ErrorType;
        };

        template <typename Class, typename Error>
        class ErrorHandlerTraits<void (Class::*)(Error &)>: public ErrorHandlerTraits<void (&)(Error &)> {};

        template <typename Class, typename Error>
        class ErrorHandlerTraits<void (Class::*)(Error &) const>: public ErrorHandlerTraits<void (&)(Error &)> {};

        template <typename Class, typename Error>
        class ErrorHandlerTraits<void (Class::*)(const Error &)>: public ErrorHandlerTraits<void (&)(Error &)> {};

        template <typename Class, typename Error>
        class ErrorHandlerTraits<void (Class::*)(const Error &) const>: public ErrorHandlerTraits<void (&)(Error &)> {};

        template <typename Class, typename Error>
        class ErrorHandlerTraits<void (Class::*)(std::unique_ptr<Error>)>:
            public ErrorHandlerTraits<void (&)(std::unique_ptr<Error>)> {};

        template <typename Class, typename Error>
        class ErrorHandlerTraits<void (Class::*)(std::unique_ptr<Error>) const>:
            public ErrorHandlerTraits<void (&)(std::unique_ptr<Error>)> {};

        template <typename... Handlers>
        bool handleAllErrors(llvm::Error error, Handlers &&... handlers) {
            auto dummyError =
                llvm::handleErrors(std::move(error), ([&](typename ErrorHandlerTraits<Handlers>::ErrorType & errors) {
                     handlers(errors);
                     return llvm::make_error<basic::DummyError>();
                })..., [](AlreadyHandledError & error) {
                    return llvm::make_error<basic::DummyError>();
                });
            if (dummyError) {
                llvm::consumeError(std::move(dummyError));
                return true;
            } else {
                return false;
            }
        }
    }
}

#endif //JUICE_BASIC_ERROR_H_
