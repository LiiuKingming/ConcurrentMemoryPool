//
// Created by 28943 on 2020/2/20.
//
#ifndef CPLUSPLUS_COMMON_H
#define CPLUSPLUS_COMMON_H

#include <iostream>
#include <assert.h>
#include <windows.h>
using namespace std;

const size_t MAX_SIZE = 64 * 1024;
const size_t NFREE_LIST = MAX_SIZE / 8;
const size_t MAX_PAGES = 129;
const size_t PAGE_SHIFT = 12; // 4k为页位移

inline void*& NextObj(void* obj){
    // 通过强转指针类型返回头部4/8个字节即存储的下一内存块的地址
    return *((void**)obj);
}

class FreeList{
private:
    void* m_freeList = nullptr;

public:
    void Push(void* obj){ // 头插
        NextObj(obj) = m_freeList;
        m_freeList = obj;
    }

    void PushRange(void* start, void* end){
        NextObj(end) = m_freeList;
        m_freeList = start;
    }

    void* Pop(){ // 头删
        void* obj = m_freeList;
        m_freeList = NextObj(obj);

        return obj;
    }

    size_t PopRange(void* start, void* end, size_t num){
        size_t actualNum = 0;
        void* prev = nullptr;
        void* cur = m_freeList;

        for(; actualNum < num && cur != nullptr; ++actualNum){
            prev = cur;
            cur = NextObj(cur);
        }

        start = m_freeList;
        end = prev;
        m_freeList = cur;

        return actualNum;
    }

    bool Empty(){
        return m_freeList == nullptr;
    }
};

class SizeClass{
public:
    // 控制[1%, 10%]左右的内存碎片浪费
    // [1, 128] 8byte对齐 freelist[0, 16)
    // [129, 1024] 16byte对齐 freelist[16, 72)
    // [1025, 8*1024] 128byte对齐 freelist[72, 128)
    // [8*1204 + 1, 64*1024] 1024byte对齐 freelist[128, 184)
    static size_t m_RoundUp(size_t size, int alignment){
        return (size + alignment - 1)&(~(alignment - 1));
    }

    // [9-16] + 7 = [16-23] -> 16 8 4 2 1
    // [17-32] + 15 = [32,47] ->32 16 8 4 2 1
    static size_t RoundUp(size_t size){
        assert(size <= MAX_SIZE);

        if (size <= 128){
            return  m_RoundUp(size, 8);
        } else if (size <= 1024){
            return  m_RoundUp(size, 16);
        } else if (size <= 8192){
            return  m_RoundUp(size, 128);
        } else if(size <= 65536){
            return  m_RoundUp(size, 1024);
        }

        return -1;
    }

    // [9, 16] + 7  -> [16, 23]
    static size_t m_ListIndex(size_t size, int alignShift){
        return ((size + (1 << alignShift) - 1) >> alignShift) - 1;
    }

    static size_t ListIndex(size_t size){
        assert(size <= MAX_SIZE);

        // 每个区间有多少个链
        static int s_groupArray[4] = {16, 56, 56, 56};

        if(size <= 128){
            return m_ListIndex(size, 3); // 1 << 3
        } else if(size <= 1024){
            return m_ListIndex(size - 128, 4) + s_groupArray[0]; // 1 << 4

        } else if(size <= 8192){
            return m_ListIndex(size - 1024, 7) + s_groupArray[1] + s_groupArray[0]; // 1 << 7

        } else if(size <= 65536){
            return m_ListIndex(size - 8192, 10) + s_groupArray[2] + s_groupArray[1] + s_groupArray[0]; // 1 << 10
        }

        return -1;
    }

    // tc找cc要page
    static size_t NumMoveSize(size_t size){
        if(size == 0){
            return 0;
        }

        size_t num = MAX_SIZE / size;
        if(num < 2){
            num = 2;
        }
        // 最小情况申请8byte恰好申请4k
        if(num > 512){
            return 512;
        }

        return num;
    }

    static size_t NumMovePage(size_t size){
        size_t num = NumMoveSize(size);
        size_t npage = num * size;

        npage >>= 12; // 相当于npage / 4k
        if(npage == 0){
            npage = 1;
        }

        return npage;
    }
};

#ifdef _WIN32
typedef unsigned int PAGE_ID;
#else
typedef unsigned long long PAPAGE_ID ;
#endif // _WIN32

struct Span{
    PAGE_ID m_pageid; // 页号
    int m_pagesize; // 页的数量

    FreeList m_freeList; // 对象自由链表
    int m_usecount; // 内存块对象使用计数

    Span* m_next;
    Span* m_prev;
};

class SpanList{
    Span* m_head;

public:
    SpanList(){
        m_head = new Span;
        m_head->m_next= m_head;
        m_head->m_prev = m_head;
    }

    Span* Begin(){
        return m_head->m_next;
    }

    Span* End(){
        return m_head;
    }

    void PushFront(Span* newspan){
        Insert(m_head->m_next, newspan);
    }

    void PopFront(){
        Erase(m_head->m_next);
    }

    void PushBack(Span* newspan){
        Insert(m_head, newspan);
    }

    void PopBack(){
        Erase(m_head->m_prev);
    }

    void Insert(Span* pos, Span* newspan){
        Span* prev = pos->m_prev;

        // prev newspan pos
        prev->m_next = newspan;
        newspan->m_next = pos;
        pos->m_prev = newspan;
        newspan->m_prev = prev;
    }

    void Erase(Span* pos){
        assert(pos != m_head);

        Span* prev = pos->m_prev;
        Span* next = pos->m_next;

        prev->m_next = next;
        next->m_prev = prev;
    }

    bool Empty(){
        return Begin() == End();
    }

};

inline static void* SystemAlloc(size_t numPage){
#ifdef _WIN32
    void* ptr = VirtualAlloc(nullptr, numPage * (1 << PAGE_SHIFT),
            MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
    // brk mmap
#endif
    if (ptr == nullptr){
        throw std::bad_alloc();
    }

    return ptr;
}
#endif //CPLUSPLUS_COMMON_H