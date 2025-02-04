//  Copyright (c) 2014 Thomas Heller
//  Copyright (c) 2014-2015 Anton Bikineev
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <pika/config.hpp>
#include <pika/serialization/access.hpp>
#include <pika/serialization/input_archive.hpp>
#include <pika/serialization/output_archive.hpp>

namespace pika { namespace serialization {

    template <typename T>
    output_archive& operator<<(output_archive& ar, T const& t)
    {
        ar.invoke(t);
        return ar;
    }

    template <typename T>
    input_archive& operator>>(input_archive& ar, T& t)
    {
        ar.invoke(t);
        return ar;
    }

    template <typename T>
    output_archive& operator&(output_archive& ar, T const& t)    //-V524
    {
        ar.invoke(t);
        return ar;
    }

    template <typename T>
    input_archive& operator&(input_archive& ar, T& t)    //-V524
    {
        ar.invoke(t);
        return ar;
    }
}}    // namespace pika::serialization

#include <pika/serialization/detail/polymorphic_nonintrusive_factory_impl.hpp>
