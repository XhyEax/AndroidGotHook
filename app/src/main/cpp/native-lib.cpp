#include <android/log.h>

#define LOG_TAG   "NInject"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)


void hack() {
    LOGI("native start.\n");
}

//so加载时自动调用
void __attribute__((constructor)) __init() {
    hack();
}