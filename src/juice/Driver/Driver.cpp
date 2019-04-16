// src/juice/Driver/Driver.cpp - Driver class
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#include "juice/Driver/Driver.h"

#include "juice/Driver/CompilerDriver.h"
#include "juice/Driver/ErrorDriver.h"
#include "juice/Driver/REPLDriver.h"
#include "juice/Driver/UsageDriver.h"
#include "juice/Driver/VersionDriver.h"
#include "tclap/CmdLine.h"

namespace juice {
    namespace driver {
        Driver * Driver::withArguments(std::vector<std::string> & args) {

            Driver * driver = nullptr;

            try {
                TCLAP::CmdLine cmd("", ' ', "", false);

                cmd.setExceptionHandling(false);

                TCLAP::SwitchArg helpArg("h", "help", "Displays usage information and exits", cmd);
                TCLAP::SwitchArg versionArg("v", "version", "Displays version information and exits", cmd);
                TCLAP::UnlabeledValueArg<std::string> filenameArg("filename", "The file to compile", false,
                                                                  "example.juice", "string", cmd);

                cmd.parse(args);

                bool help = helpArg.getValue();
                bool version = versionArg.getValue();

                if (help) driver = new UsageDriver(false);
                else if (version) driver = new VersionDriver;
                else {
                    if (!filenameArg.isSet()) driver = new REPLDriver();
                    else {
                        std::string filename = filenameArg.getValue();
                        driver = new CompilerDriver(filename);
                    }
                }
            } catch (TCLAP::ArgException & e) {
                driver = new ErrorDriver(e.error() + " for arg " + e.argId());
            }

            return driver;
        }
    }
}
