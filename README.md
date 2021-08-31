## Simple Android ARM&ARM64 GOT Hook

详见：[简易Android ARM&ARM64 GOT Hook (一)](https://blog.xhyeax.com/2021/08/23/android-arm-got-hook/)
[简易Android ARM&ARM64 GOT Hook (二)](https://blog.xhyeax.com/2021/08/30/android-arm-plt-hook/)

## 编译
使用`Android Studio`打开项目，点击`Make Project`

编译完成后，在`模块名/build/intermediates/cmake/debug/obj/CPU架构/`目录下，

可以找到生成的可执行文件(`victim`)和动态库(`libinject.so`)

## 测试
`workdir`文件中包含补丁脚本、`elf-cleaner`、adb测试脚本。编译完成后按顺序运行即可

## 存在的问题
1. 未绕过`dlopen`命名空间限制，在`Android 7`以上无法打开非公共库
2. 未hook`dlopen`，无法实时修改加载模块的GOT表
3. 基于maps解析，兼容性可能存在一定问题
4. 基于静态注入，无法绕过完整性检测
5. 未提供卸载函数，无法恢复GOT表
6. ...

## 总结
通过本项目，学习了`GOT Hook`原理，ELF文件结构，导入符号的查找方式以及`ARM64`段错误的处理，目的已基本达到。虽然功能还不够完善，但短期内应该不会再改动了（这次是真的了）。

实际应用可以考虑使用字节的[bhook](https://github.com/bytedance/bhook)

## 参考
[android中基于plt/got的hook实现原理](https://blog.csdn.net/byhook/article/details/103500524)
[聊聊Linux动态链接中的PLT和GOT(2)——延迟重定位]([https://linyt.blog.csdn.net/article/details/51636753])
[WARNING: linker: unsupported flags DT_FLAGS_1=0x8000001](https://github.com/termux/termux-packages/issues/4894)
[constructor属性函数在动态库加载中的执行顺序](https://zhuanlan.zhihu.com/p/108274829)
[Android7.0以上命名空间详解(dlopen限制)](https://www.52pojie.cn/thread-948942-1-1.html)
[Android中GOT表HOOK手动实现](https://blog.csdn.net/u011247544/article/details/78564564)
[Android GOT Hook](https://www.cnblogs.com/mmmmar/p/8228391.html)

[基于Android的ELF PLT/GOT符号和重定向过程ELF Hook实现](https://blog.csdn.net/L173864930/article/details/40507359)
[ELF文件格式与got表hook简单实现](https://bbs.pediy.com/thread-267842.htm)
[重定位节 - 链接程序和库指南](https://docs.oracle.com/cd/E26926_01/html/E25910/chapter6-54839.html)
[符号表节 - 链接程序和库指南](https://docs.oracle.com/cd/E26926_01/html/E25910/chapter6-79797.html)
[散列表节 - 链接程序和库指南](https://docs.oracle.com/cd/E26926_01/html/E25910/chapter6-48031.html)
[动态节 - 链接程序和库指南](https://docs.oracle.com/cd/E26926_01/html/E25910/chapter6-42444.html)
[ELF文件结构详解](https://bbs.pediy.com/thread-255670.htm)
[PLT HOOK](https://zhuanlan.zhihu.com/p/269441842)
[bhook](https://github.com/bytedance/bhook)
[xhook](https://github.com/iqiyi/xhook)
