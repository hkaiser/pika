//  Copyright (c) 2021 ETH Zurich
//  Copyright (c) 2020 John Biddiscombe
//  Copyright (c) 2016 Hartmut Kaiser
//  Copyright (c) 2016 Thomas Heller
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <pika/config.hpp>
#include <pika/assert.hpp>
#include <pika/async_cuda/cuda_event.hpp>
#include <pika/async_cuda/custom_gpu_api.hpp>
#include <pika/async_cuda/detail/cuda_debug.hpp>
#include <pika/async_cuda/detail/cuda_event_callback.hpp>
#include <pika/concurrency/concurrentqueue.hpp>
#include <pika/synchronization/spinlock.hpp>
#include <pika/threading_base/scheduler_base.hpp>
#include <pika/threading_base/thread_pool_base.hpp>

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace pika { namespace cuda { namespace experimental { namespace detail {
    // this code runs on a std::thread, but we will use a spinlock
    // as we never suspend - only ever try_lock, or exit
    using mutex_type = pika::lcos::local::spinlock;
    mutex_type& get_vector_mtx()
    {
        static mutex_type vector_mtx;
        return vector_mtx;
    }

#if defined(PIKA_DEBUG)
    std::atomic<std::size_t>& get_register_polling_count()
    {
        static std::atomic<std::size_t> register_polling_count{0};
        return register_polling_count;
    }
#endif

    // Holds a CUDA event and a callback. The callback is intended to be called when
    // the event is ready.
    struct event_callback
    {
        cudaEvent_t event;
        event_callback_function_type f;
    };

    using event_callback_queue_type =
        concurrency::ConcurrentQueue<event_callback>;
    using event_callback_vector_type = std::vector<event_callback>;

    event_callback_vector_type& get_event_callback_vector()
    {
        static event_callback_vector_type event_callback_vector;
        return event_callback_vector;
    }

    event_callback_queue_type& get_event_callback_queue()
    {
        static event_callback_queue_type event_callback_queue;
        return event_callback_queue;
    }

    std::size_t get_number_of_enqueued_events()
    {
        return get_event_callback_queue().size_approx();
    }

    static std::atomic<std::size_t> active_events_counter{0};

    std::size_t get_number_of_active_events()
    {
        return active_events_counter;
    }

    void add_to_event_callback_queue(event_callback&& continuation)
    {
        PIKA_ASSERT_MSG(get_register_polling_count() != 0,
            "CUDA event polling has not been enabled on any pool. Make sure "
            "that CUDA event polling is enabled on at least one thread pool.");

        get_event_callback_queue().enqueue(PIKA_MOVE(continuation));

        cud_debug.debug(debug::str<>("event queued"), "event",
            debug::hex<8>(continuation.event), "enqueued events",
            debug::dec<3>(get_number_of_enqueued_events()), "active events",
            debug::dec<3>(get_number_of_active_events()));
    }

    void add_to_event_callback_vector(event_callback&& continuation)
    {
        get_event_callback_vector().push_back(PIKA_MOVE(continuation));
        ++active_events_counter;

        cud_debug.debug(
            debug::str<>("event callback moved from queue to vector"), "event",
            debug::hex<8>(continuation.event), "enqueued events",
            debug::dec<3>(get_number_of_enqueued_events()), "active events",
            debug::dec<3>(get_number_of_active_events()));
    }

    void add_event_callback(
        event_callback_function_type&& f, cudaStream_t stream)
    {
        cudaEvent_t event;
        if (!cuda_event_pool::get_event_pool().pop(event))
        {
            PIKA_THROW_EXCEPTION(
                invalid_status, "add_event_callback", "could not get an event");
        }
        check_cuda_error(cudaEventRecord(event, stream));

        detail::add_to_event_callback_queue(
            event_callback{event, PIKA_MOVE(f)});
    }

    // Background progress function for async CUDA operations. Checks for completed
    // cudaEvent_t and calls the associated callback when ready. We first process
    // events that have been added to the vector of events, which should be
    // processed under a lock.  After that we process events that have been added to
    // the lockfree queue. If an event from the queue is not ready it is added to
    // the vector of events for later checking.
    pika::threads::policies::detail::polling_status poll()
    {
        using pika::threads::policies::detail::polling_status;

        auto& event_callback_vector = detail::get_event_callback_vector();

        // Don't poll if another thread is already polling
        std::unique_lock<pika::cuda::experimental::detail::mutex_type> lk(
            detail::get_vector_mtx(), std::try_to_lock);
        if (!lk.owns_lock())
        {
            if (cud_debug.is_enabled())
            {
                static auto poll_deb =
                    cud_debug.make_timer(1, debug::str<>("Poll - lock failed"));
                cud_debug.timed(poll_deb, "enqueued events",
                    debug::dec<3>(get_number_of_enqueued_events()),
                    "active events",
                    debug::dec<3>(get_number_of_active_events()));
            }
            return polling_status::idle;
        }

        if (cud_debug.is_enabled())
        {
            static auto poll_deb =
                cud_debug.make_timer(1, debug::str<>("Poll - lock success"));
            cud_debug.timed(poll_deb, "enqueued events",
                debug::dec<3>(get_number_of_enqueued_events()), "active events",
                debug::dec<3>(get_number_of_active_events()));
        }

        // Grab the handle to the event pool so we can return completed events
        cuda_event_pool& pool =
            pika::cuda::experimental::cuda_event_pool::get_event_pool();

        // Iterate over our list of events and see if any have completed
        event_callback_vector.erase(
            std::remove_if(event_callback_vector.begin(),
                event_callback_vector.end(),
                [&](event_callback& continuation) {
                    cudaError_t status = cudaEventQuery(continuation.event);

                    if (status == cudaErrorNotReady)
                    {
                        return false;
                    }

                    cud_debug.debug(debug::str<>("set ready vector"), "event",
                        debug::hex<8>(continuation.event), "enqueued events",
                        debug::dec<3>(get_number_of_enqueued_events()),
                        "active events",
                        debug::dec<3>(get_number_of_active_events()));
                    continuation.f(status);
                    pool.push(PIKA_MOVE(continuation.event));
                    return true;
                }),
            event_callback_vector.end());
        active_events_counter = event_callback_vector.size();

        detail::event_callback continuation;
        while (detail::get_event_callback_queue().try_dequeue(continuation))
        {
            cudaError_t status = cudaEventQuery(continuation.event);

            if (status == cudaErrorNotReady)
            {
                add_to_event_callback_vector(PIKA_MOVE(continuation));
            }
            else
            {
                cud_debug.debug(debug::str<>("set ready queue"), "event",
                    debug::hex<8>(continuation.event), "enqueued events",
                    debug::dec<3>(get_number_of_enqueued_events()),
                    "active events",
                    debug::dec<3>(get_number_of_active_events()));
                continuation.f(status);
                pool.push(PIKA_MOVE(continuation.event));
            }
        }

        using pika::threads::policies::detail::polling_status;
        return get_event_callback_vector().empty() ? polling_status::idle :
                                                     polling_status::busy;
    }

    std::size_t get_work_count()
    {
        std::size_t work_count = get_number_of_active_events();
        work_count += get_number_of_enqueued_events();

        return work_count;
    }

    // -------------------------------------------------------------
    void register_polling(pika::threads::thread_pool_base& pool)
    {
#if defined(PIKA_DEBUG)
        ++get_register_polling_count();
#endif
        cud_debug.debug(debug::str<>("enable polling"));
        auto* sched = pool.get_scheduler();
        sched->set_cuda_polling_functions(
            &pika::cuda::experimental::detail::poll, &get_work_count);
    }

    // -------------------------------------------------------------
    void unregister_polling(pika::threads::thread_pool_base& pool)
    {
#if defined(PIKA_DEBUG)
        {
            std::unique_lock<pika::cuda::experimental::detail::mutex_type> lk(
                detail::get_vector_mtx());
            bool event_queue_empty =
                get_event_callback_queue().size_approx() == 0;
            bool event_vector_empty = get_event_callback_vector().empty();
            lk.unlock();
            PIKA_ASSERT_MSG(event_queue_empty,
                "CUDA event polling was disabled while there are unprocessed "
                "CUDA events. Make sure CUDA event polling is not disabled too "
                "early.");
            PIKA_ASSERT_MSG(event_vector_empty,
                "CUDA event polling was disabled while there are unprocessed "
                "CUDA events. Make sure CUDA event polling is not disabled too "
                "early.");
        }
#endif
        cud_debug.debug(debug::str<>("disable polling"));
        auto* sched = pool.get_scheduler();
        sched->clear_cuda_polling_function();
    }
}}}}    // namespace pika::cuda::experimental::detail
