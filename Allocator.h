// ------------------------------
// projects/allocator/Allocator.h
// Copyright (C) 2015
// Glenn P. Downing
// ------------------------------

#ifndef Allocator_h
#define Allocator_h

// --------
// includes
// --------

#include <cassert>   // assert
#include <cstddef>   // ptrdiff_t, size_t
#include <new>       // bad_alloc, new
#include <stdexcept> // invalid_argument
#include <typeinfo>
#include "gtest/gtest_prod.h"

using namespace std;

// ---------
// Allocator
// ---------

template <typename T, std::size_t N>
class Allocator {
    public:
        // --------
        // typedefs
        // --------

        typedef T                 value_type;

        typedef std::size_t       size_type;
        typedef std::ptrdiff_t    difference_type;

        typedef       value_type*       pointer;
        typedef const value_type* const_pointer;

        typedef       value_type&       reference;
        typedef const value_type& const_reference;

    public:
        // -----------
        // operator ==
        // -----------

        friend bool operator == (const Allocator&, const Allocator&) {
            return true;}                                              // this is correct

        // -----------
        // operator !=
        // -----------

        friend bool operator != (const Allocator& lhs, const Allocator& rhs) {
            return !(lhs == rhs);}


    private:
        // ----
        // data
        // ----

        char a[N];

        // -----
        // valid
        // -----

        /**
         * O(1) in space
         * O(n) in time
         * Check that the front sentinel is equal to the back sentinel por block
         */
        bool valid () const {

            if ((*this)[0] == 0) return false;

            int positive_sentinel = 0;
            int i = 0;

            while (i < N) {
                positive_sentinel = (*this)[i] < 0 ? (-1 * (*this)[i]) : (*this)[i];
                /*cout << "valid_front: " << (*this)[i] << "\n";
                cout << "valid_back: " << (*this)[i + sizeof(int) + positive_sentinel] << "\n";
                cout << "valid_last: " << (*this)[N - sizeof(int)] << "\n";*/
                if ((*this)[i] != (*this)[i + sizeof(int) + positive_sentinel])
                    return false;
                i = i + (2 * sizeof(int)) + positive_sentinel;
            }
            return true;}

        /**
         * O(1) in space
         * O(1) in time
         * Tests definitions to be able to access to private methods
         * throught test, in this case to access to the [] operator
         * on my tests
         * https://code.google.com/p/googletest/wiki/AdvancedGuide#Private_Class_Members
         */
        FRIEND_TEST(TestAllocator2, index);
        
        FRIEND_TEST(TestAllocator4, allocate_1);
        FRIEND_TEST(TestAllocator4, allocate_2);
        FRIEND_TEST(TestAllocator4, allocate_3);
        FRIEND_TEST(TestAllocator4, allocate_4);
        FRIEND_TEST(TestAllocator4, allocate_5);
        FRIEND_TEST(TestAllocator4, allocate_5);
        FRIEND_TEST(TestAllocator4, deallocate_1);
        FRIEND_TEST(TestAllocator4, deallocate_2);
        FRIEND_TEST(TestAllocator4, deallocate_3);
        FRIEND_TEST(TestAllocator4, deallocate_4);
        FRIEND_TEST(TestAllocator4, Allocator_1);
        FRIEND_TEST(TestAllocator4, Allocator_2);
        FRIEND_TEST(TestAllocator4, Allocator_3);

        int& operator [] (int i) {
            return *reinterpret_cast<int*>(&a[i]);}

    public:
        // ------------
        // constructors
        // ------------

        /**
         * O(1) in space
         * O(1) in time
         * throw a bad_alloc exception, if N is less than sizeof(T) + (2 * sizeof(int))
         */
        Allocator () {

            if (N < (sizeof(T) + (2 * sizeof(int))))
                throw bad_alloc();
            (*this)[0] = N - (2 * sizeof(int));
            (*this)[N - sizeof(int)] = N - (2 * sizeof(int)); 

            assert(valid());}

        // Default copy, destructor, and copy assignment
        // Allocator  (const Allocator&);
        // ~Allocator ();
        // Allocator& operator = (const Allocator&);

        // --------
        // allocate
        // --------

