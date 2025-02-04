//  Copyright (c) 2007-2015 Hartmut Kaiser
//  Copyright (c)      2014 Thomas Heller
//  Copyright (c)      2015 Anton Bikineev
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <pika/config.hpp>
#include <pika/assert.hpp>
#include <pika/serialization/binary_filter.hpp>
#include <pika/serialization/container.hpp>
#include <pika/serialization/serialization_chunk.hpp>
#include <pika/serialization/traits/serialization_access_data.hpp>

#include <cstddef>    // for size_t
#include <cstdint>
#include <cstring>    // for memcpy
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace pika { namespace serialization {

    namespace detail {

        ///////////////////////////////////////////////////////////////////////
        struct basic_chunker
        {
            constexpr basic_chunker(std::vector<serialization_chunk>*) {}

            static constexpr std::size_t get_chunk_size()
            {
                return 0;
            }

            static void set_chunk_size(std::size_t) {}

            static constexpr std::uint8_t get_chunk_type()
            {
                return chunk_type_index;
            }

            static constexpr std::size_t get_chunk_data_index()
            {
                return 0;
            }

            static constexpr std::size_t get_num_chunks()
            {
                return 1;
            }

            static void push_back(serialization_chunk&& /*chunk*/) {}

            static void reset() {}
        };

        struct vector_chunker
        {
            vector_chunker(std::vector<serialization_chunk>* chunks)
              : chunks_(chunks)
            {
            }

            std::size_t get_chunk_size() const
            {
                return chunks_->back().size_;
            }

            void set_chunk_size(std::size_t size)
            {
                chunks_->back().size_ = size;
            }

            std::uint8_t get_chunk_type() const
            {
                return chunks_->back().type_;
            }

            std::size_t get_chunk_data_index() const
            {
                return chunks_->back().data_.index_;
            }

            std::size_t get_num_chunks() const
            {
                return chunks_->size();
            }

            void push_back(serialization_chunk&& chunk)
            {
                chunks_->push_back(PIKA_MOVE(chunk));
            }

            void reset()
            {
                chunks_->clear();
                chunks_->push_back(create_index_chunk(0, 0));
            }

            std::vector<serialization_chunk>* chunks_;
        };

        struct counting_chunker
        {
            counting_chunker(std::vector<serialization_chunk>*)
              : chunk_()
              , num_chunks_(0)
            {
            }

            std::size_t get_chunk_size() const
            {
                return chunk_.size_;
            }

            void set_chunk_size(std::size_t size)
            {
                chunk_.size_ = size;
            }

            std::uint8_t get_chunk_type() const
            {
                return chunk_.type_;
            }

            std::size_t get_chunk_data_index() const
            {
                return chunk_.data_.index_;
            }

            std::size_t get_num_chunks() const
            {
                return num_chunks_;
            }

            void push_back(serialization_chunk&& chunk)
            {
                chunk_ = PIKA_MOVE(chunk);
                ++num_chunks_;
            }

            void reset()
            {
                chunk_ = create_index_chunk(0, 0);
                num_chunks_ = 1;
            }

            serialization_chunk chunk_;
            std::size_t num_chunks_;
        };
    }    // namespace detail

    ///////////////////////////////////////////////////////////////////////////
    template <typename Container, typename Chunker>
    struct output_container : erased_output_container
    {
        using access_traits = traits::serialization_access_data<Container>;

        output_container(
            Container& cont, std::vector<serialization_chunk>* chunks = nullptr)
          : cont_(cont)
          , current_(0)
          , chunker_(chunks)
        {
            chunker_.reset();
        }

        ~output_container() = default;

        void flush() override
        {
            PIKA_ASSERT(chunker_.get_chunk_type() == chunk_type_index ||
                chunker_.get_chunk_size() != 0);

            // complement current serialization_chunk by setting its length
            if (chunker_.get_chunk_type() == chunk_type_index)
            {
                PIKA_ASSERT(chunker_.get_chunk_size() == 0);

                chunker_.set_chunk_size(
                    current_ - chunker_.get_chunk_data_index());
            }
        }

        std::size_t get_num_chunks() const override
        {
            return chunker_.get_num_chunks();
        }

        void reset() override
        {
            chunker_.reset();
            access_traits::reset(cont_);
        }

        void set_filter(binary_filter* /* filter */) override
        {
            PIKA_ASSERT(chunker_.get_num_chunks() == 1 &&
                chunker_.get_chunk_size() == 0);
            chunker_.reset();
        }

        void save_binary(void const* address, std::size_t count) override
        {
            PIKA_ASSERT(count != 0);

            // make sure there is a current serialization_chunk descriptor
            // available
            if (chunker_.get_chunk_type() == chunk_type_pointer ||
                chunker_.get_chunk_size() != 0)
            {
                // add a new serialization_chunk,
                // the chunk size will be set at the end
                chunker_.push_back(create_index_chunk(current_, 0));
            }

            std::size_t new_current = current_ + count;
            if (access_traits::size(cont_) < new_current)
                access_traits::resize(cont_, count);

            access_traits::write(cont_, count, current_, address);

            current_ = new_current;
        }

        std::size_t save_binary_chunk(
            void const* address, std::size_t count) override
        {
            if (count < PIKA_ZERO_COPY_SERIALIZATION_THRESHOLD)
            {
                // fall back to serialization_chunk-less archive
                this->output_container::save_binary(address, count);

                // the container has grown by count bytes
                return count;
            }
            else
            {
                PIKA_ASSERT(chunker_.get_chunk_type() == chunk_type_index ||
                    chunker_.get_chunk_size() != 0);

                // complement current serialization_chunk by setting its length
                if (chunker_.get_chunk_type() == chunk_type_index)
                {
                    PIKA_ASSERT(chunker_.get_chunk_size() == 0);

                    chunker_.set_chunk_size(
                        current_ - chunker_.get_chunk_data_index());
                }

                // add a new serialization_chunk referring to the external
                // buffer
                chunker_.push_back(create_pointer_chunk(address, count));
                // the container did not grow
                return 0;
            }
        }

        bool is_preprocessing() const override
        {
            return access_traits::is_preprocessing();
        }

    protected:
        Container& cont_;
        std::size_t current_;
        Chunker chunker_;
    };

    ///////////////////////////////////////////////////////////////////////////
    template <typename Container, typename Chunker>
    struct filtered_output_container : output_container<Container, Chunker>
    {
        using access_traits = traits::serialization_access_data<Container>;
        using base_type = output_container<Container, Chunker>;

        filtered_output_container(
            Container& cont, std::vector<serialization_chunk>* chunks = nullptr)
          : base_type(cont, chunks)
          , start_compressing_at_(0)
          , filter_(nullptr)
        {
        }

        ~filtered_output_container() = default;

        void flush()
        {
            std::size_t written = 0;

            if (access_traits::size(this->cont_) < this->current_)
                access_traits::resize(this->cont_, this->current_);

            this->current_ = start_compressing_at_;

            do
            {
                bool flushed = access_traits::flush(filter_, this->cont_,
                    this->current_,
                    access_traits::size(this->cont_) - this->current_, written);

                this->current_ += written;
                if (flushed)
                    break;

                // resize container
                std::size_t size = access_traits::size(this->cont_);
                access_traits::resize(this->cont_, 2 * size);

            } while (true);

            // truncate container
            access_traits::resize(this->cont_, this->current_);
        }

        void set_filter(binary_filter* filter)    // override
        {
            PIKA_ASSERT(nullptr == filter_ && filter != nullptr);
            filter_ = filter;
            start_compressing_at_ = this->current_;

            this->base_type::set_filter(nullptr);
        }

        void save_binary(void const* address, std::size_t count)    // override
        {
            PIKA_ASSERT(count != 0);

            // during construction the filter may not have been set yet
            if (filter_ != nullptr)
                filter_->save(address, count);
            this->current_ += count;
        }

        std::size_t save_binary_chunk(
            void const* address, std::size_t count)    // override
        {
            if (count < PIKA_ZERO_COPY_SERIALIZATION_THRESHOLD)
            {
                // fall back to serialization_chunk-less archive
                PIKA_ASSERT(count != 0);
                filter_->save(address, count);
                this->current_ += count;
                return count;
            }
            else
            {
                return this->base_type::save_binary_chunk(address, count);
            }
        }

    protected:
        std::size_t start_compressing_at_;
        binary_filter* filter_;
    };
}}    // namespace pika::serialization
