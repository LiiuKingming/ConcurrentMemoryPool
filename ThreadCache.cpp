//
// Created by 28943 on 2020/2/20.
//

#include "ThreadCache.h"
#include "CentralCache.h"

void* ThreadCache::Allocte(size_t size){
    size_t index = SizeClass::ListIndex(size);
    FreeList& freeList = m_freelist[index];

    if(!freeList.Empty()){
        return freeList.Pop();
    } else {
        return FetchFromCentralCache(SizeClass::RoundUp(index));
    }
}

void ThreadCache::Deallocte(void* ptr, size_t size){
    size_t index = SizeClass::ListIndex(size); // ?
    FreeList& freeList = m_freelist[index];

    freeList.Push(ptr);

    // 释放对象时，若链表过长，回收内存回到中心堆
    //if(){ReleaseToCentralCache();}

}


void* ThreadCache::FetchFromCentralCache(size_t size) {
    size_t num = SizeClass::NumMoveSize(size);

    void* start = nullptr, *end = nullptr;
    size_t actualNum = centralCacheInst.FetchRangeObj(start, end, num, size);

    if (actualNum == 1){
        return start;
    } else {
        size_t index = SizeClass::ListIndex(size);
        FreeList& list = m_freelist[index];
        list.PushRange(NextObj(start), end);

        return start;
    }
}


/*
// 单元测试代码 舍弃
void* ThreadCache::FetchFromCentralCache(size_t index){
    size_t num = 20; // test number

    //模拟取内存对象的代码, 测试Tread Catche的逻辑正确性
    size_t size = (index + 1) * 8;
    char* start = (char*)malloc(size * num);
    char* cur = start;

    for(size_t i = 0; i < num - 1; ++i){
        char* next = cur + size;
        NextObj(cur) = next;

        cur = next;
    }

    NextObj(cur) = nullptr;

    void* head = NextObj(start);
    void* tail = cur;

    m_freelist[index].PushRange(head,tail);

    return start;

}
*/