//
// Created by 28943 on 2020/2/20.
//
#ifndef CPLUSPLUS_THREADCACHE_H
#define CPLUSPLUS_THREADCACHE_H

#include "Common.h"

class ThreadCache{
private:
    // 调用默认构造初始化
    FreeList m_freelist[NFREE_LIST];

public:
    // 申请内存, size表示对象的大小
    void* Allocte(size_t size);

    // 释放内存
    void Deallocte(void* ptr, size_t size);

    //从中心缓存获取对象
    void* FetchFromCentralCache(size_t index);

    // 如果自由链表中对象超过一定长度就要释放给中心缓存
    void ListTooLong(FreeList& freeList, size_t num);
};

#endif //CPLUSPLUS_THREADCACHE_H