//
// Created by Xhy on 2021/8/23.
//

#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/mman.h>
#include <android/log.h>

#include "gotutil.h"
#include "elfutil.h"
#include "mylog.h"

#define PAGE_START(addr) ((addr) & PAGE_MASK)
#define PAGE_END(addr)   (PAGE_START(addr) + PAGE_SIZE)

uintptr_t getGOTBase(int &GOTSize, const char *modulePath) {
    uintptr_t GOTBase = 0;
    uintptr_t moduleBase = getModuleBase(modulePath);
    if (moduleBase == 0) {
        return GOTBase;
    }
    int GOTOffset = getGOTOffsetAndSize(modulePath, GOTSize);
    if (GOTOffset == 0) {
        LOGE("GOTOffset not found!\n");
        return GOTBase;
    }

    GOTBase = moduleBase + GOTOffset;
    LOGE("GOTOffset: %lX GOTBase: %lX GOTSize: %d\n", GOTOffset, GOTBase, GOTSize);
    return GOTBase;
}

uintptr_t getSymAddrInGOT(uintptr_t GOTBase, int GOTSize, uintptr_t ori) {
    if (GOTBase == 0) {
        LOGE("getSymAddrInGOT failed! addr [%lX] is wrong\n", GOTBase);
        return 0;
    }

    for (int i = 0; i < GOTSize; ++i) {
        uintptr_t addr = GOTBase + i * 4;
        uintptr_t item = *(uintptr_t *) (addr);
//        LOGE("GOT [%d]: %lX\n", i, item);
        if (item == ori) {
            return addr;
        }
    }

    LOGE("getSymAddrInGOT %lX not found!\n", ori);
    return 0;
}

void replaceFunction(uintptr_t addr, uintptr_t replace, uintptr_t ori) {
    if (addr == 0) {
        LOGE("replace failed! addr [%lX] is wrong\n", addr);
        return;
    }
    // 比对函数地址
    uintptr_t item = *(uintptr_t *) (addr);
    if (item == replace) {
        LOGE("[%lX] function has been replaced.\n", addr);
        return;
    }
    if (item != ori) {
        LOGE("replace failed! unexpected function address [%lX]=%lX\n", addr, item);
        return;
    }
    //修改权限、替换地址、清空指令缓存
    LOGE("replace [%lX]=%lX to %lX\n", addr, item, replace);
    mprotect((void *) PAGE_START(addr), PAGE_SIZE, PROT_READ | PROT_WRITE);
    *(uintptr_t *) addr = replace;
    __builtin___clear_cache((char *) PAGE_START(addr), (char *) PAGE_END(addr));
}


// 基于链接视图解析ELF
uintptr_t hackBySection(const char *moudle_path, const char *target_lib, const char *target_func,
                        uintptr_t replace) {
    LOGE("hack start.\n");
    // 获取目标函数地址
    void *handle = dlopen(target_lib, RTLD_LAZY);
    auto ori = (uintptr_t) dlsym(handle, target_func);
    LOGE("hack ori addr: %lX\n", ori);
    int GOTSize = 0;
    // 获取GOT表地址及大小 (解析Section)
    uintptr_t GOTBase = getGOTBase(GOTSize, moudle_path);
    // 遍历GOT表，查找符号地址
    uintptr_t replaceAddr = getSymAddrInGOT(GOTBase, GOTSize, ori);
    // 替换地址
    replaceFunction(replaceAddr, replace, ori);
    return ori;
}

// 基于执行视图解析ELF
uintptr_t hackBySegment(const char *moudle_path, const char *target_lib, const char *target_func,
                        uintptr_t replace) {
    LOGE("hackDynamic start.\n");
    // 获取目标函数地址
    void *handle = dlopen(target_lib, RTLD_LAZY);
    auto ori = (uintptr_t) dlsym(handle, target_func);
    LOGE("hackDynamic getpid addr: %lX\n", ori);
    // 获取符号地址 (解析Segment)
    uintptr_t replaceAddr = getSymAddrDynamic(moudle_path, target_func);
    // 替换地址
    replaceFunction(replaceAddr, replace, ori);
    return ori;
}
