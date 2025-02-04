//  Copyright (c) 2016 Hartmut Kaiser
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <pika/config.hpp>

#include <pika/allocator_support/internal_allocator.hpp>
#include <pika/assert.hpp>
#include <pika/async_cuda/target.hpp>
#include <pika/futures/traits/future_access.hpp>
#include <pika/modules/errors.hpp>
#include <pika/threading_base/thread_helpers.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <utility>

#include <pika/async_cuda/custom_gpu_api.hpp>

namespace pika { namespace cuda { namespace experimental {
    void target::native_handle_type::init_processing_units()
    {
        cudaDeviceProp props;
        cudaError_t error = cudaGetDeviceProperties(&props, device_);
        if (error != cudaSuccess)
        {
            // report error
            PIKA_THROW_EXCEPTION(kernel_error, "cuda::init_processing_unit()",
                std::string("cudaGetDeviceProperties failed: ") +
                    cudaGetErrorString(error));
        }

        std::size_t mp = props.multiProcessorCount;
        switch (props.major)
        {
        case 2:    // Fermi
            if (props.minor == 1)
            {
                mp = mp * 48;
            }
            else
            {
                mp = mp * 32;
            }
            break;
        case 3:    // Kepler
            mp = mp * 192;
            break;
        case 5:    // Maxwell
            mp = mp * 128;
            break;
        case 6:    // Pascal
            mp = mp * 64;
            break;
        default:
            break;
        }
        processing_units_ = mp;
        processor_family_ = props.major;
        processor_name_ = props.name;
    }

    target::native_handle_type::native_handle_type(int device)
      : device_(device)
      , stream_(0)
    {
        init_processing_units();
    }

    target::native_handle_type::~native_handle_type()
    {
        reset();
    }

    void target::native_handle_type::reset() noexcept
    {
        if (stream_)
        {
            cudaError_t err = cudaStreamDestroy(stream_);    // ignore error
            PIKA_UNUSED(err);
        }
    }

    target::native_handle_type::native_handle_type(
        target::native_handle_type const& rhs) noexcept
      : device_(rhs.device_)
      , processing_units_(rhs.processing_units_)
      , processor_family_(rhs.processor_family_)
      , processor_name_(rhs.processor_name_)
      , stream_(0)
    {
    }

    target::native_handle_type::native_handle_type(
        target::native_handle_type&& rhs) noexcept
      : device_(rhs.device_)
      , processing_units_(rhs.processing_units_)
      , processor_family_(rhs.processor_family_)
      , processor_name_(rhs.processor_name_)
      , stream_(rhs.stream_)
    {
        rhs.stream_ = 0;
    }

    target::native_handle_type& target::native_handle_type::operator=(
        target::native_handle_type const& rhs) noexcept
    {
        if (this == &rhs)
            return *this;

        device_ = rhs.device_;
        processing_units_ = rhs.processing_units_;
        processor_family_ = rhs.processor_family_;
        processor_name_ = rhs.processor_name_;
        reset();

        return *this;
    }

    target::native_handle_type& target::native_handle_type::operator=(
        target::native_handle_type&& rhs) noexcept
    {
        if (this == &rhs)
            return *this;

        device_ = rhs.device_;
        processing_units_ = rhs.processing_units_;
        processor_family_ = rhs.processor_family_;
        processor_name_ = rhs.processor_name_;
        stream_ = rhs.stream_;
        rhs.stream_ = 0;

        return *this;
    }

    cudaStream_t target::native_handle_type::get_stream() const
    {
        std::lock_guard<mutex_type> l(mtx_);

        if (stream_ == 0)
        {
            cudaError_t error = cudaSetDevice(device_);
            if (error != cudaSuccess)
            {
                PIKA_THROW_EXCEPTION(kernel_error,
                    "cuda::experimental::target::native_handle::get_stream()",
                    std::string("cudaSetDevice failed: ") +
                        cudaGetErrorString(error));
            }
            error = cudaStreamCreateWithFlags(&stream_, cudaStreamNonBlocking);
            if (error != cudaSuccess)
            {
                PIKA_THROW_EXCEPTION(kernel_error,
                    "cuda::experimental::target::native_handle::get_stream()",
                    std::string("cudaStreamCreate failed: ") +
                        cudaGetErrorString(error));
            }
        }
        return stream_;
    }

    ///////////////////////////////////////////////////////////////////////////
    void target::synchronize() const
    {
        cudaStream_t stream = handle_.get_stream();

        if (stream == 0)
        {
            PIKA_THROW_EXCEPTION(invalid_status,
                "cuda::experimental::target::synchronize",
                "no stream available");
        }

        cudaError_t error = cudaStreamSynchronize(stream);
        if (error != cudaSuccess)
        {
            PIKA_THROW_EXCEPTION(kernel_error,
                "cuda::experimental::target::synchronize",
                std::string("cudaStreamSynchronize failed: ") +
                    cudaGetErrorString(error));
        }
    }

    pika::future<void> target::get_future_with_event() const
    {
        return detail::get_future_with_event(handle_.get_stream());
    }

    pika::future<void> target::get_future_with_callback() const
    {
        return detail::get_future_with_callback(handle_.get_stream());
    }

    target& get_default_target()
    {
        static target target_;
        return target_;
    }
}}}    // namespace pika::cuda::experimental
