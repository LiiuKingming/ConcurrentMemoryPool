//
// Created by 28943 on 2020/1/11.
//

#ifndef CPLUSPLUS_CONCURRENTMALLOC_H
#define CPLUSPLUS_CONCURRENTMALLOC_H

#include "ThreadCache.h"
#include "PageCache.h"

static void* ConcurrentMalloc(size_t size){
    if (size <= MAX_SIZE) { // [1byte, 64kb]
        if (pThreaCache == nullptr) {
            pThreaCache = new ThreadCache; // Ìæ»»³É¶ÔÏó³Ø
            // cout << std::this_thread::get_id() << "->" << pThreaCache << endl;
        }
        return pThreaCache->Allocte(size);

    } else if(size <= ((MAX_PAGES - 1) << PAGE_SHIFT)) {// (64kb, 128*4kb]
        size_t alignSize = SizeClass::m_RoundUp(size, 1 << PAGE_SHIFT);
        size_t pagenum = (alignSize >> PAGE_SHIFT);

        Span* span = PageCache::GetInstance().NewSpan(pagenum);
        span->m_objSize = alignSize;

        void* ptr = (void*)(span->m_pageid << PAGE_SHIFT);
        return ptr;
    } else { // [128*4kb,-]
        size_t alignSize = SizeClass::m_RoundUp(size, 1 << PAGE_SHIFT);
        size_t pagenum = (alignSize >> PAGE_SHIFT);
        return SystemAlloc(pagenum);
    }
}

static void ConcurrentFree(void* ptr){
    size_t pageid = (PAGE_ID)ptr >> PAGE_SHIFT;
    Span* span = PageCache::GetInstance().GetIdToSpan(pageid);

    if (span == nullptr){ // [128*4kb,-]
        SystemFree(ptr);
        return;
    }

    size_t size = span->m_objSize;
    if (size <= MAX_SIZE) {
        pThreaCache->Deallocte(ptr, size);
    } else if(size <= ((MAX_PAGES - 1) << PAGE_SHIFT)) { // (64kb, 128*4kb]
        PageCache::GetInstance().ReleaseSpanToPageCache(span);
    }
}

#endif //CPLUSPLUS_CONCURRENTMALLOC_H
