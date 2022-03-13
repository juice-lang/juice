// include/juice/Platform/Macros.h - Defines macros for easily recognizing the platform we are running on
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2022 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_PLATFORM_MACROS_H
#define JUICE_PLATFORM_MACROS_H

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    #define OS_WINDOWS 1
    #define OS_MAC 0
    #define OS_LINUX 0
#elif __APPLE__
    #include <TargetConditionals.h>
    #if TARGET_IPHONE_SIMULATOR || TARGET_OS_MACCATALYST || TARGET_OS_IPHONE
        #error "Unsupported Apple platform"
    #elif TARGET_OS_MAC
        #define OS_WINDOWS 0
        #define OS_MAC 1
        #define OS_LINUX 0
    #else
        #error "Unknown Apple platform"
    #endif
#elif __linux__
    #define OS_WINDOWS 0
    #define OS_MAC 0
    #define OS_LINUX 1
#else
    #error "Unsupported platform"
#endif

#endif //JUICE_PLATFORM_MACROS_H
