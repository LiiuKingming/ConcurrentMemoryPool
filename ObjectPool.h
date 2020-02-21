//
// Created by 28943 on 2020/3/1.
//

#ifndef CPLUSPLUS_OBJECTPOOL_H
#define CPLUSPLUS_OBJECTPOOL_H

#include "Common.h"
#include <windows.h>

template<class T, size_t initNum = 100>
class ObjectPool{
private:
    char* m_start;
    char* m_end;       // 管理内存池指针
    size_t m_itemSize;

    T* m_freeList;
public:
    ObjectPool() {
        m_itemSize = sizeof(T) < sizeof(T*) ? sizeof(T*) : sizeof(T);

        //m_start = (char*)malloc(initNum * m_itemSize);
#ifdef _WIN32
        m_start = VirtualAlloc(0, initNum * m_itemSize,
                               MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
        // brk mmap
#endif
        m_end = m_start + initNum * m_itemSize;
        m_freeList = nullptr;
    }

    T*& Next_Obj(T* obj) {
        return  *(T**)obj;
    }

    //T* Alloc()
    T* New() {
        //1.如果freeList有对象，优先取这里的
        //2.否则再到池里面取
        T* obj = nullptr;
        if (m_freeList != nullptr) {
            obj = m_freeList;
            m_freeList = Next_Obj(obj);
        } else {
            if (m_start == m_end) {
                m_start = (char*)malloc(initNum * sizeof(T));
                m_end = m_start + initNum * m_itemSize;
            }

            obj = (T*)m_start;
            m_start += m_itemSize;
        }

        // new的定位表达式
        new(obj)T;

        return obj;
    }

    void Detele(T* ptr) {
        ptr->~T();

        Next_Obj(ptr) = m_freeList;
        m_freeList = ptr;
    }

};

#endif //CPLUSPLUS_OBJECTPOOL_H
