/* MoL Allocator
 * -------------
 *
 * A simple buddy allocator. Nothing too special, part of the MoL project (hence the __MOL namespace). Feel free to change the namespace to suit your project better
 *
 * Copyright 2017 Justin Huang
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * ChangeLog
 * ---------
 * v1.0 - Justin Huang
 * Initial commit
 */


#include <algorithm>
#include <cstdint>
#include <exception>
#include <memory>

typedef uint8_t byte;

inline uint32_t fastmax(uint32_t x, uint32_t y)
{
    return (x ^ ((x^y) & ((x > y)-1)));
}

inline uint32_t pow2(uint32_t n)
{
    uint32_t rVal = 1;
    while (rVal < n){
        rVal = rVal << 1;
    }
    return rVal;
}

namespace __MOL{
    /* Storing everything in bytes internally because I want "nice" byte alignment */
    template<class T>
    struct Allocator //A long-term thing, memory large chunk of memory
    {
    public:
        typedef T value_type;

        Allocator(uint32_t n = 1024) noexcept //Allocator for n bytes
        {
            //Make size fit into max(smallest power of 2 that fits size, 1024)
            __max = fastmax(1024, pow2(n)) * sizeof(T); //TODO: Make this not tiny (i.e. 1KB min -> it's useless at that size)
            __datastore = new byte[__max](); // Quack
            __header* h = reinterpret_cast<__header*>(__datastore);
            h->magic = MAGIC_FREE;
            h->size = __max;
            h->next = h;
            h->prev = h;
            _head = __datastore;
            _end = __datastore+__max; //Just an easy reference for what the end of my actual block is
        }

        ~Allocator(){}

        uint32_t max_size() {return (__max/sizeof(T));}

        T* allocate(const uint32_t n)
        {
            if(!_head){
                throw std::bad_alloc();
            }
            uint32_t reqSize = pow2((sizeof(T) * n)+HEADER_SIZE);
            T* rVal = NULL;
            __header* head = reinterpret_cast<__header*>(_head);
            __header* currH = head;

            /* Check if we have enough space to fit everything in. Head will always be the largest size the guys can hold */
            if(reqSize > head->size)
            {
                throw std::bad_alloc();
            }

            /* Choose the best guy to pack into  */
            while(currH->next != head)
            {
                if(currH->next->size >= reqSize)
                {
                    currH = currH->next;
                }
            }

            while(currH->size > reqSize*2)
            { //Will always be greater than or equal
            /* Give them the snip snip */
            uint32_t newSize = currH->size/2;
            __header* newBlock = reinterpret_cast<__header*>(reinterpret_cast<byte*>(currH) + newSize); //???
            currH->size = newSize; //Always a power of 2 so nice things will happen :)

            /* Init new block */
            newBlock->magic = MAGIC_FREE;
            newBlock->prev = currH;
            newBlock->next = currH->next;
            newBlock->size = newSize;
            currH->next = newBlock;

            /* Always allocate the furthest right possible */
            currH = newBlock;
        }
        /* Found the appropriate block in currH */
        __header* prevBlock = currH->prev;
        if(prevBlock == currH){
            _head = NULL;
        }

        /* This guy is no longer free */
        currH->magic = MAGIC_FULL;
        prevBlock->next = currH->next;

        rVal = reinterpret_cast<T*>(reinterpret_cast<byte*>(currH) + HEADER_SIZE);
        return rVal;
    }

    void deallocate(T* p, uint32_t n)
    {
        /* Quack */
        __header* h = reinterpret_cast<__header*>(reinterpret_cast<byte*>(p)-HEADER_SIZE);
        /* Set this guy as free, add him to the pool */
        h->magic = MAGIC_FREE;

        if(_head)
        {
            /* Add this guy back into the free memory pool */
            __header* search = h;
            do
            {
                search = reinterpret_cast<__header*>(reinterpret_cast<byte*>(search)+search->size); //Reach the end of the block
                if(search == reinterpret_cast<__header*>(_end))
                {
                    search = reinterpret_cast<__header*>(_head);
                }
            } while(search->magic != MAGIC_FREE && search != h);

            h->prev = search->prev;
            h->next = search;
            search->prev = h;

            //Merge backwards only
            while (h->prev->magic == MAGIC_FREE && h->prev->size == h->size){
                h = h->prev;
                h->size *= 2;
                h->next = h->next->next;
                h->next->magic = 0;
                h->next->prev = 0;
                h->next->next = 0;
                h->next->size = 0;
            }
        }
        else
        {
            /* This guy is the only guy that is free *yey* */
            _head = reinterpret_cast<byte*>(h);
            h->next = h;
            h->prev = h;
        }
    }

    private:

        struct __header
        {
            uint32_t magic;
            uint32_t size;
            __header* next; //Points to next free block
            __header* prev; //Points to prev free block
        };

        const uint32_t HEADER_SIZE = sizeof(__header);
        const uint32_t MAGIC_FULL = 0xDEADBEEF;
        const uint32_t MAGIC_FREE = 0xB0B5DEAD;

        uint32_t __max;
        byte* __datastore;
        byte* _head;
        byte* _end;
    };
}
