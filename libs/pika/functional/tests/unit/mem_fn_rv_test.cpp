// Taken from the Boost.Bind library
//
//  mem_fn_test.cpp - mem_fn.hpp with rvalues
//
//  Copyright (c) 2001, 2002 Peter Dimov and Multi Media Ltd.
//  Copyright (c) 2005 Peter Dimov
//  Copyright (c) 2013 Agustin Berge
//
//  SPDX-License-Identifier: BSL-1.0
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include <pika/config.hpp>

#if defined(PIKA_MSVC)
#pragma warning(disable : 4786)    // identifier truncated in debug info
#pragma warning(disable : 4710)    // function not inlined
#pragma warning(                                                               \
    disable : 4711)    // function selected for automatic inline expansion
#pragma warning(disable : 4514)    // unreferenced inline removed
#endif

#include <pika/functional/mem_fn.hpp>

#include <iostream>
#include <memory>

#include <pika/modules/testing.hpp>

unsigned int hash = 0;

struct X
{
    int f0()
    {
        f1(17);
        return 0;
    }
    int g0() const
    {
        g1(17);
        return 0;
    }

    int f1(int a1)
    {
        hash = (hash * 17041 + a1) % 32768;
        return 0;
    }
    int g1(int a1) const
    {
        hash = (hash * 17041 + a1 * 2) % 32768;
        return 0;
    }

    int f2(int a1, int a2)
    {
        f1(a1);
        f1(a2);
        return 0;
    }
    int g2(int a1, int a2) const
    {
        g1(a1);
        g1(a2);
        return 0;
    }

    int f3(int a1, int a2, int a3)
    {
        f2(a1, a2);
        f1(a3);
        return 0;
    }
    int g3(int a1, int a2, int a3) const
    {
        g2(a1, a2);
        g1(a3);
        return 0;
    }

    int f4(int a1, int a2, int a3, int a4)
    {
        f3(a1, a2, a3);
        f1(a4);
        return 0;
    }
    int g4(int a1, int a2, int a3, int a4) const
    {
        g3(a1, a2, a3);
        g1(a4);
        return 0;
    }

    int f5(int a1, int a2, int a3, int a4, int a5)
    {
        f4(a1, a2, a3, a4);
        f1(a5);
        return 0;
    }
    int g5(int a1, int a2, int a3, int a4, int a5) const
    {
        g4(a1, a2, a3, a4);
        g1(a5);
        return 0;
    }

    int f6(int a1, int a2, int a3, int a4, int a5, int a6)
    {
        f5(a1, a2, a3, a4, a5);
        f1(a6);
        return 0;
    }
    int g6(int a1, int a2, int a3, int a4, int a5, int a6) const
    {
        g5(a1, a2, a3, a4, a5);
        g1(a6);
        return 0;
    }

    int f7(int a1, int a2, int a3, int a4, int a5, int a6, int a7)
    {
        f6(a1, a2, a3, a4, a5, a6);
        f1(a7);
        return 0;
    }
    int g7(int a1, int a2, int a3, int a4, int a5, int a6, int a7) const
    {
        g6(a1, a2, a3, a4, a5, a6);
        g1(a7);
        return 0;
    }

    int f8(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8)
    {
        f7(a1, a2, a3, a4, a5, a6, a7);
        f1(a8);
        return 0;
    }
    int g8(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) const
    {
        g7(a1, a2, a3, a4, a5, a6, a7);
        g1(a8);
        return 0;
    }
};

std::shared_ptr<X> make()
{
    return std::shared_ptr<X>(new X);
}

int main()
{
    pika::util::mem_fn (&X::f0)(make());
    pika::util::mem_fn (&X::g0)(make());

    pika::util::mem_fn (&X::f1)(make(), 1);
    pika::util::mem_fn (&X::g1)(make(), 1);

    pika::util::mem_fn (&X::f2)(make(), 1, 2);
    pika::util::mem_fn (&X::g2)(make(), 1, 2);

    pika::util::mem_fn (&X::f3)(make(), 1, 2, 3);
    pika::util::mem_fn (&X::g3)(make(), 1, 2, 3);

    pika::util::mem_fn (&X::f4)(make(), 1, 2, 3, 4);
    pika::util::mem_fn (&X::g4)(make(), 1, 2, 3, 4);

    pika::util::mem_fn (&X::f5)(make(), 1, 2, 3, 4, 5);
    pika::util::mem_fn (&X::g5)(make(), 1, 2, 3, 4, 5);

    pika::util::mem_fn (&X::f6)(make(), 1, 2, 3, 4, 5, 6);
    pika::util::mem_fn (&X::g6)(make(), 1, 2, 3, 4, 5, 6);

    pika::util::mem_fn (&X::f7)(make(), 1, 2, 3, 4, 5, 6, 7);
    pika::util::mem_fn (&X::g7)(make(), 1, 2, 3, 4, 5, 6, 7);

    pika::util::mem_fn (&X::f8)(make(), 1, 2, 3, 4, 5, 6, 7, 8);
    pika::util::mem_fn (&X::g8)(make(), 1, 2, 3, 4, 5, 6, 7, 8);

    PIKA_TEST_EQ(hash, 2155u);

    return pika::util::report_errors();
}
