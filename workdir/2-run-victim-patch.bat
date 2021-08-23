@echo off
adb push ../inject/build/intermediates/cmake/debug/obj/armeabi-v7a/libinject.so /data/local/tmp/
adb push victim-patch /data/local/tmp/
adb shell chmod +x /data/local/tmp/victim-patch
adb shell /data/local/tmp/victim-patch
pause