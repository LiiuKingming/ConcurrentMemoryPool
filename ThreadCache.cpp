//
// Created by 28943 on 2020/1/9.
//

#include "ThreadCache.h"
#include "CentralCache.h"

void* ThreadCache::Allocte(size_t size){
    size_t index = SizeClass::ListIndex(size);
    FreeList& freeList = m_freelist[index];

    if(!freeList.Empty()){
        return freeList.Pop();
    } else {
        return FetchFromCentralCache(SizeClass::RoundUp(size));
    }
}

void ThreadCache::Deallocte(void* ptr, size_t size){
    size_t index = SizeClass::ListIndex(size); // ?
    FreeList& freeList = m_freelist[index];

    freeList.Push(ptr);

    // 释放对象时，若链表过长，回收内存回到中心堆
    // 对象个数满足一定条件 或 内存大小超出一定值
    size_t num = SizeClass::NumMoveSize(size);
    if (freeList.Num() >= num){
        ListTooLong(freeList, num, size);
    }
}

void ThreadCache::ListTooLong(FreeList &freeList, size_t num, size_t size) {
    void* start = nullptr, *end = nullptr;
    freeList.PopRange(start, end, num);

    NextObj(end) = nullptr;
    // centralCacheInst.ReleaseListToSpans(start, size);
    CentralCache::GetInstance().ReleaseListToSpans(start, size);
}

void* ThreadCache::FetchFromCentralCache(size_t size) {
    size_t num = SizeClass::NumMoveSize(size);

    void* start = nullptr, *end = nullptr;
    //size_t actualNum = centralCacheInst.FetchRangeObj(start, end, num, size);
    size_t actualNum = CentralCache::GetInstance().FetchRangeObj(start, end, num, size);

    if (actualNum == 1){
        return start;
    } else {
        size_t index = SizeClass::ListIndex(size);
        FreeList& list = m_freelist[index];
        list.PushRange(NextObj(start), end, actualNum - 1);

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