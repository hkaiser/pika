//  Copyright (c) 2017 Hartmut Kaiser
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <pika/serialization/detail/serialize_collection.hpp>
#include <pika/serialization/serialization_fwd.hpp>

#include <cstdint>
#include <deque>

namespace pika { namespace serialization {

    template <typename T, typename Allocator>
    void serialize(input_archive& ar, std::deque<T, Allocator>& d, unsigned)
    {
        // normal load ...
        std::uint64_t size;
        ar >> size;    //-V128
        if (size == 0)
            return;

        detail::load_collection(ar, d, size);
    }

    template <typename T, typename Allocator>
    void serialize(
        output_archive& ar, std::deque<T, Allocator> const& d, unsigned)
    {
        // normal save ...
        std::uint64_t size = d.size();
        ar << size;
        if (d.empty())
            return;

        detail::save_collection(ar, d);
    }
}}    // namespace pika::serialization
