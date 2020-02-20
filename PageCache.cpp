//
// Created by 28943 on 2020/1/9.
//

#include "PageCache.h"
#include "ObjectPool.h"

Span* PageCache::m_NewSpan(size_t numpage) {
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

            Span* splitspan = new Span;// 替换成对象池
            /*
            ObjectPool<Span> spanpool;
            Span* splitspan = spanpool.New();
            */

            // 分裂尾部
            splitspan->m_pageid = span->m_pageid + span->m_pagesize - numpage;
            splitspan->m_pagesize = numpage;
            //调整span映射map
            for (PAGE_ID i = 0; i < numpage; ++i){
                m_idSpanmap[splitspan->m_pageid + i] = splitspan;
            }

            span->m_pagesize -= numpage;

            m_spanList[splitspan->m_pagesize].PushBack(span);

            return splitspan;
        }
    }

    // 没有现成的span可以分裂使用, 从page cache中获取bigspan分裂使用
    void* ptr = SystemAlloc(MAX_PAGES - 1);

    Span* bigspan = new Span; // 替换成对象池
    /*
    ObjectPool<Span> spanpool;
    Span* bigspan = spanpool.New();
    */
    bigspan->m_pageid = (PAGE_ID)ptr >> PAGE_SHIFT; // 获取以4k为单位整页
    bigspan->m_pagesize = MAX_PAGES - 1;

    //调整span映射map
    for (PAGE_ID i = 0; i < bigspan->m_pagesize; ++i){
        // m_idSpanmap.insert(std::make_pair(bigspan->m_pageid + i, bigspan));
        m_idSpanmap[bigspan->m_pageid + i] = bigspan;
    }

    m_spanList[bigspan->m_pagesize].PushFront(bigspan);

    // 从page cache获取完进行最多一次的递归分裂, 对效率影响很小.
    return m_NewSpan(numpage);
}

Span* PageCache::NewSpan(size_t numpage) {
     m_mtx.lock();

    Span* span = m_NewSpan(numpage);

     m_mtx.unlock();

    return span;
}

void PageCache::ReleaseSpanToPageCache(Span *span) {
    m_mtx.lock();
    // 向前合并
    while (1){
        PAGE_ID prevPageId = span->m_pageid - 1;
        auto pit = m_idSpanmap.find(prevPageId);
        // 前面的页不存在
        if (pit == m_idSpanmap.end()){
            break;
        }

        //前页存在. 判断出在使用中的条件,无法合并
        Span* prevSpan = pit->second;
        if(prevSpan->m_usecount != 0){
            break;
        }

        // 合并
        // 合并后的页数如果超过MAX_PAGES就不合并了
        if (span->m_pagesize + prevSpan->m_pagesize >= MAX_PAGES){
            break;
        }

        span->m_pageid = prevSpan->m_pageid;
        span->m_pagesize += prevSpan->m_pagesize;
        for (PAGE_ID i = 0; i < prevSpan->m_pagesize; ++i){
            m_idSpanmap[prevSpan->m_pageid + i] = span;
        }

        m_spanList[prevSpan->m_pagesize].Erase(prevSpan);
        delete prevSpan;
    }

    // 向后合并
    while (1){
        PAGE_ID nextPageId = span->m_pageid + span->m_pagesize;
        auto nextIt = m_idSpanmap.find(nextPageId);
        // 后面的页不存在
        if (nextIt == m_idSpanmap.end()){
            break;
        }

        // 前页存在. 判断出在使用中的条件,无法合并
        Span* nextPage = nextIt->second;
        if(nextPage->m_usecount != 0){
            break;
        }

        // 合并
        // 合并后的页数如果超过MAX_PAGES就不合并了
        if (span->m_pagesize + nextPage->m_pagesize >= MAX_PAGES){
            break;
        }

        span->m_pagesize += nextPage->m_pagesize;
        for (PAGE_ID i = 0; i < nextPage->m_pagesize; ++i){
            m_idSpanmap[nextPage->m_pageid + i] = span;
        }

        m_spanList[nextPage->m_pagesize].Erase(nextPage);
        delete nextPage;
    }

    //前后都合并完成后, 插入到spanList对应长度的链上
    m_spanList[span->m_pagesize].PushFront(span);
    m_mtx.unlock();
}

// 获取该id的span
Span* PageCache::GetIdToSpan(PAGE_ID id){
    // std::map<PAGE_ID, Span*>::iterator it = m_idSpanmap.find(id);
    auto it = m_idSpanmap.find(id);
    if (it != m_idSpanmap.end()){
        return it->second;
    } else {
        return nullptr;
    }
}
