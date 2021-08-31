@echo off
cd ..
:Loop
adb push ../inject/build/intermediates/cmake/debug/obj/arm64-v8a/libinject.so /data/local/tmp/libinject-arm64.so
adb push victim-patch-arm64 /data/local/tmp/
adb shell chmod +x /data/local/tmp/victim-patch-arm64
adb shell /data/local/tmp/victim-patch-arm64
pause
goto :Loop