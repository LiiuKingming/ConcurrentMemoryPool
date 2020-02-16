//
// Created by 28943 on 2020/2/20.
//

#ifndef CPLUSPLUS_CENTRALCACHE_H
#define CPLUSPLUS_CENTRALCACHE_H

#include "Common.h"

class CentralCache{
    SpanList m_spanLists[NFREE_LIST];
public:
    // 从中心缓存获取一定数量的对象给thread cache
    size_t FetchRangeObj(void*& start, void*& end, size_t num, size_t size);

    // 将一定数量的对象释放到span跨度
    void ReleaseListToSpans(void* start);

    // 从spanlist 或者 page cache获取一个span
    Span* GetOneSpan(size_t size);
};

static CentralCache centralCacheInst;

#endif //CPLUSPLUS_CENTRALCACHE_H
