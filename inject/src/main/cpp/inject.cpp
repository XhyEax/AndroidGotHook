
#include <cstdint>
#include <dlfcn.h>
#include <android/log.h>
#include <jni.h>

#include "gotutil.h"
#include "mylog.h"
#include "gotutil.h"

#if defined(__LP64__)
#define MODULE_NAME "victim-patch-arm64"
#else
#define MODULE_NAME "victim-patch-arm"
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
    // hackBySection or hackBySegment
    uintptr_t ori = hackBySegment(MODULE_NAME, "libc.so", "getpid",
                                  (uintptr_t) getpidReplace);
    getpidOri = (int (*)()) (ori);
}

//so加载时由linker调用
void __attribute__((constructor)) init() {
    LOGE("call from constructor\n");
    hack();
    LOGE("constructor finish.\n");
}

// JNI LoadNativeLibrary中调用
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    if (nullptr == vm) return JNI_ERR;
    LOGE("call from JNI_OnLoad\n");
    hack();
    return JNI_VERSION_1_6;
}