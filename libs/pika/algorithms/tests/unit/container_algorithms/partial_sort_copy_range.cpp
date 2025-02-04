//  Copyright (c) 2020 Francisco Jose Tapia (fjtapia@gmail.com )
//  Copyright (c) 2021 Akhil J Nair
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <pika/assert.hpp>
#include <pika/execution.hpp>
#include <pika/init.hpp>
#include <pika/iterator_support/tests/iter_sent.hpp>
#include <pika/modules/testing.hpp>
#include <pika/parallel/container_algorithms/partial_sort_copy.hpp>

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <list>
#include <random>
#include <string>
#include <vector>

#include "test_utils.hpp"

////////////////////////////////////////////////////////////////////////////
unsigned int seed = std::random_device{}();
std::mt19937 gen(seed);

#define SIZE 1007

template <typename IteratorTag>
void test_partial_sort_range_sent(IteratorTag)
{
    using compare_t = std::less<std::uint64_t>;
    std::list<std::uint64_t> lst;
    std::vector<std::uint64_t> A, B;
    A.reserve(SIZE + 1);
    B.reserve(SIZE + 1);

    for (std::uint64_t i = 0; i <= SIZE; ++i)
    {
        A.emplace_back(i);
        B.emplace_back(i);
    }

    std::shuffle(A.begin(), A.end() - 1, gen);
    lst.insert(lst.end(), A.begin(), A.end());

    for (std::uint64_t i = 0; i <= SIZE; ++i)
    {
        A = B;

        pika::ranges::partial_sort_copy(lst.begin(),
            sentinel<std::uint64_t>{SIZE}, A.begin(),
            sentinel<std::uint64_t>{*(A.begin() + i)}, compare_t());

        for (std::uint64_t j = 0; j < i; ++j)
        {
            PIKA_ASSERT(A[j] == j);
        };
    };
}

template <typename ExPolicy, typename IteratorTag>
void test_partial_sort_range_sent(ExPolicy policy, IteratorTag)
{
    using compare_t = std::less<std::uint64_t>;
    std::list<std::uint64_t> lst;
    std::vector<std::uint64_t> A, B;
    A.reserve(SIZE + 1);
    B.reserve(SIZE + 1);

    for (std::uint64_t i = 0; i <= SIZE; ++i)
    {
        A.emplace_back(i);
        B.emplace_back(i);
    }

    std::shuffle(A.begin(), A.end() - 1, gen);
    lst.insert(lst.end(), A.begin(), A.end());

    for (std::uint64_t i = 0; i <= SIZE; ++i)
    {
        A = B;

        pika::ranges::partial_sort_copy(policy, lst.begin(),
            sentinel<std::uint64_t>{SIZE}, A.begin(),
            sentinel<std::uint64_t>{*(A.begin() + i)}, compare_t());

        for (std::uint64_t j = 0; j < i; ++j)
        {
            PIKA_ASSERT(A[j] == j);
        };
    };
}

template <typename ExPolicy, typename IteratorTag>
void test_partial_sort_range_async_sent(ExPolicy p, IteratorTag)
{
    using compare_t = std::less<std::uint64_t>;
    std::list<std::uint64_t> lst;
    std::vector<std::uint64_t> A, B;
    A.reserve(SIZE + 1);
    B.reserve(SIZE + 1);

    for (std::uint64_t i = 0; i <= SIZE; ++i)
    {
        A.emplace_back(i);
        B.emplace_back(i);
    }

    std::shuffle(A.begin(), A.end() - 1, gen);
    lst.insert(lst.end(), A.begin(), A.end());

    for (std::uint64_t i = 0; i <= SIZE; ++i)
    {
        A = B;

        auto result = pika::ranges::partial_sort_copy(p, lst.begin(),
            sentinel<std::uint64_t>{SIZE}, A.begin(),
            sentinel<std::uint64_t>{*(A.begin() + i)}, compare_t());
        result.wait();

        for (std::uint64_t j = 0; j < i; ++j)
        {
            PIKA_ASSERT(A[j] == j);
        };
    };
}

