// ------------------------------------
// projects/allocator/TestAllocator1.c++
// Copyright (C) 2015
// Glenn P. Downing
// ------------------------------------

// --------
// includes
// --------

#include <algorithm> // count
#include <memory>    // allocator

#include "gtest/gtest.h"

#include "Allocator.h"

// --------------
// TestAllocator1
// --------------

template <typename A>
struct TestAllocator1 : testing::Test {
    // --------
    // typedefs
    // --------

    typedef          A                  allocator_type;
    typedef typename A::value_type      value_type;
    typedef typename A::difference_type difference_type;
    typedef typename A::pointer         pointer;};

typedef testing::Types<
            std::allocator<int>,
            std::allocator<double>,
            Allocator<int,    100>,
            Allocator<double, 100> >
        my_types_1;

TYPED_TEST_CASE(TestAllocator1, my_types_1);

TYPED_TEST(TestAllocator1, test_1) {
    typedef typename TestFixture::allocator_type  allocator_type;
    typedef typename TestFixture::value_type      value_type;
    typedef typename TestFixture::difference_type difference_type;
    typedef typename TestFixture::pointer         pointer;

          allocator_type  x;
    const difference_type s = 1;
    const value_type      v = 2;
    const pointer         p = x.allocate(s);
    if (p != nullptr) {
        x.construct(p, v);
        ASSERT_EQ(v, *p);
        x.destroy(p);
        x.deallocate(p, s);}}

TYPED_TEST(TestAllocator1, test_10) {
    typedef typename TestFixture::allocator_type  allocator_type;
    typedef typename TestFixture::value_type      value_type;
    typedef typename TestFixture::difference_type difference_type;
    typedef typename TestFixture::pointer         pointer;

          allocator_type  x;
    const difference_type s = 10;
    const value_type      v = 2;
    const pointer         b = x.allocate(s);
    if (b != nullptr) {
        pointer e = b + s;
        pointer p = b;
        try {
            while (p != e) {
                x.construct(p, v);
                ++p;}}
        catch (...) {
            while (b != p) {
                --p;
                x.destroy(p);}
            x.deallocate(b, s);
            throw;}
        ASSERT_EQ(s, std::count(b, e, v));
        while (b != e) {
            --e;
            x.destroy(e);}
        x.deallocate(b, s);}}

// --------------
// TestAllocator2
// --------------

TEST(TestAllocator2, const_index) {
    const Allocator<int, 100> x;
    ASSERT_EQ(x[0], 92);}

TEST(TestAllocator2, index) {
    Allocator<int, 100> x;
    ASSERT_EQ(x[0], 92);}

// --------------
// TestAllocator3
// --------------

template <typename A>
struct TestAllocator3 : testing::Test {
    // --------
    // typedefs
    // --------

    typedef          A                  allocator_type;
    typedef typename A::value_type      value_type;
    typedef typename A::difference_type difference_type;
    typedef typename A::pointer         pointer;};

typedef testing::Types<
            Allocator<int,    100>,
            Allocator<double, 100> >
        my_types_2;

TYPED_TEST_CASE(TestAllocator3, my_types_2);

TYPED_TEST(TestAllocator3, test_1) {
    typedef typename TestFixture::allocator_type  allocator_type;
    typedef typename TestFixture::value_type      value_type;
    typedef typename TestFixture::difference_type difference_type;
    typedef typename TestFixture::pointer         pointer;

          allocator_type  x;
    const difference_type s = 1;
    const value_type      v = 2;
    const pointer         p = x.allocate(s);
    if (p != nullptr) {
        x.construct(p, v);
        ASSERT_EQ(v, *p);
        x.destroy(p);
        x.deallocate(p, s);}}

