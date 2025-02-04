//  Copyright (C) 2012 Hartmut Kaiser
//  Copyright (C) 2008 Peter Dimov
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt

#include <pika/init.hpp>
#include <pika/modules/testing.hpp>
#include <pika/runtime.hpp>
#include <pika/thread.hpp>

#include <functional>

using pika::program_options::options_description;
using pika::program_options::variables_map;

///////////////////////////////////////////////////////////////////////////////
struct X
{
    mutable int hash;

    X()
      : hash(0)
    {
    }

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

///////////////////////////////////////////////////////////////////////////////
int pika_main(variables_map& /*vm*/)
{
    {
        X x;

        // 0
        pika::thread(&X::f0, &x).join();
        pika::thread(&X::f0, std::ref(x)).join();

        pika::thread(&X::g0, &x).join();
        pika::thread(&X::g0, x).join();
        pika::thread(&X::g0, std::ref(x)).join();

        // 1
        pika::thread(&X::f1, &x, 1).join();
        pika::thread(&X::f1, std::ref(x), 1).join();

        pika::thread(&X::g1, &x, 1).join();
        pika::thread(&X::g1, x, 1).join();
        pika::thread(&X::g1, std::ref(x), 1).join();

        // 2
        pika::thread(&X::f2, &x, 1, 2).join();
        pika::thread(&X::f2, std::ref(x), 1, 2).join();

        pika::thread(&X::g2, &x, 1, 2).join();
        pika::thread(&X::g2, x, 1, 2).join();
        pika::thread(&X::g2, std::ref(x), 1, 2).join();

        // 3
        pika::thread(&X::f3, &x, 1, 2, 3).join();
        pika::thread(&X::f3, std::ref(x), 1, 2, 3).join();

        pika::thread(&X::g3, &x, 1, 2, 3).join();
        pika::thread(&X::g3, x, 1, 2, 3).join();
        pika::thread(&X::g3, std::ref(x), 1, 2, 3).join();

        // 4
        pika::thread(&X::f4, &x, 1, 2, 3, 4).join();
        pika::thread(&X::f4, std::ref(x), 1, 2, 3, 4).join();

        pika::thread(&X::g4, &x, 1, 2, 3, 4).join();
        pika::thread(&X::g4, x, 1, 2, 3, 4).join();
        pika::thread(&X::g4, std::ref(x), 1, 2, 3, 4).join();

        // 5
        pika::thread(&X::f5, &x, 1, 2, 3, 4, 5).join();
        pika::thread(&X::f5, std::ref(x), 1, 2, 3, 4, 5).join();

        pika::thread(&X::g5, &x, 1, 2, 3, 4, 5).join();
        pika::thread(&X::g5, x, 1, 2, 3, 4, 5).join();
        pika::thread(&X::g5, std::ref(x), 1, 2, 3, 4, 5).join();

        // 6
        pika::thread(&X::f6, &x, 1, 2, 3, 4, 5, 6).join();
        pika::thread(&X::f6, std::ref(x), 1, 2, 3, 4, 5, 6).join();

        pika::thread(&X::g6, &x, 1, 2, 3, 4, 5, 6).join();
        pika::thread(&X::g6, x, 1, 2, 3, 4, 5, 6).join();
        pika::thread(&X::g6, std::ref(x), 1, 2, 3, 4, 5, 6).join();

        // 7
        pika::thread(&X::f7, &x, 1, 2, 3, 4, 5, 6, 7).join();
        pika::thread(&X::f7, std::ref(x), 1, 2, 3, 4, 5, 6, 7).join();

        pika::thread(&X::g7, &x, 1, 2, 3, 4, 5, 6, 7).join();
        pika::thread(&X::g7, x, 1, 2, 3, 4, 5, 6, 7).join();
        pika::thread(&X::g7, std::ref(x), 1, 2, 3, 4, 5, 6, 7).join();

        // 8
        pika::thread(&X::f8, &x, 1, 2, 3, 4, 5, 6, 7, 8).join();
        pika::thread(&X::f8, std::ref(x), 1, 2, 3, 4, 5, 6, 7, 8).join();

        pika::thread(&X::g8, &x, 1, 2, 3, 4, 5, 6, 7, 8).join();
        pika::thread(&X::g8, x, 1, 2, 3, 4, 5, 6, 7, 8).join();
        pika::thread(&X::g8, std::ref(x), 1, 2, 3, 4, 5, 6, 7, 8).join();

        PIKA_TEST_EQ(x.hash, 23558);
    }

    pika::finalize();
    return pika::util::report_errors();
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    // Configure application-specific options
    options_description cmdline("Usage: " PIKA_APPLICATION_STRING " [options]");

    // Initialize and run pika
    pika::init_params init_args;
    init_args.desc_cmdline = cmdline;

    return pika::init(pika_main, argc, argv, init_args);
}