        /**
         * O(1) in space
         * O(n) in time
         * after allocation there must be enough space left for a valid block
         * the smallest allowable block is sizeof(T) + (2 * sizeof(int))
         * choose the first block that fits
         * throw a bad_alloc exception, if n is invalid
         */
        pointer allocate (size_type n) {

            const int space_needed = (n * sizeof(T)) + (2 * (sizeof(int)));

            if (N < space_needed)
                throw bad_alloc();
            
            if (n ==  0)
                return 0;

            const int data_space_needed = n * sizeof(T);
            int i = 0;

            while (i < N) { 

                int& front_sentinel = (*this)[i];
                int positive_sentinel = front_sentinel < 0 ? (-1 * front_sentinel) : front_sentinel;
                int& back_sentinel = (*this)[i + positive_sentinel + sizeof(int)];
                
                if (front_sentinel != back_sentinel)
                    throw bad_alloc();

                if (front_sentinel == data_space_needed) {
                    front_sentinel = back_sentinel = (-1 * data_space_needed);
                    break;
                }

                if (front_sentinel > space_needed) {
                    front_sentinel = (-1 * data_space_needed);
                    back_sentinel = back_sentinel - space_needed;
                    (*this)[i + data_space_needed + sizeof(int)] = front_sentinel;
                    (*this)[i + space_needed] = back_sentinel;
                    break;
                }
                i = i + positive_sentinel + 2 * sizeof(int); 
            }

            if (i >= N)
                return 0;

            return reinterpret_cast<pointer>(&a[i + sizeof(int)]);}
            

        // ---------
        // construct
        // ---------

        /**
         * O(1) in space
         * O(1) in time
         */
        void construct (pointer p, const_reference v) {
            new (p) T(v);                               // this is correct and exempt
            assert(valid());}                           // from the prohibition of new

        // ----------
        // deallocate
        // ----------

        /**
         * O(1) in space
         * O(1) in time
         * after deallocation adjacent free blocks must be coalesced
         * throw an invalid_argument exception, if p is invalid
         * deallocate memory starting on the pointer passed to the method
         * get the value of the block to deallocate and check if the prev
         * and next block are free checking the sentinel value is positive
         * if the adjacent blocks are free then coalesces then all in one
         * free block making the front and the back sentinel a positive value
         * of the size of the block, without the 2 int sizes of the sentinels
         */
        void deallocate (pointer p, size_type) {

            if (p == 0)
                return;

            char* _p = (char*)p;
            int idx = _p - &a[0];
            
            if ((_p < &a[sizeof(int)]) || (_p > &a[N - sizeof(int)] - 1))
                throw invalid_argument("Invalid pointer - Pointer is not inside pool");
            
            int& front_sentinel = (*this)[idx - sizeof(int)];            
            int positive_sentinel = front_sentinel < 0 ? (-1 * front_sentinel) : front_sentinel;
            int total_sentinel = positive_sentinel;
            int& back_sentinel = (*this)[idx + positive_sentinel];
            bool done = false;
            
            if (front_sentinel != back_sentinel)
                throw invalid_argument("Invalid pointer - Pointer is not valid starting address block");

            if ((_p - (3 * sizeof(int))) > &a[0]) {

                int& prev_back_sentinel = (*this)[idx - (2 * sizeof(int))];
                
                if (prev_back_sentinel >= 0) {
                    int& prev_front_sentinel = (*this)[idx - (3 * sizeof(int)) - prev_back_sentinel];
                    total_sentinel = prev_back_sentinel + (2 * sizeof(int) + positive_sentinel);
                    prev_front_sentinel = total_sentinel;
                    back_sentinel = total_sentinel;
                    done = true;
                }
            }

            if ((_p + positive_sentinel + (3 * sizeof(int))) < &a[N]) {

                int& next_front_sentinel = (*this)[idx + positive_sentinel + sizeof(int)];
                
                if ((next_front_sentinel) >= 0) {
                    total_sentinel = total_sentinel + (2 * sizeof(int) + next_front_sentinel);
                    int& next_back_sentinel = (*this)[idx + positive_sentinel + (2 * (sizeof(int))) + next_front_sentinel];
                    if (done) {
                        (*this)[idx - (3 * sizeof(int)) - (*this)[idx - (2 * sizeof(int))]] = total_sentinel;
                    }
                    else front_sentinel = total_sentinel;
                    next_back_sentinel = total_sentinel;
                    done = true;
                }
            }

            if (!done) {
                front_sentinel = total_sentinel;
                back_sentinel = total_sentinel;
            }

            assert(valid());}
            

        // -------
        // destroy
        // -------

        /**
         * O(1) in space
         * O(1) in time
         */
        void destroy (pointer p) {
            p->~T();               // this is correct
            assert(valid());}

        /**
         * O(1) in space
         * O(1) in time
         * public [] operator, it is inmutable. We don't want anybody
         * changes [] operator. 
         */
        const int& operator [] (int i) const {
            return *reinterpret_cast<const int*>(&a[i]);}};

#endif // Allocator_h
