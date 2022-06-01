
#include <cstdint>
#include <dlfcn.h>
#include <android/log.h>
#include <jni.h>

#include "gotutil.h"
#include "mylog.h"
#include "gotutil.h"

#if defined(__LP64__)
#define MODULE_PATH "victim-patch-arm64"
#else
#define MODULE_PATH "victim-patch-arm"
#endif

// 原方法的备份
int (*getpidOri)();

// 替换方法
int getpidReplace() {
    LOGE("before hook getpid\n");
    //调用原方法
    int pid = (int) getpidOri();
    LOGE("after hook getpid: %d\n", pid);
    return 233333;
}

void hack() {
//    uintptr_t ori = hackBySection(MODULE_PATH, "libc.so", "getpid",
//                                  (uintptr_t) getpidReplace);
    uintptr_t ori = hackBySegment(MODULE_PATH, "libc.so", "getpid",
                                  (uintptr_t) getpidReplace);
    getpidOri = (int (*)()) (ori);
}

//so加载时由linker调用
void __attribute__((constructor)) init() {
    LOGE("call from constructor\n");
    hack();
}

// JNI LoadNativeLibrary中调用
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    if (NULL == vm) return JNI_ERR;
    LOGE("call from JNI_OnLoad\n");
    hack();
    return JNI_VERSION_1_6;
}