TYPED_TEST(TestAllocator3, test_10) {
    typedef typename TestFixture::allocator_type  allocator_type;
    typedef typename TestFixture::value_type      value_type;
    typedef typename TestFixture::difference_type difference_type;
    typedef typename TestFixture::pointer         pointer;

          allocator_type  x;
    const difference_type s = 10;
    const value_type      v = 2;
    const pointer         b = x.allocate(s);
    if (b != nullptr) {
        pointer e = b + s;
        pointer p = b;
        try {
            while (p != e) {
                x.construct(p, v);
                ++p;}}
        catch (...) {
            while (b != p) {
                --p;
                x.destroy(p);}
            x.deallocate(b, s);
            throw;}
        ASSERT_EQ(s, std::count(b, e, v));
        while (b != e) {
            --e;
            x.destroy(e);}
        x.deallocate(b, s);}}

/*-------------------------------------------------------------------------*/
/*                                     My Tests                            */
/*-------------------------------------------------------------------------*/

// ----------------------------------
// allocate() - finds first fit
// ----------------------------------

TEST(TestAllocator4, allocate_1) {
    int s = 5;
    typedef Allocator<int, 100>::pointer pointer;
    Allocator<int, 100> x;
    const pointer p = x.allocate(s);
    ASSERT_EQ(x[0], -20);
    ASSERT_EQ(x[24], -20);
    x.deallocate(p, s);
    ASSERT_EQ(x[96], 92);
}

TEST(TestAllocator4, allocate_2) {
    double s = 9;
    typedef Allocator<double, 200>::pointer pointer;
    Allocator<double, 200> x;
    const pointer p = x.allocate(s);
    ASSERT_EQ(x[0], -72);
    ASSERT_EQ(x[76], -72);
    x.deallocate(p, s);
    ASSERT_EQ(x[196], 192);
}


// ----------------------------------
// allocate() - finds no fit
// ----------------------------------

TEST(TestAllocator4, allocate_3) {
    int s1 = 10;
    int s2 = 5;
    typedef Allocator<int, 68>::pointer pointer;
    Allocator<int, 68> x;
    const pointer p1 = x.allocate(s1);
    ASSERT_EQ(x[0], -40);
    ASSERT_EQ(x[44], -40);

    // Remaining available memory
    ASSERT_EQ(x[48], 12);

    // Allocator will need at least 28 bytes to allocate s2
    // s2 does not fit, then nothing change on sentinels
    // and p2 is null;
    const pointer p2 = x.allocate(s2);
    ASSERT_EQ(x[48], 12);
    ASSERT_EQ(p2, nullptr);

    x.deallocate(p1, s1);
}

TEST(TestAllocator4, allocate_4) {
    double s1 = 8;
    double s2 = 5;
    typedef Allocator<double, 90>::pointer pointer;
    Allocator<double, 90> x;
    const pointer p1 = x.allocate(s1);
    ASSERT_EQ(x[0], -64);
    ASSERT_EQ(x[68], -64);

    // Remaining available memory
    ASSERT_EQ(x[72], 10);

    // Allocator will need at least 48 bytes to allocate s2
    // s2 does not fit, then nothing change on sentinels
    // and p2 is null;
    const pointer p2 = x.allocate(s2);
    ASSERT_EQ(x[72], 10);
    ASSERT_EQ(p2, nullptr);

    x.deallocate(p1, s1);
}

// ----------------------------------
// allocate() - throws bad_alloc
// ----------------------------------

TEST(TestAllocator4, allocate_5) {
    int s = 50;
    string my_catch = "No bad_alloc";
    typedef Allocator<int, 100>::pointer pointer;
    Allocator<int, 100> x;
    try {
        const pointer p = x.allocate(s);
    } 
    catch (bad_alloc &e){
        my_catch = "There is not enough space for allocation";
    }
    ASSERT_EQ(my_catch, "There is not enough space for allocation");
}

// ----------------------------------
// allocate() - return null pointer
// ----------------------------------

TEST(TestAllocator4, allocate_6) {
    int s = 0;
    typedef Allocator<int, 100>::pointer pointer;
    Allocator<int, 100> x;
    const pointer p = x.allocate(s);
    ASSERT_EQ(p, nullptr);
}


// -------------------------------------
// deallocate() - coalesces after block
// -------------------------------------

