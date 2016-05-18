boot, inc 和 lib 目录下的代码来自[JOS](https://pdos.csail.mit.edu/6.828/2014/).

尝试脱离框架，体验从零开始编写内核代码的酸爽。

编译执行：
```shell
make qemu
```

GDB 调试：
```shell
# 终端 A
make debug
# 终端 B
make gdb
```

简单文件系统测试：
```shell
make gen-disk
make extract
./gen-disk <one-file>
./extract a.disk <another-file>
diff <one-file> <another-file>
```

## 进度

- [x]  引导
- [x]  小游戏 (用户程序)
- [x]  分页
- [ ]  内存管理
- [ ]  系统调用
- [ ]  上下文切换
- [ ]  文件系统
- [ ]  Shell
