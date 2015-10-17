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
         * <your documentation>
         */
        bool valid () const {
            //return (!b && !e && !_l) || ((_b <= _e) && (_e <= _l));}
            return true;}

        /**
         * O(1) in space
         * O(1) in time
         * <your documentation>
         * https://code.google.com/p/googletest/wiki/AdvancedGuide#Private_Class_Members
         */
        FRIEND_TEST(TestAllocator2, index);
        /*int& operator [] (int i) {
            return *reinterpret_cast<int*>(&a[i]);}*/

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
            try {
                if (N < (sizeof(T) + (2 * sizeof(int))))
                    throw bad_alloc();
                (*this)[0] = N - (2 * sizeof(int));
                (*this)[N - sizeof(int)] = N - (2 * sizeof(int)); }
            catch (bad_alloc &e) {
                (*this)[0] = 0;
                cout << e.what() << " - There is not enough space for allocation" << "\n";
                exit(-1);
            }
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

            if ((n < 0) || (N < space_needed) || (sizeof(T) < 1))
                    throw bad_alloc();
            else if (n ==  0)
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
         * <your documentation>
         */
        void deallocate (pointer p, size_type) {

            if (p == 0)
                return;

            char* _p = (char*)p;
            
            if ((_p < &a[sizeof(int)]) || (_p > &a[N - sizeof(int)] - 1))
                throw invalid_argument("Invalid pointer: Pointer is not inside pool");
            
            int& front_sentinel = *reinterpret_cast<int*>(_p - sizeof(int));            
            int positive_sentinel = front_sentinel < 0 ? (-1 * front_sentinel) : front_sentinel;
            int total_sentinel = positive_sentinel;
            int& back_sentinel = *reinterpret_cast<int*>(_p + positive_sentinel);
            int& prev_back_sentinel = *reinterpret_cast<int*>(_p - (2 * sizeof(int)));
            int& prev_front_sentinel = *reinterpret_cast<int*>((_p - (3 * sizeof(int)) - prev_back_sentinel));
            bool done = false;
            
            if (front_sentinel != back_sentinel)
                throw invalid_argument("Invalid pointer: Pointer is not valid starting address block");

            if ((_p - (3 * sizeof(int))) > &a[0]) {

                if (prev_back_sentinel >= 0) {
                    
                    cout << "prev: " << prev_back_sentinel << "\n";
                    total_sentinel = prev_back_sentinel + (2 * sizeof(int) + positive_sentinel);
                    cout << "total: " << total_sentinel << "\n";
                    prev_front_sentinel = total_sentinel;
                    cout << "front: " << prev_front_sentinel << "\n";
                    back_sentinel = total_sentinel;
                    cout << "back: " << back_sentinel << "\n";
                    done = true;
                }
            }

            if ((_p + positive_sentinel + (3 * sizeof(int))) <= &a[N]) {

                int& next_front_sentinel = *reinterpret_cast<int*>(_p + positive_sentinel + sizeof(int));
                
                if ((next_front_sentinel) >= 0) {
                    total_sentinel = total_sentinel + (2 * sizeof(int) + next_front_sentinel);
                    int& next_back_sentinel = *reinterpret_cast<int*>(_p + positive_sentinel + (sizeof(int)) + next_front_sentinel);
                    if (done) {
                        prev_front_sentinel = total_sentinel;
                    }
                    else front_sentinel = total_sentinel;
                    next_back_sentinel = total_sentinel;
                    cout << "next: " << next_back_sentinel << "\n";
                    cout << "total: " << total_sentinel << "\n";
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
         * <your documentation>
         */
        const int& operator [] (int i) const {
            return *reinterpret_cast<const int*>(&a[i]);}};

#endif // Allocator_h
