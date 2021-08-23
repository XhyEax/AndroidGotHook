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

#define LOG_TAG   "NInject"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

#define PAGE_START(addr) ((addr) & PAGE_MASK)
#define PAGE_END(addr)   (PAGE_START(addr) + PAGE_SIZE)

uintptr_t getModuleBase(const char *module_path) {
    uintptr_t addr = 0;
    char buff[256] = "\n";

    FILE *fp = fopen("/proc/self/maps", "r");
    while (fgets(buff, sizeof(buff), fp)) {
        if (strstr(buff, "r-xp") && strstr(buff, module_path) &&
            sscanf(buff, "%lx", &addr) == 1)
            break;
    }
//    LOGI("buff: %s", buff);
    fclose(fp);
    return addr;
}

void getGOTBase(uintptr_t &GOTBase, int &GOTSize, const char *module_path) {
    uintptr_t moduleBase = getModuleBase(module_path);
    LOGI("hack moduleBase: %X\n", moduleBase);
    if (moduleBase != 0) {
        int GOTOffset = 0;
        getGOTOffsetAndSize(module_path, GOTOffset, GOTSize);
        GOTBase = moduleBase + GOTOffset;
        LOGI("GOTBase: %X GOTSize: %d\n", GOTBase, GOTSize);
    }
}

void replaceGOT(uintptr_t GOTBase, int GOTSize, uintptr_t replace, uintptr_t ori) {
    for (int i = 0; i < GOTSize; ++i) {
        uintptr_t addr = GOTBase + i * 4;
        uintptr_t item = *(uintptr_t *) (addr);
//        LOGI("GOT [%d]: %X\n", i, item);
        if (item == ori) {
            LOGI("replace %X to %X\n", ori, replace);
            mprotect((void *) PAGE_START(addr), PAGE_SIZE, PROT_READ | PROT_WRITE);
            *(uintptr_t *) addr = replace;
            __builtin___clear_cache((char *) PAGE_START(addr), (char *) PAGE_END(addr));
            return;
        }
    }

    LOGI("replaceGOT %X not found!\n", ori);
}
