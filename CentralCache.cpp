//
// Created by 28943 on 2020/2/20.
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
    Span* span = pageCacheInst.NewSpan(numpage);

    return span;
}



// 从中心缓存central cache获取一定数量的对象给thread cach
size_t CentralCache::FetchRangeObj(void*& start, void*& end, size_t num, size_t size) {
    Span* span = GetOneSpan(size);
    FreeList& freelist = span->m_freeList;
    size_t actualNum = freelist.PopRange(start, end, num);
    span->m_usecount += actualNum; // 获取到多少, usecount计数就增加多少

    return actualNum;
}
