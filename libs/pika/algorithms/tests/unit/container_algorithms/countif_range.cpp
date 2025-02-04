//  Copyright (c) 2018 Christopher Ogle

//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <pika/init.hpp>
#include <pika/modules/testing.hpp>
#include <pika/parallel/container_algorithms/count.hpp>
#include <pika/parallel/util/projection_identity.hpp>

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <random>
#include <string>
#include <vector>

#include "test_utils.hpp"

struct user_defined_struct
{
    user_defined_struct() = default;
    user_defined_struct(int val)
      : val(val)
    {
    }

    ~user_defined_struct() = default;

    bool operator==(const int& rhs_val) const
    {
        return val == rhs_val;
    }

    bool operator==(const user_defined_struct& rhs) const
    {
        return val == rhs.val;
    }

    int val;
};

struct random_fill
{
    random_fill(int rand_base, int range)
      : gen(std::rand())
      , dist(rand_base - range / 2, rand_base + range / 2)
    {
    }

    int operator()()
    {
        return dist(gen);
    }

    std::mt19937 gen;
    std::uniform_int_distribution<> dist;
};

template <typename IteratorTag, typename DataType>
void test_count(IteratorTag, DataType)
{
    std::vector<DataType> c{10007};
    std::generate(std::begin(c), std::end(c), random_fill(0, 20));

    auto countif_lambda = [](const DataType& ele) { return ele == 10; };

    auto result = pika::ranges::count_if(c, countif_lambda);
    auto expected = std::count_if(std::begin(c), std::end(c), countif_lambda);

    PIKA_TEST_EQ(expected, result);
}

template <typename ExPolicy, typename IteratorTag, typename DataType>
void test_count(ExPolicy&& policy, IteratorTag, DataType)
{
    static_assert(pika::is_execution_policy<ExPolicy>::value,
        "pika::is_execution_policy<ExPolicy>::value");

    std::vector<DataType> c{10007};
    std::generate(std::begin(c), std::end(c), random_fill(0, 20));

    auto countif_lambda = [](const DataType& ele) { return ele == 10; };

    auto result = pika::ranges::count_if(policy, c, countif_lambda);
    auto expected = std::count_if(std::begin(c), std::end(c), countif_lambda);

    PIKA_TEST_EQ(expected, result);
}

template <typename ExPolicy, typename IteratorTag, typename DataType>
void test_count_async(ExPolicy&& policy, IteratorTag, DataType)
{
    static_assert(pika::is_execution_policy<ExPolicy>::value,
        "pika::is_execution_policy<ExPolicy>::value");

    std::vector<DataType> c{10007};
    std::generate(std::begin(c), std::end(c), random_fill(0, 20));

    auto countif_lambda = [](const DataType& ele) { return ele == 10; };

    auto f = pika::ranges::count_if(policy, c, countif_lambda);
    auto result = f.get();

    auto expected = std::count_if(std::begin(c), std::end(c), countif_lambda);

    PIKA_TEST_EQ(expected, result);
}

template <typename IteratorTag, typename DataType>
void test_count()
{
    using namespace pika::execution;

    test_count(IteratorTag(), DataType());

    test_count(seq, IteratorTag(), DataType());
    test_count(par, IteratorTag(), DataType());
    test_count(par_unseq, IteratorTag(), DataType());

    test_count_async(seq(task), IteratorTag(), DataType());
    test_count_async(par(task), IteratorTag(), DataType());
}

void count_test()
{
    test_count<std::random_access_iterator_tag, int>();
    test_count<std::random_access_iterator_tag, user_defined_struct>();
}

///////////////////////////////////////////////////////////////////////////////
int pika_main(pika::program_options::variables_map& vm)
{
    unsigned int seed = (unsigned int) std::time(nullptr);
    if (vm.count("seed"))
        seed = vm["seed"].as<unsigned int>();

    std::cout << "using seed: " << seed << std::endl;
    std::srand(seed);

    count_test();
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
