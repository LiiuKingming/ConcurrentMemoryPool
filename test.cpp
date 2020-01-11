//
// Created by 28943 on 2020/2/20.
//
#include "ThreadCache.h"
#include <vector>

void UniThreadCache(){
    ThreadCache tc;
    vector<void*> v;

    for (size_t i = 0; i < 22; ++i){
        v.push_back(tc.Allocte(7));
    }

    for (size_t i = 0; i < v.size(); ++i){
       cout << "[" << i << "]" << v[i] << endl;
    }

    for (auto ptr : v){
        tc.Deallocte(ptr, 7);
    }
}

void UnitTestSizeClass()
{
    // 控制在[1%，10%]左右的内碎片浪费
    // [1,128] 8byte对齐 freelist[0,16)
    // [129,1024] 16byte对齐 freelist[16,72)
    // [1025,8*1024] 128byte对齐 freelist[72,128)
    // [8*1024+1,64*1024] 1024byte对齐 freelist[128,184)
    cout << SizeClass::RoundUp(1) << endl;
    cout << SizeClass::RoundUp(127) << endl;
    cout << endl;

    cout << SizeClass::RoundUp(129) << endl;
    cout << SizeClass::RoundUp(1023) << endl;
    cout << endl;

    cout << SizeClass::RoundUp(1025) << endl;
    cout << SizeClass::RoundUp(8*1024-1) << endl;
    cout << endl;

    cout << SizeClass::RoundUp(8 * 1024+1) << endl;
    cout << SizeClass::RoundUp(64 * 1024-1) << endl;
    cout << endl << endl;

    cout << SizeClass::ListIndex(1) << endl;
    cout << SizeClass::ListIndex(128) << endl;
    cout << endl;

    cout << SizeClass::ListIndex(129) << endl;
    cout << SizeClass::ListIndex(1023) << endl;
    cout << endl;

    cout << SizeClass::ListIndex(1025) << endl;
    cout << SizeClass::ListIndex(8 * 1024 - 1) << endl;
    cout << endl;

    cout << SizeClass::ListIndex(8 * 1024 + 1) << endl;
    cout << SizeClass::ListIndex(64 * 1024 - 1) << endl;
    cout << endl;
}

void UnitTestSystemAlloc()
{
    void* ptr = SystemAlloc(MAX_PAGES - 1);
    PAGE_ID id = (PAGE_ID)ptr >> PAGE_SHIFT;
    void* ptrshift = (void*)(id << PAGE_SHIFT);

    char* obj1 = (char*)ptr;
    char* obj2 = (char*)ptr + 8;
    char* obj3 = (char*)ptr + 16;
    PAGE_ID id1 = (PAGE_ID)obj1 >> PAGE_SHIFT;
    PAGE_ID id2 = (PAGE_ID)obj2 >> PAGE_SHIFT;
    PAGE_ID id3 = (PAGE_ID)obj3 >> PAGE_SHIFT;
}

int main(){
    // UniThreadCache();
    // UnitTestSizeClass();
    UnitTestSystemAlloc();

    return 0;
}





