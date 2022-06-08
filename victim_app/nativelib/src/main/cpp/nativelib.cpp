#include <jni.h>
#include <string>
#include <unistd.h>

#include <android/log.h>

#define LOG_TAG   "NInject-Native"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

#include "gotutil.h"

typedef int (*getpid_fun)();

// 全局函数指针
getpid_fun global_getpid = (getpid_fun) getpid;

// 原方法的备份
getpid_fun getpidOri;

// 替换方法
int getpidReplace() {
    LOGI("before hook getpid\n");
    //调用原方法
    int pid = getpidOri();
    LOGI("after hook getpid: %d\n", pid);
    return 23333;
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    // hackBySection or hackBySegment
    uintptr_t ori = hackBySegment("libnativelib.so", "libc.so", "getpid",
                                  (uintptr_t) getpidReplace);
    getpidOri = (getpid_fun) ori;
    return JNI_VERSION_1_6;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_xhy_nativelib_NativeLib_stringFromJNI(
        JNIEnv *env,
        jclass clazz) {
    auto local_getpid = getpid;
    int pid = getpid();
    int local_pid = local_getpid();
    int global_pid = global_getpid();

    char buff[256];
    sprintf(buff, "direct call: %d, local call: %d, global call: %d\n",
            pid, local_pid, global_pid);
    LOGI("%s", buff);

    return env->NewStringUTF(buff);
}