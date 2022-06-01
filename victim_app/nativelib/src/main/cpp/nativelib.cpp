#include <jni.h>
#include <string>
#include <unistd.h>

#include <android/log.h>

#define LOG_TAG   "NInject-Native"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

#include "gotutil.h"

// 原方法的备份
int (*getpidOri)();

// 替换方法
int getpidReplace() {
    LOGI("before hook getpid\n");
    //调用原方法
    int pid = (int) getpidOri();
    LOGI("after hook getpid: %d\n", pid);
    return 23333;
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    uintptr_t ori = hackBySection("libnativelib.so", "libc.so", "getpid",
                                  (uintptr_t) getpidReplace);
//    uintptr_t ori = hackBySegment("libnativelib.so", "libc.so", "getpid",
//                                  (uintptr_t) getpidReplace);
    getpidOri = (int (*)()) (ori);
    return JNI_VERSION_1_6;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_xhy_nativelib_NativeLib_stringFromJNI(
        JNIEnv *env,
        jclass clazz) {
    int pid = getpid();
    LOGI("native-lib getpid: %d\n", pid);
    std::string hello = std::to_string(pid);
    return env->NewStringUTF(hello.c_str());
}