TEST(TestAllocator4, deallocate_1) {
    int s1 = 10;
    int s2 = 5;
    typedef Allocator<int, 100>::pointer pointer;
    Allocator<int, 100> x;
    const pointer p1 = x.allocate(s1);
    ASSERT_EQ(x[0], -40);
    ASSERT_EQ(x[44], -40);

    // Remaining available memory
    ASSERT_EQ(x[48], 44);

    const pointer p2 = x.allocate(s2);
    // Ocupied 28 more bytes
    ASSERT_EQ(x[48], -20);

    // Remaining available memory
    ASSERT_EQ(x[76], 16);

    x.deallocate(p2, s2);

    // I have the memory available again when after blocks
    // coalesces with the posterior block that call deallocate()
    ASSERT_EQ(x[48], 44);

    x.deallocate(p1, s1);

    // I have the memory available again when after blocks
    // coalesces with the posterior block
    ASSERT_EQ(x[0], 92);
}


// -------------------------------------
// deallocate() - coalesces before block
// -------------------------------------

TEST(TestAllocator4, deallocate_2) {
    int s1 = 10;
    int s2 = 5;
    typedef Allocator<int, 100>::pointer pointer;
    Allocator<int, 100> x;
    const pointer p1 = x.allocate(s1);
    ASSERT_EQ(x[0], -40);
    ASSERT_EQ(x[44], -40);

    // Remaining available memory
    ASSERT_EQ(x[48], 44);

    const pointer p2 = x.allocate(s2);
    // Ocupied 28 more bytes
    ASSERT_EQ(x[48], -20);

    // Remaining available memory
    ASSERT_EQ(x[76], 16);

    x.deallocate(p1, s1);
    // No coalesces yet, since this is the first block
    // but the block is already available
    ASSERT_EQ(x[0], 40);
    ASSERT_EQ(x[44], 40);

    x.deallocate(p2, s2);
    // coalesces with the previous block, now the whole 
    // space is available
    ASSERT_EQ(x[0], 92);
    ASSERT_EQ(x[96], 92);
}

// ---------------------------------------
// deallocate() - throws invalid_argument
// ---------------------------------------

TEST(TestAllocator4, deallocate_3) {
    int s = 50;
    string my_catch = "No invalid_argument";
    typedef Allocator<int, 100>::pointer pointer;
    Allocator<int, 100> x;
    try {
        const pointer p = &x[sizeof(int)] - 2;
        x.deallocate(p, s);
    } 
    catch (invalid_argument &e){
        my_catch = "Invalid pointer - Pointer is not inside pool";
    }
    
    ASSERT_EQ(my_catch, "Invalid pointer - Pointer is not inside pool");
}

TEST(TestAllocator4, deallocate_4) {
    int s = 50;
    string my_catch = "No invalid_argument";
    typedef Allocator<int, 100>::pointer pointer;
    Allocator<int, 100> x;
    try {
        const pointer p = &x[100 - sizeof(int)] + 2;
        x.deallocate(p, s);
    } 
    catch (invalid_argument &e){
        my_catch = "Invalid pointer - Pointer is not inside pool";
    }
    
    ASSERT_EQ(my_catch, "Invalid pointer - Pointer is not inside pool");
}

// ---------------------------------------
// Allocator() - sets sentinels
// ---------------------------------------

TEST(TestAllocator4, Allocator_1) {
    Allocator<int, 300> x;
    ASSERT_EQ(x[0], 292);
    ASSERT_EQ(x[296], 292);
}

TEST(TestAllocator4, Allocator_2) {
    Allocator<double, 150> x;
    ASSERT_EQ(x[0], 142);
    ASSERT_EQ(x[146], 142);
}

// ---------------------------------------
// Allocator() - throws bad_alloc
// ---------------------------------------

TEST(TestAllocator4, Allocator_3) {
    string my_catch = "No bad_alloc";
    try {
        Allocator<double, 15> x;
    } 
    catch (bad_alloc &e){
        my_catch = "There is not the minimum required space for allocation";
    }
    
    ASSERT_EQ(my_catch, "There is not the minimum required space for allocation");
}