//
// Created by 28943 on 2020/2/20.
//

#ifndef CPLUSPLUS_PAGECACHE_H
#define CPLUSPLUS_PAGECACHE_H

#include "Common.h"


class PageCache{
    SpanList m_spanList[MAX_PAGES];
public:
    //
    Span* NewSpan(size_t numpage);

};

static PageCache pageCacheInst;

#endif //CPLUSPLUS_PAGECACHE_H
