//  Copyright (c) 2014-2016 Hartmut Kaiser
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <pika/modules/testing.hpp>
#include <pika/parallel/container_algorithms/transform_reduce.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <numeric>
#include <string>
#include <vector>

#include "test_utils.hpp"

///////////////////////////////////////////////////////////////////////////////
template <typename IteratorTag>
void test_transform_reduce_binary(IteratorTag)
{
    typedef std::vector<int>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::vector<int> c = test::random_iota<int>(1007);
    std::vector<int> d = test::random_iota<int>(1007);
    int init = std::rand() % 1007;    //-V101

    int r = pika::ranges::transform_reduce(
        iterator(std::begin(c)), iterator(std::end(c)), std::begin(d), init);

    PIKA_TEST_EQ(
        r, std::inner_product(std::begin(c), std::end(c), std::begin(d), init));
}

template <typename ExPolicy, typename IteratorTag>
void test_transform_reduce_binary(ExPolicy&& policy, IteratorTag)
{
    static_assert(pika::is_execution_policy<ExPolicy>::value,
        "pika::is_execution_policy<ExPolicy>::value");

    typedef std::vector<int>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::vector<int> c = test::random_iota<int>(1007);
    std::vector<int> d = test::random_iota<int>(1007);
    int init = std::rand() % 1007;    //-V101

    int r = pika::ranges::transform_reduce(policy, iterator(std::begin(c)),
        iterator(std::end(c)), std::begin(d), init);

    PIKA_TEST_EQ(
        r, std::inner_product(std::begin(c), std::end(c), std::begin(d), init));
}

template <typename ExPolicy, typename IteratorTag>
void test_transform_reduce_binary_async(ExPolicy&& p, IteratorTag)
{
    static_assert(pika::is_execution_policy<ExPolicy>::value,
        "pika::is_execution_policy<ExPolicy>::value");

    typedef std::vector<int>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::vector<int> c = test::random_iota<int>(1007);
    std::vector<int> d = test::random_iota<int>(1007);
    int init = std::rand() % 1007;    //-V101

    pika::future<int> fut_r = pika::ranges::transform_reduce(
        p, iterator(std::begin(c)), iterator(std::end(c)), std::begin(d), init);

    fut_r.wait();
    PIKA_TEST_EQ(fut_r.get(),
        std::inner_product(std::begin(c), std::end(c), std::begin(d), init));
}
