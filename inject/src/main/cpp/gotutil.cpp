//
// Created by Xhy on 2021/8/23.
//

#include <cstdlib>
#include <cstring>
#include <cinttypes>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/mman.h>
#include <android/log.h>

#include "gotutil.h"
#include "elfutil.h"
#include "mylog.h"

#define PAGE_START(addr) ((addr) & PAGE_MASK)
#define PAGE_END(addr)   (PAGE_START(addr) + PAGE_SIZE)

#define MAX_ADDRARRAY_SIZE 32

uintptr_t getGOTBase(int &GOTSize, const char *module_name) {
    uintptr_t GOTBase = 0;
    char moduleFullPath[256] = {0};
    uintptr_t moduleBase = getModuleBase(module_name, moduleFullPath);
    if (moduleBase == 0) {
        return GOTBase;
    }
    int GOTOffset = getGOTOffsetAndSize(moduleFullPath, GOTSize);
    if (GOTOffset == 0) {
        LOGE("GOTOffset not found!\n");
        return GOTBase;
    }

    GOTBase = moduleBase + GOTOffset;

    LOGE("GOTOffset: %" SCNxPTR " GOTBase: %" SCNxPTR " GOTSize: %d\n", GOTOffset, GOTBase,
         GOTSize);
    return GOTBase;
}

int getSymAddrInGOT(uintptr_t GOTBase, int GOTSize, uintptr_t ori, uintptr_t *addrArray) {
    if (GOTBase == 0) {
        LOGE("getSymAddrInGOT failed! addr [%" SCNxPTR "] is wrong\n", GOTBase);
        return 0;
    }

    int addrArraySize = 0;

    for (int i = 0; i < GOTSize; ++i) {
        uintptr_t addr = GOTBase + i * 4;
        uintptr_t item = *(uintptr_t *) (addr);
        if (item == ori) {
//            LOGE("GOT [%d]: %" SCNxPTR "\n", i, item);
            addrArray[addrArraySize++] = addr;
        }
    }
    if (addrArraySize == 0) {
        LOGE("getSymAddrInGOT %" SCNxPTR " not found!\n", ori);
    }

    return addrArraySize;
}

void replaceFunction(uintptr_t addr, uintptr_t replace, uintptr_t ori) {
    if (addr == 0) {
        LOGE("replace failed! addr [%" SCNxPTR "] is wrong\n", addr);
        return;
    }
    // 比对函数地址
    uintptr_t item = *(uintptr_t *) (addr);
    if (item == replace) {
        LOGE("[%" SCNxPTR "] function has been replaced.\n", addr);
        return;
    }
    if (item != ori) {
        LOGE("replace failed! unexpected function address [%" SCNxPTR "]=%" SCNxPTR "\n", addr,
             item);
        return;
    }
    //修改权限、替换地址、清空指令缓存
    LOGE("replace [%" SCNxPTR "]=%" SCNxPTR " with %" SCNxPTR "\n", addr, item, replace);
    mprotect((void *) PAGE_START(addr), PAGE_SIZE, PROT_READ | PROT_WRITE);
    *(uintptr_t *) addr = replace;
    __builtin___clear_cache((char *) PAGE_START(addr), (char *) PAGE_END(addr));
}


// 基于链接视图解析ELF
uintptr_t hackBySection(const char *module_name, const char *target_lib, const char *target_func,
                        uintptr_t replace) {
    LOGE("hackBySection start.\n");
    // 获取目标函数地址
    void *handle = dlopen(target_lib, RTLD_LAZY);
    auto ori = (uintptr_t) dlsym(handle, target_func);
//    LOGE("ori addr: %" SCNxPTR "\n", ori);
    int GOTSize = 0;
    // 获取GOT表地址及大小 (解析Section)
    uintptr_t GOTBase = getGOTBase(GOTSize, module_name);
    // 遍历GOT表，查找符号地址
    uintptr_t addrArray[MAX_ADDRARRAY_SIZE];
    int addrArraySize = getSymAddrInGOT(GOTBase, GOTSize, ori, addrArray);
    // 替换地址
    for (int i = 0; i < addrArraySize; i++) {
        uintptr_t replaceAddr = addrArray[i];
        replaceFunction(replaceAddr, replace, ori);
    }
    return ori;
}

// 基于执行视图解析ELF
uintptr_t hackBySegment(const char *module_name, const char *target_lib, const char *target_func,
                        uintptr_t replace) {
    LOGE("hackBySegment start.\n");
    // 获取目标函数地址
    void *handle = dlopen(target_lib, RTLD_LAZY);
    auto ori = (uintptr_t) dlsym(handle, target_func);
//    LOGE("ori addr: %" SCNxPTR "\n", ori);
    // 获取符号地址 (解析Segment)
    uintptr_t addrArray[MAX_ADDRARRAY_SIZE];
    int addrArraySize = getSymAddrDynamic(module_name, target_func, addrArray);
    // 替换地址
    for (int i = 0; i < addrArraySize; i++) {
        uintptr_t replaceAddr = addrArray[i];
        replaceFunction(replaceAddr, replace, ori);
    }
    return ori;
}
