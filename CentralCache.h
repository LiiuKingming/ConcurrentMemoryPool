//
// Created by 28943 on 2020/1/9.
//

#ifndef CPLUSPLUS_CENTRALCACHE_H
#define CPLUSPLUS_CENTRALCACHE_H

#include "Common.h"

class CentralCache{
    SpanList m_spanLists[NFREE_LIST];

    // 设计成单例模式
    CentralCache() = default;
    CentralCache(const CentralCache&) = delete;
    CentralCache& operator=(const CentralCache&) = delete;

    //static CentralCache s_inst;

public:
    static CentralCache& GetInstance(){
        static CentralCache s_inst;
        return s_inst;
    }

    // 从中心缓存获取一定数量的对象给thread cache
    size_t FetchRangeObj(void*& start, void*& end, size_t num, size_t size);

    // 将一定数量的对象释放到span跨度
    void ReleaseListToSpans(void* start, size_t size);

    // 从spanlist 或者 page cache获取一个span
    Span* GetOneSpan(size_t size);
};
// static CentralCache centralCacheInst;
#endif //CPLUSPLUS_CENTRALCACHE_H
