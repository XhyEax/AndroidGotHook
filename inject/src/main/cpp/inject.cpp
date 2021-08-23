
#include <cstdint>
#include <dlfcn.h>
#include <android/log.h>

#include "gotutil.h"

#define LOG_TAG   "NInject"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

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

void hack() {
    LOGI("hack start.\n");
    // 获取目标函数地址
    void *handle = dlopen("libc.so", RTLD_LAZY);
    getpidOri = (int (*)()) dlsym(handle, "getpid");
    LOGI("hack getpid addr: %X\n", getpidOri);
    uintptr_t GOTBase = 0;
    int GOTSize = 0;
    // 获取GOT表地址及大小
    getGOTBase(GOTBase, GOTSize, "/data/local/tmp/victim-patch");
    // 遍历GOT表，替换目标函数地址
    if (GOTSize > 0)
        replaceGOT(GOTBase, GOTSize, (uintptr_t) getpidReplace, (uintptr_t) getpidOri);
}

//so加载时自动调用
void __attribute__((constructor)) __init() {
    hack();
}