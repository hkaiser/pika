//  Copyright (c) 2015 John Biddiscombe
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <pika/init.hpp>
#include <pika/modules/testing.hpp>

#include <iostream>
#include <string>
#include <vector>

// use smaller array sizes for debug tests
#if defined(PIKA_DEBUG)
#define PIKA_SORT_TEST_SIZE 50000
#define PIKA_SORT_TEST_SIZE_STRINGS 10000
#endif

#include "sort_tests.hpp"

///////////////////////////////////////////////////////////////////////////////
void test_exceptions()
{
    using namespace pika::execution;

    // default comparison operator (std::less)
    test_sort_exception(seq, int());
    test_sort_exception(par, int());

    // user supplied comparison operator (std::less)
    test_sort_exception(seq, int(), std::less<int>());
    test_sort_exception(par, int(), std::less<int>());

    // Async execution, default comparison operator
    test_sort_exception_async(seq(task), int());
    test_sort_exception_async(par(task), int());

    // Async execution, user comparison operator
    test_sort_exception_async(seq(task), int(), std::less<int>());
    test_sort_exception_async(par(task), int(), std::less<int>());
}

////////////////////////////////////////////////////////////////////////////////
int pika_main(pika::program_options::variables_map& vm)
{
    unsigned int seed = (unsigned int) std::time(nullptr);
    if (vm.count("seed"))
        seed = vm["seed"].as<unsigned int>();

    std::cout << "using seed: " << seed << std::endl;
    std::srand(seed);

    test_exceptions();
    return pika::finalize();
}

int main(int argc, char* argv[])
{
    // add command line option which controls the random number generator seed
    using namespace pika::program_options;
    options_description desc_commandline(
        "Usage: " PIKA_APPLICATION_STRING " [options]");

    desc_commandline.add_options()("seed,s", value<unsigned int>(),
        "the random number generator seed to use for this run");

    // By default this test should run on all available cores
    std::vector<std::string> const cfg = {"pika.os_threads=all"};

    pika::init_params init_args;
    init_args.desc_cmdline = desc_commandline;
    init_args.cfg = cfg;

    PIKA_TEST_EQ_MSG(pika::init(pika_main, argc, argv, init_args), 0,
        "pika main exited with non-zero status");

    return pika::util::report_errors();
}
