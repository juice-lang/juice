// include/juice/Basic/SFINAE.h - Helper constructs for working with templates
//
// This source file is part of the juice open source project
//
// Copyright (c) 2019 juice project authors
// Licensed under MIT License
//
// See https://github.com/juice-lang/juice/blob/master/LICENSE for license information
// See https://github.com/juice-lang/juice/blob/master/CONTRIBUTORS.txt for the list of juice project authors


#ifndef JUICE_SFINAE_H
#define JUICE_SFINAE_H

#include <type_traits>

namespace juice {
    namespace basic {
        namespace detail {
            template<bool...> struct bool_pack;
            template<bool... bs>
            //if any are false, they'll be shifted in the second version, so types won't match
            using all_true = std::is_same<bool_pack<bs..., true>, bool_pack<true, bs...>>;
        }

        template <typename... Ts>
        using all_true = detail::all_true<Ts::value...>;

        template <typename T, typename... Ts>
        using all_same = all_true<std::is_same<T,Ts>...>;
    }
}

#endif //JUICE_SFINAE_H
