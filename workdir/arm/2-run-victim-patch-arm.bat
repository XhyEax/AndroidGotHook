@echo off
cd ..
:Loop
adb push ../inject/build/intermediates/cmake/debug/obj/armeabi-v7a/libinject.so /data/local/tmp/libinject-arm.so
adb push victim-patch-arm /data/local/tmp/
adb shell chmod +x /data/local/tmp/victim-patch-arm
adb shell /data/local/tmp/victim-patch-arm
pause
goto :Loop