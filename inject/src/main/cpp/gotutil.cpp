//
// Created by Xhy on 2021/8/23.
//

#include <cstdlib>
#include <cstring>
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
        LOGI("moduleBase not found!\n");
        return GOTBase;
    }
    LOGI("hack moduleBase: %lX\n", moduleBase);
    int GOTOffset = getGOTOffsetAndSize(modulePath, GOTSize);
    if (GOTOffset == 0) {
        LOGI("GOTOffset not found!\n");
        return GOTBase;
    }

    GOTBase = moduleBase + GOTOffset;
    LOGI("GOTOffset: %lX GOTBase: %lX GOTSize: %d\n", GOTOffset, GOTBase, GOTSize);
    return GOTBase;
}

uintptr_t getSymAddrInGOT(uintptr_t GOTBase, int GOTSize, uintptr_t ori) {
    if (GOTBase == 0) {
        LOGI("getSymAddrInGOT failed! addr [%lX] is wrong\n", GOTBase);
        return 0;
    }

    for (int i = 0; i < GOTSize; ++i) {
        uintptr_t addr = GOTBase + i * 4;
        uintptr_t item = *(uintptr_t *) (addr);
//        LOGI("GOT [%d]: %lX\n", i, item);
        if (item == ori) {
            return addr;
        }
    }

    LOGI("getSymAddrInGOT %lX not found!\n", ori);
    return 0;
}

void replaceFunction(uintptr_t addr, uintptr_t replace, uintptr_t ori) {
    if (addr == 0) {
        LOGI("replace failed! addr [%lX] is wrong\n", addr);
        return;
    }
    // 比对函数地址
    uintptr_t item = *(uintptr_t *) (addr);
    if (item == replace) {
        LOGI("[%lX] function has been replaced.\n", addr);
        return;
    }
    if (item != ori) {
        LOGI("replace failed! unexpected function address [%lX]=%lX\n", addr, item);
        return;
    }
    //修改权限、替换地址、清空指令缓存
    LOGI("replace [%lX]=%lX to %lX\n", addr, ori, replace);
    mprotect((void *) PAGE_START(addr), PAGE_SIZE, PROT_READ | PROT_WRITE);
    *(uintptr_t *) addr = replace;
    __builtin___clear_cache((char *) PAGE_START(addr), (char *) PAGE_END(addr));
}

