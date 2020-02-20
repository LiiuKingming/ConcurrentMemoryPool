//
// Created by 28943 on 2020/1/9.
//

#ifndef CPLUSPLUS_PAGECACHE_H
#define CPLUSPLUS_PAGECACHE_H

#include "Common.h"

class PageCache{
    SpanList m_spanList[MAX_PAGES];
    std::unordered_map<PAGE_ID, Span*> m_idSpanmap;
    std::mutex m_mtx;

    // 设计成单例模式
    PageCache() = default;
    PageCache (const PageCache&) = delete;
    // static PageCache s_inst;

public:
    static PageCache& GetInstance(){
        static PageCache s_inst;
        return s_inst;
    }

    Span* NewSpan(size_t numpage);
    Span* m_NewSpan(size_t numpage);
    void ReleaseSpanToPageCache(Span* span);
    Span* GetIdToSpan(PAGE_ID id);
};

// static PageCache pageCacheInst;

#endif //CPLUSPLUS_PAGECACHE_H
