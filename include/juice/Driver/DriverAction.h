// include/juice/Driver/DriverAction.h - The main action that the driver should perform
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2021 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_DRIVER_DRIVERACTION_H
#define JUICE_DRIVER_DRIVERACTION_H

#include <cstdint>

#include "llvm/ADT/Optional.h"
#include "llvm/ADT/SmallString.h"

namespace juice {
    namespace driver {
        struct DriverAction {
            enum Kind: uint8_t {
                dumpParse,
                dumpAST,
                emitIR,
                emitObject,
                emitExecutable
            };

            DriverAction(): _kind(Kind::emitExecutable) {}
            // NOLINTNEXTLINE(google-explicit-constructor)
            /* implicit */ constexpr DriverAction(Kind kind): _kind(kind) {}
            // NOLINTNEXTLINE(google-explicit-constructor)
            /* implicit */ constexpr DriverAction(uint8_t value): _kind(static_cast<Kind>(value)) {}

            // NOLINTNEXTLINE(google-explicit-constructor)
            /* implicit */ constexpr operator Kind() const { return _kind; }

            constexpr bool operator==(DriverAction other) const { return _kind == other._kind; }
            constexpr bool operator==(Kind other) const { return _kind == other; }
            constexpr bool operator!=(DriverAction other) const { return _kind != other._kind; }
            constexpr bool operator!=(Kind other) const { return _kind != other; }


            llvm::Optional<llvm::SmallString<128>>
            outputFile(llvm::StringRef inputFilename, llvm::StringRef outputFilename) const;

        private:
            Kind _kind;
        };
    }
}

#endif //JUICE_DRIVER_DRIVERACTION_H
