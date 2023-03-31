# OSH lab1
**罗浩铭PB21030838**

## 内核裁剪
> 分析哪些选项能够切实地影响编译后的内核大小或编译的时间


1. 我们先去除了可视化、电源管理、IA32模拟器、无线网络、硬件驱动（无线网络、图形渲染等不需要支持的硬件），得到的内核大小为8.8MB
2. 我们再去除了安全、加密算法、部分文件系统管理（监控目录变更的inotify、管理磁盘限额的quota）、几乎全部硬件驱动（除了real time clock）、block layer的几乎全部（除了Legacy autoloading support），得到的内核大小为6.8MB，编译时间为11分45秒
3. 我们接着去除了Plan 9支持、ELF与MISC二进制文件支持、部分分支预测功能（IBPB viz. Indirect Branch Predictor Barrier, IBRS viz. Indirect Branch Restricted Speculation），得到的内核大小为6.5MB，编译时间为11分33秒
4. 再去除kernel hacking中除了Stack backtrace support的全部，得到的内核大小为5.5MB，编译时间为10分44秒
5. 接着去除网络中的RF switch子系统支持、kprobes、缓冲区溢出检测、seccomp机制（用于安全执行不受信任的代码）、无名内存分页，得到的内核大小为5.4MB，编译时间为10分45秒
6. 添加回kprobes、缓冲区溢出检测、无名内存分页，去除CPU运行情况监控、Mitigations for speculative execution vulnerabilities的全部，得到的内核大小为4.9MB，编译时间为11分03秒
7. 最后一轮，去除所有网络支持，得到的内核大小为3.3MB，编译时间为6分32秒
  
则我们大致可以得到：影响内核大小及编译时间的主要因素为：网络协议栈、可视化（不确定）、硬件驱动（不确定）、漏洞缓解补丁、内核调试等（已经大致按影响程度排序）
  

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









