//  Taken from the Boost.Bind library
//
//  bind_dm_test.cpp - data members
//
//  Copyright (c) 2005 Peter Dimov
//  Copyright (c) 2013 Agustin Berge
//
//  SPDX-License-Identifier: BSL-1.0
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#if defined(PIKA_MSVC)
#pragma warning(disable : 4786)    // identifier truncated in debug info
#pragma warning(disable : 4710)    // function not inlined
#pragma warning(                                                               \
    disable : 4711)    // function selected for automatic inline expansion
#pragma warning(disable : 4514)    // unreferenced inline removed
#endif

#include <pika/functional/bind.hpp>

namespace placeholders = pika::util::placeholders;

#include <functional>
#include <iostream>

#include <pika/modules/testing.hpp>

struct X
{
    int m;
};

X f(int v)
{
    X r = {v};
    return r;
}

int main()
{
    X x = {17041};
    X* px = &x;

    PIKA_TEST_EQ(pika::util::bind(&X::m, placeholders::_1)(x), 17041);
    PIKA_TEST_EQ(pika::util::bind(&X::m, placeholders::_1)(px), 17041);

    PIKA_TEST_EQ(pika::util::bind(&X::m, x)(), 17041);
    PIKA_TEST_EQ(pika::util::bind(&X::m, px)(), 17041);
    PIKA_TEST_EQ(pika::util::bind(&X::m, std::ref(x))(), 17041);

    X const cx = x;
    X const* pcx = &cx;

    PIKA_TEST_EQ(pika::util::bind(&X::m, placeholders::_1)(cx), 17041);
    PIKA_TEST_EQ(pika::util::bind(&X::m, placeholders::_1)(pcx), 17041);

    PIKA_TEST_EQ(pika::util::bind(&X::m, cx)(), 17041);
    PIKA_TEST_EQ(pika::util::bind(&X::m, pcx)(), 17041);
    PIKA_TEST_EQ(pika::util::bind(&X::m, std::ref(cx))(), 17041);

    return pika::util::report_errors();
}
