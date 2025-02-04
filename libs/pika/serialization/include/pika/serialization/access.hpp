//  Copyright (c) 2014 Thomas Heller
//  Copyright (c) 2014-2015 Anton Bikineev
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <pika/config.hpp>
#include <pika/serialization/brace_initializable_fwd.hpp>
#include <pika/serialization/serialization_fwd.hpp>
#include <pika/serialization/traits/brace_initializable_traits.hpp>
#include <pika/serialization/traits/polymorphic_traits.hpp>

#include <string>
#include <type_traits>
#include <utility>

namespace pika { namespace serialization {

    namespace detail {

        template <typename T>
        PIKA_FORCEINLINE void serialize_force_adl(
            output_archive& ar, const T& t, unsigned)
        {
            serialize(ar, const_cast<T&>(t), 0);
        }

        template <class T>
        PIKA_FORCEINLINE void serialize_force_adl(
            input_archive& ar, T& t, unsigned)
        {
            serialize(ar, t, 0);
        }
    }    // namespace detail

    ///////////////////////////////////////////////////////////////////////////
    // This trait must live outside of 'class access' below as otherwise MSVC
    // will find the serialize() function in 'class access' as a dependent class
    // (which is an MS extension)
    template <typename T>
    class has_serialize_adl
    {
        template <typename T1>
        static std::false_type test(...);

        template <typename T1,
            typename = decltype(
                serialize(std::declval<pika::serialization::output_archive&>(),
                    std::declval<typename std::remove_const<T1>::type&>(), 0u))>
        static std::true_type test(int);

    public:
        static constexpr bool value = decltype(test<T>(0))::value;
    };

    ///////////////////////////////////////////////////////////////////////////
    template <typename T, typename Enable = void>
    struct serialize_non_intrusive
    {
        template <typename T1>
        struct dependent_false : std::false_type
        {
        };

        static_assert(
            dependent_false<T>::value, "No serialization method found");
    };

    template <typename T>
    struct serialize_non_intrusive<T,
        std::enable_if_t<has_serialize_adl<T>::value>>
    {
        template <typename Archive>
        static void call(Archive& ar, T& t, unsigned)
        {
            // this additional indirection level is needed to
            // force ADL on the second phase of template lookup.
            // call of serialize function directly from base_object
            // finds only serialize-member function and doesn't
            // perform ADL
            detail::serialize_force_adl(ar, t, 0);
        }
    };

    template <typename T>
    class has_struct_serialization
    {
    public:
        template <typename T1>
        static std::false_type test(...);

        template <typename T1,
            typename = decltype(serialize_struct(
                std::declval<pika::serialization::output_archive&>(),
                std::declval<typename std::remove_const<T1>::type&>(), 0u,
                pika::traits::detail::arity<T1>()))>
        static std::true_type test(int);

    public:
        static constexpr bool value = decltype(test<T>(0))::value;
    };

    ///////////////////////////////////////////////////////////////////////
    template <typename T, typename Enable = void>
    struct serialize_brace_initialized
    {
        template <typename T1>
        struct dependent_false : std::false_type
        {
        };

        static_assert(
            dependent_false<T>::value, "No serialization method found");
    };

    template <typename T>
    struct serialize_brace_initialized<T,
        std::enable_if_t<has_struct_serialization<T>::value>>
    {
        template <typename Archive>
        static void call(Archive& ar, T& t, unsigned)
        {
            // This is automatic serialization for types
            // which are simple (brace-initializable) structs,
            // what that means every struct's field
            // has to be serializable and public.
            serialize_struct(ar, t, 0);
        }
    };

    template <typename T>
    struct serialize_non_intrusive<T,
        std::enable_if_t<!has_serialize_adl<T>::value>>
      : serialize_brace_initialized<T>
    {
    };

    ///////////////////////////////////////////////////////////////////////////
    class access
    {
    public:
        template <typename T>
        class has_serialize
        {
            template <typename T1>
            static std::false_type test(...);

            // the following expression sfinae trick
            // appears to work on clang-3.4, gcc-4.9,
            // icc-16, msvc-2017 (at least)
            // note that this detection would have been much easier
            // to implement if there hadn't been an issue with gcc:
            // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=82478
            template <typename T1,
                typename = decltype(
                    std::declval<typename std::remove_const<T1>::type&>()
                        .serialize(std::declval<
                                       pika::serialization::output_archive&>(),
                            0u))>
            static std::true_type test(int);

        public:
            static constexpr bool value = decltype(test<T>(0))::value;
        };

    private:
        template <typename T>
        class serialize_dispatcher
        {
            struct intrusive_polymorphic
            {
                // both following template functions are viable
                // to call right overloaded function according to T constness
                // and to prevent calling templated version of serialize function
                static void call(
                    pika::serialization::input_archive& ar, T& t, unsigned)
                {
                    t.serialize(ar, 0);
                }

                static void call(pika::serialization::output_archive& ar,
                    T const& t, unsigned)
                {
                    t.serialize(ar, 0);
                }
            };

            struct non_intrusive
            {
                template <class Archive>
                static void call(Archive& ar, T& t, unsigned)
                {
                    serialize_non_intrusive<T>::call(ar, t, 0);
                }
            };

            struct empty
            {
                template <class Archive>
                static void call(Archive&, T&, unsigned)
                {
                }
            };

            struct intrusive_usual
            {
                template <class Archive>
                static void call(Archive& ar, T& t, unsigned)
                {
                    // cast it to let it be run for templated
                    // member functions
                    const_cast<std::decay_t<T>&>(t).serialize(ar, 0);
                }
            };

        public:
            using type =
                std::conditional_t<pika::traits::is_intrusive_polymorphic_v<T>,
                    intrusive_polymorphic,
                    std::conditional_t<has_serialize<T>::value, intrusive_usual,
                        std::conditional_t<std::is_empty<T>::value, empty,
                            non_intrusive>>>;
        };

    public:
        template <typename Archive, typename T>
        static void serialize(Archive& ar, T& t, unsigned)
        {
            serialize_dispatcher<T>::type::call(ar, t, 0);
        }

        template <typename Archive, typename T>
        PIKA_FORCEINLINE static void save_base_object(
            Archive& ar, T const& t, unsigned)
        {
            // explicitly specify virtual function
            // of base class to avoid infinite recursion
            t.T::save(ar, 0);
        }

        template <typename Archive, typename T>
        PIKA_FORCEINLINE static void load_base_object(
            Archive& ar, T& t, unsigned)
        {
            // explicitly specify virtual function
            // of base class to avoid infinite recursion
            t.T::load(ar, 0);
        }

        template <typename T>
        PIKA_FORCEINLINE static std::string get_name(T const* t)
        {
            return t->pika_serialization_get_name();
        }
    };

}}    // namespace pika::serialization