template <typename IteratorTag>
void test_partial_sort_range(IteratorTag)
{
    using compare_t = std::less<std::uint64_t>;
    std::list<std::uint64_t> lst;
    std::vector<std::uint64_t> A, B;
    A.reserve(SIZE);
    B.reserve(SIZE);

    for (std::uint64_t i = 0; i < SIZE; ++i)
    {
        A.emplace_back(i);
        B.emplace_back(i);
    }

    std::shuffle(A.begin(), A.end(), gen);
    lst.insert(lst.end(), A.begin(), A.end());

    for (std::uint64_t i = 0; i <= SIZE; ++i)
    {
        A = B;

        pika::ranges::partial_sort_copy(lst, A, compare_t());

        for (std::uint64_t j = 0; j < i; ++j)
        {
            PIKA_ASSERT(A[j] == j);
        };
    };
}

template <typename ExPolicy, typename IteratorTag>
void test_partial_sort_range(ExPolicy policy, IteratorTag)
{
    using compare_t = std::less<std::uint64_t>;
    std::list<std::uint64_t> lst;
    std::vector<std::uint64_t> A, B;
    A.reserve(SIZE);
    B.reserve(SIZE);

    for (std::uint64_t i = 0; i < SIZE; ++i)
    {
        A.emplace_back(i);
        B.emplace_back(i);
    }

    std::shuffle(A.begin(), A.end(), gen);
    lst.insert(lst.end(), A.begin(), A.end());

    for (std::uint64_t i = 0; i <= SIZE; ++i)
    {
        A = B;

        pika::ranges::partial_sort_copy(policy, lst, A, compare_t());

        for (std::uint64_t j = 0; j < i; ++j)
        {
            PIKA_ASSERT(A[j] == j);
        };
    };
}

template <typename ExPolicy, typename IteratorTag>
void test_partial_sort_range_async(ExPolicy p, IteratorTag)
{
    using compare_t = std::less<std::uint64_t>;
    std::list<std::uint64_t> lst;
    std::vector<std::uint64_t> A, B;
    A.reserve(SIZE);
    B.reserve(SIZE);

    for (std::uint64_t i = 0; i < SIZE; ++i)
    {
        A.emplace_back(i);
        B.emplace_back(i);
    }

    std::shuffle(A.begin(), A.end(), gen);
    lst.insert(lst.end(), A.begin(), A.end());

    for (std::uint64_t i = 0; i <= SIZE; ++i)
    {
        A = B;

        auto result = pika::ranges::partial_sort_copy(p, lst, A, compare_t());
        result.wait();

        for (std::uint64_t j = 0; j < i; ++j)
        {
            PIKA_ASSERT(A[j] == j);
        };
    };
}

template <typename IteratorTag>
void test_partial_sort_range()
{
    using namespace pika::execution;

    test_partial_sort_range(IteratorTag());
    test_partial_sort_range(seq, IteratorTag());
    test_partial_sort_range(par, IteratorTag());
    test_partial_sort_range(par_unseq, IteratorTag());

    test_partial_sort_range_async(seq(task), IteratorTag());
    test_partial_sort_range_async(par(task), IteratorTag());

    test_partial_sort_range_sent(IteratorTag());
    test_partial_sort_range_sent(seq, IteratorTag());
    test_partial_sort_range_sent(par, IteratorTag());
    test_partial_sort_range_sent(par_unseq, IteratorTag());

    test_partial_sort_range_async_sent(seq(task), IteratorTag());
    test_partial_sort_range_async_sent(par(task), IteratorTag());
}

void partial_sort_range_test()
{
    test_partial_sort_range<std::random_access_iterator_tag>();
    test_partial_sort_range<std::forward_iterator_tag>();
}

int pika_main(pika::program_options::variables_map& vm)
{
    if (vm.count("seed"))
        seed = vm["seed"].as<unsigned int>();

    std::cout << "using seed: " << seed << std::endl;
    gen.seed(seed);

    partial_sort_range_test();

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
