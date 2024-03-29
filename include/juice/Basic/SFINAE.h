// include/juice/Basic/SFINAE.h - Helper constructs for working with templates
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 - 2020 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_BASIC_SFINAE_H
#define JUICE_BASIC_SFINAE_H

#include <type_traits>

namespace juice {
    namespace basic {
        namespace detail {
            template <bool...> struct bool_pack;

            template <bool... booleans>
            using all_true = std::is_same<bool_pack<booleans..., true>, bool_pack<true, booleans...>>;
        }

        template <typename... Ts>
        using all_true = detail::all_true<Ts::value...>;

        template <typename T, typename... Ts>
        using all_same = all_true<std::is_same<T,Ts>...>;

        template <typename T, typename... Ts>
        constexpr bool all_same_v = all_same<T, Ts...>::value;
    }
}

#endif //JUICE_BASIC_SFINAE_H
