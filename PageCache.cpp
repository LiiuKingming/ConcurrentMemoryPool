//
// Created by 28943 on 2020/2/20.
//

#include "PageCache.h"

Span* PageCache::NewSpan(size_t numpage) {
    if(!m_spanList[numpage].Empty()){
        Span* span = m_spanList[numpage].Begin();
        m_spanList[numpage].PopFront();

        return span;
    }

    for (size_t i = numpage + 1; i < MAX_PAGES; ++i){
        if (!m_spanList[i].Empty()){
            // 分裂
            Span* span = m_spanList[i].Begin();
            m_spanList[i].PopFront();

            Span* splitspan = new Span;
            splitspan->m_pageid = span->m_pageid + numpage;
            splitspan->m_pagesize = span->m_pagesize - numpage;

            span->m_pagesize = numpage;

            m_spanList[splitspan->m_pagesize].PushBack(splitspan);

            return span;
        }
    }

    // 没有现成的span可以分裂使用, 从page cache中获取bigspan分裂使用
    void* ptr = SystemAlloc(MAX_PAGES - 1);

    Span* bigspan = new Span;
    bigspan->m_pageid = (PAGE_ID)ptr >> PAGE_SHIFT;
    bigspan->m_pagesize = MAX_PAGES - 1;

    m_spanList[bigspan->m_pagesize].PushFront(bigspan);

    // 从page cache获取完进行最多一次的递归分裂, 对效率影响很小.
    return NewSpan(numpage);
}
