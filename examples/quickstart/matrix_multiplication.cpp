////////////////////////////////////////////////////////////////////////////////
//  Copyright (c) 2021 Dimitra Karatza
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
////////////////////////////////////////////////////////////////////////////////

// Parallel implementation of matrix multiplication

#include <pika/algorithm.hpp>
#include <pika/execution.hpp>
#include <pika/init.hpp>

#include <cstddef>
#include <iostream>
#include <random>
#include <vector>

std::mt19937 gen;

///////////////////////////////////////////////////////////////////////////////
//[mul_print_matrix
void print_matrix(
    std::vector<int> M, std::size_t rows, std::size_t cols, const char* message)
{
    std::cout << "\nMatrix " << message << " is:" << std::endl;
    for (std::size_t i = 0; i < rows; i++)
    {
        for (std::size_t j = 0; j < cols; j++)
            std::cout << M[i * cols + j] << " ";
        std::cout << "\n";
    }
}
//]

///////////////////////////////////////////////////////////////////////////////
//[mul_pika_main
int pika_main(pika::program_options::variables_map& vm)
{
    using element_type = int;

    // Define matrix sizes
    std::size_t rowsA = vm["n"].as<std::size_t>();
    std::size_t colsA = vm["m"].as<std::size_t>();
    std::size_t rowsB = colsA;
    std::size_t colsB = vm["k"].as<std::size_t>();
    std::size_t rowsR = rowsA;
    std::size_t colsR = colsB;

    // Initialize matrices A and B
    std::vector<int> A(rowsA * colsA);
    std::vector<int> B(rowsB * colsB);
    std::vector<int> R(rowsR * colsR);

    // Define seed
    unsigned int seed = std::random_device{}();
    if (vm.count("seed"))
        seed = vm["seed"].as<unsigned int>();

    gen.seed(seed);
    std::cout << "using seed: " << seed << std::endl;

    // Define range of values
    std::size_t lower = vm["l"].as<std::size_t>();
    std::size_t upper = vm["u"].as<std::size_t>();

    // Matrices have random values in the range [lower, upper]
    std::uniform_int_distribution<element_type> dis(lower, upper);
    auto generator = std::bind(dis, gen);
    pika::ranges::generate(A, generator);
    pika::ranges::generate(B, generator);

    // Perform matrix multiplication
    pika::for_loop(pika::execution::par, 0, rowsA, [&](auto i) {
        pika::for_loop(0, colsB, [&](auto j) {
            R[i * colsR + j] = 0;
            pika::for_loop(0, rowsB, [&](auto k) {
                R[i * colsR + j] += A[i * colsA + k] * B[k * colsB + j];
            });
        });
    });

    // Print all 3 matrices
    print_matrix(A, rowsA, colsA, "A");
    print_matrix(B, rowsB, colsB, "B");
    print_matrix(R, rowsR, colsR, "R");

    return pika::finalize();
}
//]

///////////////////////////////////////////////////////////////////////////////
//[mul_main
int main(int argc, char* argv[])
{
    using namespace pika::program_options;
    options_description cmdline("usage: " PIKA_APPLICATION_STRING " [options]");
    // clang-format off
    cmdline.add_options()
        ("n",
        pika::program_options::value<std::size_t>()->default_value(2),
        "Number of rows of first matrix")
        ("m",
        pika::program_options::value<std::size_t>()->default_value(3),
        "Number of columns of first matrix (equal to the number of rows of "
        "second matrix)")
        ("k",
        pika::program_options::value<std::size_t>()->default_value(2),
        "Number of columns of second matrix")
        ("seed,s",
        pika::program_options::value<unsigned int>(),
        "The random number generator seed to use for this run")
        ("l",
        pika::program_options::value<std::size_t>()->default_value(0),
        "Lower limit of range of values")
        ("u",
        pika::program_options::value<std::size_t>()->default_value(10),
        "Upper limit of range of values");
    // clang-format on
    pika::init_params init_args;
    init_args.desc_cmdline = cmdline;

    return pika::init(pika_main, argc, argv, init_args);
}
//]
