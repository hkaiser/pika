//  Copyright (c) 2014-2015 Anton Bikineev
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// pikainspect:nodeprecatedinclude:boost/intrusive_ptr.hpp
// pikainspect:nodeprecatedname:boost::intrusive_ptr

#include <pika/serialization/base_object.hpp>
#include <pika/serialization/input_archive.hpp>
#include <pika/serialization/intrusive_ptr.hpp>
#include <pika/serialization/output_archive.hpp>
#include <pika/serialization/serialize.hpp>
#include <pika/serialization/shared_ptr.hpp>

#include <pika/modules/testing.hpp>

#if defined(PIKA_SERIALIZATION_HAVE_BOOST_TYPES)
#include <boost/intrusive_ptr.hpp>
#endif

#include <iostream>
#include <memory>
#include <string>
#include <vector>

// =========================shared_ptr test==============================
struct A
{
    int a;

    explicit A(int a = 1)
      : a(a)
    {
    }
    virtual ~A() {}

    virtual const char* foo() = 0;

    template <class Archive>
    void serialize(Archive& ar, unsigned)
    {
        ar& a;
    }
    PIKA_SERIALIZATION_POLYMORPHIC_ABSTRACT(A);
};

struct B : A
{
    int b;

    explicit B(int b = 2)
      : b(b)
    {
    }

    const char* foo()
    {
        return "B::foo";
    }

    template <class Archive>
    void load(Archive& ar, unsigned)
    {
        ar& pika::serialization::base_object<A>(*this);
        ar& b;
    }
    template <class Archive>
    void save(Archive& ar, unsigned) const
    {
        ar& pika::serialization::base_object<A>(*this);
        ar& b;
    }
    PIKA_SERIALIZATION_SPLIT_MEMBER();
    PIKA_SERIALIZATION_POLYMORPHIC_SPLITTED(B);
};

class C : public B
{
    friend class pika::serialization::access;
    int c;

    template <class Archive>
    void serialize(Archive& ar, unsigned)
    {
        ar& pika::serialization::base_object<B>(*this);
        ar& c;
    }
    PIKA_SERIALIZATION_POLYMORPHIC(C)

public:
    explicit C(int c = 3)
      : c(c)
    {
    }

    const char* foo()
    {
        return "C::foo";
    }

    int get_c() const
    {
        return c;
    }
};

void test_shared()
{
    std::shared_ptr<A> ip(new C);
    std::shared_ptr<A> op1;
    std::shared_ptr<A> op2;
    {
        std::vector<char> buffer;
        pika::serialization::output_archive oarchive(buffer);
        oarchive << ip << ip;

        pika::serialization::input_archive iarchive(buffer);
        iarchive >> op1;
        iarchive >> op2;
    }

    PIKA_TEST_NEQ(op1.get(), ip.get());
    PIKA_TEST_NEQ(op2.get(), ip.get());
    PIKA_TEST_EQ(op1.get(), op2.get());
    PIKA_TEST_EQ(op1->foo(), std::string("C::foo"));
    PIKA_TEST_EQ(op2->foo(), std::string("C::foo"));
    PIKA_TEST_EQ(static_cast<C*>(op1.get())->a, 1);
    PIKA_TEST_EQ(static_cast<C*>(op1.get())->b, 2);
    PIKA_TEST_EQ(static_cast<C*>(op1.get())->get_c(), 3);
    PIKA_TEST_EQ(static_cast<C*>(op2.get())->a, 1);
    PIKA_TEST_EQ(static_cast<C*>(op2.get())->b, 2);
    PIKA_TEST_EQ(static_cast<C*>(op2.get())->get_c(), 3);
    PIKA_TEST_EQ(op1.use_count(), 2);
}

#if defined(PIKA_SERIALIZATION_HAVE_BOOST_TYPES)
// =========================intrusive_ptr test==============================
struct D
{
    int a;
    int count;

    explicit D(int a = 1)
      : a(a)
      , count(0)
    {
    }
    virtual ~D() {}

    virtual const char* foo() = 0;

private:
    friend class pika::serialization::access;

    template <class Archive>
    void load(Archive& ar, unsigned)
    {
        ar& a;
    }
    template <class Archive>
    void save(Archive& ar, unsigned) const
    {
        ar& a;
    }
    PIKA_SERIALIZATION_SPLIT_MEMBER();
    PIKA_SERIALIZATION_POLYMORPHIC_ABSTRACT_SPLITTED(D);
};

void intrusive_ptr_add_ref(D* d)
{
    ++d->count;
}

void intrusive_ptr_release(D* d)
{
    if (--d->count == 0)
    {
        delete d;
    }
}

struct E : D
{
    int b;

    explicit E(int b = 2)
      : b(b)
    {
    }

    const char* foo()
    {
        return "E::foo";
    }

    template <class Archive>
    void load(Archive& ar, unsigned)
    {
        ar& pika::serialization::base_object<D>(*this);
        ar& b;
    }
    template <class Archive>
    void save(Archive& ar, unsigned) const
    {
        ar& pika::serialization::base_object<D>(*this);
        ar& b;
    }
    PIKA_SERIALIZATION_SPLIT_MEMBER();
    PIKA_SERIALIZATION_POLYMORPHIC_SPLITTED(E);
};

class F : public E
{
    friend class pika::serialization::access;
    int c;

    template <class Archive>
    void serialize(Archive& ar, unsigned)
    {
        ar& pika::serialization::base_object<E>(*this);
        ar& c;
    }
    PIKA_SERIALIZATION_POLYMORPHIC(F)

public:
    explicit F(int c = 3)
      : c(c)
    {
    }

    const char* foo()
    {
        return "F::foo";
    }

    int get_c() const
    {
        return c;
    }
};

void test_intrusive()
{
    boost::intrusive_ptr<D> ip(new F);
    boost::intrusive_ptr<D> op1;
    boost::intrusive_ptr<D> op2;
    {
        std::vector<char> buffer;
        pika::serialization::output_archive oarchive(buffer);
        oarchive << ip << ip;

        pika::serialization::input_archive iarchive(buffer);
        iarchive >> op1;
        iarchive >> op2;
    }
    PIKA_TEST_NEQ(op1.get(), ip.get());
    PIKA_TEST_NEQ(op2.get(), ip.get());
    PIKA_TEST_EQ(op1.get(), op2.get());
    PIKA_TEST_EQ(op1->foo(), std::string("F::foo"));
    PIKA_TEST_EQ(op2->foo(), std::string("F::foo"));
    PIKA_TEST_EQ(static_cast<F*>(op1.get())->a, 1);
    PIKA_TEST_EQ(static_cast<F*>(op1.get())->b, 2);
    PIKA_TEST_EQ(static_cast<F*>(op1.get())->get_c(), 3);
    PIKA_TEST_EQ(static_cast<F*>(op2.get())->a, 1);
    PIKA_TEST_EQ(static_cast<F*>(op2.get())->b, 2);
    PIKA_TEST_EQ(static_cast<F*>(op2.get())->get_c(), 3);

    PIKA_TEST_EQ(ip->count, 1);
    PIKA_TEST_EQ(op1->count, 2);
    PIKA_TEST_EQ(op2->count, 2);
    op1.reset();
    PIKA_TEST_EQ(op2->count, 1);
}
#endif

int main()
{
    test_shared();
#if defined(PIKA_SERIALIZATION_HAVE_BOOST_TYPES)
    test_intrusive();
#endif

    return pika::util::report_errors();
}
