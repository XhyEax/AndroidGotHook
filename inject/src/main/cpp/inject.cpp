
#include <cstdint>
#include <dlfcn.h>
#include <android/log.h>

#include "gotutil.h"
#include "mylog.h"
#include "gotutil.h"

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

//so加载时由linker调用
void __attribute__((constructor)) init() {
//    uintptr_t ori = hackBySection(MODULE_PATH, "libc.so", "getpid",
//                                  (uintptr_t) getpidReplace);
    uintptr_t ori = hackBySegment(MODULE_PATH, "libc.so", "getpid",
                                  (uintptr_t) getpidReplace);
    getpidOri = (int (*)()) (ori);
}