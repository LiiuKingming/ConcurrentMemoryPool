//
// Created by 28943 on 2020/1/9.
//

#include "CentralCache.h"
#include "PageCache.h"

// 从spanlist 或者 page cache获取一个span

Span* CentralCache::GetOneSpan(size_t size) {
    size_t index = SizeClass::ListIndex(size);
    SpanList& spanlist = m_spanLists[index];
    Span* it = spanlist.Begin();

    // 找到存在的Span并返回
    while (it != spanlist.End()){
        if (!it->m_freeList.Empty()){
            return it;
        } else {
            it = it->m_next;
        }
    }

    // 没找到Span则从page cache获取一个Span
    size_t numpage = SizeClass::NumMovePage(size);
    Span* span = PageCache::GetInstance().NewSpan(numpage);

    // 把span对象分裂成对应大小挂到span的freelist中
    char* start = (char*)(span->m_pageid << 12);
    char* end = start + (span->m_pagesize << 12);
    while (start < end){
        char* obj = start;
        start += size;

        span->m_freeList.Push(obj);
    }
    span->m_objSize = size;
    spanlist.PushFront(span);

    return span;
}

// 从中心缓存central cache获取一定数量的对象给thread cach
size_t CentralCache::FetchRangeObj(void*& start, void*& end, size_t num, size_t size) {
    size_t index = SizeClass::ListIndex(size);
    SpanList& spanList = m_spanLists[index];
    spanList.Lock();

    Span* span = GetOneSpan(size);
    FreeList& freelist = span->m_freeList;
    size_t actualNum = freelist.PopRange(start, end, num);
    span->m_usecount += actualNum; // 获取到多少, usecount计数就增加多少

    spanList.Unlock();

    return actualNum;
}

// 释放list到page cache
void CentralCache::ReleaseListToSpans(void *start, size_t size) {
    size_t index = SizeClass::ListIndex(size);
    SpanList& spanList = m_spanLists[index];
    spanList.Lock();

    while (start){
        void* next = NextObj(start);
        PAGE_ID id = (PAGE_ID)start >> PAGE_SHIFT;
        Span* span = PageCache::GetInstance().GetIdToSpan(id);
        span->m_freeList.Push(start);
        span->m_usecount--;

        // 表示当前span切出去的对象全部返回, 可以将span还给page cache进行合并, 减少内存碎片
        if (span->m_usecount == 0){
            size_t index = SizeClass::ListIndex(span->m_objSize);
            m_spanLists[index].Erase(span);
            span->m_freeList.Clear();

            PageCache::GetInstance().ReleaseSpanToPageCache(span);
        }

        start = next;
    }
    spanList.Unlock();
}
