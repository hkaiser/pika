//  Copyright (c) 2017 Taeguk Kwon
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <pika/init.hpp>
#include <pika/modules/testing.hpp>

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "remove_tests.hpp"
#include "test_utils.hpp"

////////////////////////////////////////////////////////////////////////////
void remove_test()
{
    std::cout << "--- remove_test ---" << std::endl;
    test_remove<std::random_access_iterator_tag>(true);
    test_remove<std::bidirectional_iterator_tag>(true);
}

void remove_exception_test()
{
    std::cout << "--- remove_exception_test ---" << std::endl;
    test_remove_exception<std::random_access_iterator_tag>(true);
    test_remove_exception<std::bidirectional_iterator_tag>(true);
}

void remove_bad_alloc_test()
{
    std::cout << "--- remove_bad_alloc_test ---" << std::endl;
    test_remove_bad_alloc<std::random_access_iterator_tag>(true);
    test_remove_bad_alloc<std::bidirectional_iterator_tag>(true);
}

///////////////////////////////////////////////////////////////////////////////
int pika_main(pika::program_options::variables_map& vm)
{
    unsigned int seed = (unsigned int) std::time(nullptr);
    if (vm.count("seed"))
        seed = vm["seed"].as<unsigned int>();

    std::cout << "using seed: " << seed << std::endl;
    std::srand(seed);

    remove_test();
    remove_exception_test();
    remove_bad_alloc_test();

    std::cout << "Test Finish!" << std::endl;

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

    // Initialize and run pika
    pika::init_params init_args;
    init_args.desc_cmdline = desc_commandline;
    init_args.cfg = cfg;

    PIKA_TEST_EQ_MSG(pika::init(pika_main, argc, argv, init_args), 0,
        "pika main exited with non-zero status");

    return pika::util::report_errors();
}
