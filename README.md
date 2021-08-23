## Simple Android GOT Hook (ARM)
实现过程详见：[实现简易Android GOT/PLT Hook (ARM)](https://blog.xhyeax.com/2021/08/23/android-arm-got-hook/)

## 编译
使用`Android Studio`打开项目，点击`Make Project`
编译完成后，在`模块名\build\intermediates\cmake\debug\obj\armeabi-v7a\`目录下，
可以找到生成的可执行文件(`victim`)和动态库(`libinject.so`)

## 测试
`workdir`文件中包含补丁脚本、`elf-cleaner`、adb测试脚本。编译完成后按顺序运行即可

## 存在的问题
1. 未绕过`dlopen`命名空间限制，在`Android 7`以上无法打开非公共库
2. 未hook`dlopen`，无法实时修改加载模块的GOT表
3. 基于链接视图（`Linking View`）静态解析ELF，无法hook加壳的so
4. 基于maps解析，兼容性可能存在一定问题
5. 基于静态注入，无法绕过完整性检测
6. 未提供卸载函数，无法恢复GOT表
7. ...

## 总结
通过本项目，学习了`GOT Hook`原理以及ELF文件结构，目的已基本达到。虽然功能还不够完善，但短期内应该不会再改进（俗话说得好：不要重复造轮子）。

实际应用可以考虑使用字节的[bhook](https://github.com/bytedance/bhook)