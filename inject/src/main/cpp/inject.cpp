
#include <cstdint>
#include <dlfcn.h>
#include <android/log.h>

#include "gotutil.h"
#include "elfutil.h"
#include "mylog.h"

#if defined(__LP64__)
#define MODULE_PATH "/data/local/tmp/victim-patch-arm64"
#else
#define MODULE_PATH "/data/local/tmp/victim-patch-arm"
#endif

// 原方法的备份
int (*getpidOri)();

// 替换方法
int getpidReplace() {
    LOGI("before hook getpid\n");
    //调用原方法
    int pid = (int) getpidOri();
    LOGI("after hook getpid: %d\n", pid);
    return 233333;
}

// 基于链接视图解析ELF
void hack() {
    LOGI("hack start.\n");
    // 获取目标函数地址
    void *handle = dlopen("libc.so", RTLD_LAZY);
    getpidOri = (int (*)()) dlsym(handle, "getpid");
    LOGI("hack getpid addr: %lX\n", getpidOri);
    int GOTSize = 0;
    // 获取GOT表地址及大小 (解析Section)
    uintptr_t GOTBase = getGOTBase(GOTSize, MODULE_PATH);
    // 遍历GOT表，查找符号地址
    uintptr_t replaceAddr = getSymAddrInGOT(GOTBase, GOTSize, (uintptr_t) getpidOri);
    // 替换地址
    replaceFunction(replaceAddr, (uintptr_t) getpidReplace, (uintptr_t) getpidOri);
}

// 基于执行视图解析ELF
void hackDynamic() {
    LOGI("hackDynamic start.\n");
    // 获取目标函数地址
    void *handle = dlopen("libc.so", RTLD_LAZY);
    getpidOri = (int (*)()) dlsym(handle, "getpid");
    LOGI("hackDynamic getpid addr: %lX\n", getpidOri);
    // 获取符号地址 (解析Segment)
    uintptr_t replaceAddr = getSymAddrDynamic(MODULE_PATH, "getpid");
    // 替换地址
    replaceFunction(replaceAddr, (uintptr_t) getpidReplace, (uintptr_t) getpidOri);
}

//so加载时自动调用
void __attribute__((constructor)) __init() {
    hack();
//    hackDynamic();
}