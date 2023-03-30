# OSH lab1
**罗浩铭PB21030838**

## 内核裁剪
> 分析哪些选项能够切实地影响编译后的内核大小或编译的时间


1. 我们先去除了可视化、电源管理、IA32模拟器、无线网络、硬件驱动（无线网络、图形渲染等不需要支持的硬件），得到的内核大小为8.8MB
2. 我们再去除了安全、加密算法、部分文件系统管理（监控目录变更的inotify、管理磁盘限额的quota）、几乎全部硬件驱动（除了real time clock）、block layer的几乎全部（除了Legacy autoloading support），得到的内核大小为6.8MB，编译时间为11分45秒
  

注：获取编译时间所用命令为
```txt
echo $(date "+%Y-%m-%d %H:%M:%S") && make -j1 && echo $(date "+%Y-%m-%d %H:%M:%S")
```



## 交叉编译















## 附：实验要求
实验一完成后，你的 osh-2023-labs 作业仓库应具有这样的结构：

```txt
osh-2023-labs
- README.md
- lab0
- lab1
  - README.md      // 可选，建议在此文件中介绍你的实验过程
  - syscall
    - bzImage
    - initrd.c
  - riscv/arm/...  // 可选，该目录命名为选择的交叉编译的平台（如选择 arm 则命名为 arm）
    - bzImage      // 交叉编译得到的内核镜像文件
  - bzImage
  - .config        // 可选，编译内核时的配置文件
  - initrd.cpio.gz
```









