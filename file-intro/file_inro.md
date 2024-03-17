# OSTEP实验

### part1：File-intro

关键：如何管理持久性设备？操作系统应如何管理持久性设备？有哪些 API？实施过程中有哪些重要环节？

首先概述 API：与 UNIX 文件系统交互时您期望看到的接口

两个关键的抽象概念：

+ 第一个是文件。文件只是一个字节的线性数组，每个字节都可以读取或写入。每个文件都有某种低级名称，通常是某种数量（索引节点号 (i-number): 索引节点号是文件系统中分配给每个文件的唯一标识符）；OS不了解文件结构，文件系统的职责只是将此类数据持久存储在磁盘上
+ 第二个是目录。目录也有一个低级名称（即索引节点号），但其内容非常具体：它包含一个（用户可读名称，低级名称）对的列表。

文件命名：文件系统提供的一件伟大的事情：一个方便的方式来命名我们感兴趣的所有文件。名称在系统中非常重要，因为访问任何资源的第一步就是能够对其进行命名。在UNIX系统中，文件系统提供了一种统一的方式来访问磁盘、U盘、CD - ROM等许多设备上的文件，而实际上许多其他的东西，都位于单目录树下。

文件系统接口：创建、访问和删除文件unlink ( )



#### CODE

+ stat命令，使用到stat结构体，该结构体用于表示文件的状态信息

```c
//留个空：关于ifdef和编译架构的写法，未接触过
struct stat
  {
    __dev_t st_dev;		/* 文件所在设备的ID  */
    __ino_t st_ino;		/* 文件的inode号，用于标识文件	*/
    __mode_t st_mode;			/* 文件的类型和权限  */
    __nlink_t st_nlink;			/* 链接到文件的硬链接数目  */
    __uid_t st_uid;		/* 文件拥有者的用户ID	*/
    __gid_t st_gid;		/* 文件所属组的组ID*/
    __dev_t st_rdev;		/* 设备文件的设备号  */
    __off_t st_size;			/* 文件大小  */
    __blksize_t st_blksize;	/* 文件系统 I/O 操作的最佳块大小  */
    __blkcnt_t st_blocks;		/* 分配给文件的 512 字节块的数目 */
    struct timespec st_atim;		/* 文件的最后访问时间  */
    struct timespec st_mtim;		/* 文件的最后修改时间  */
    struct timespec st_ctim;		/* 文件状态的最后更改时间  */
  };
```

+ stat中的mode

  + 100644：表示文件的所有者有读取和写入权限，而群组和其他用户只有读取权限。这是最常见的文件权限数字，适用于大多数普通文件
  + 存在的疑惑：mode是在创建时就被赋予的吗？那默认值部分的代码是什么样的呢？没能找到具体的入口函数

  

+ stat函数

  ```c
  int stat(const char *path, struct stat *buf)
  //path用于指定要获取状态信息的文件路径
  //buf用于存储获取到的文件状态信息
  {
  	struct statx statx;//用于存储从系统调用中获取到的文件状态信息
  	long ret;
  
      /*调用系统调用 sys_statx()，传递文件路径 path 和一些标志参数，以获取文件的基本状态信息。AT_FDCWD 表示使用当前工作目录的文件描述符，AT_NO_AUTOMOUNT 表示不自动挂载文件系统，STATX_BASIC_STATS 表示获取基本状态信息。&statx 用于传递存储状态信息的结构体指针。*/
  	ret = __sysret(sys_statx(AT_FDCWD, path, AT_NO_AUTOMOUNT, STATX_BASIC_STATS, &statx));
  	if (ret == -1)
  		return ret;
      
  	/*通过位运算和结构体成员赋值，将从系统调用中获取到的文件状态信息填充到传入的 buf 结构体中*/
      
      //这里 stx_dev_minor 和 stx_dev_major 分别表示设备号的次要和主要部分。通过位运算，将两部分合并为一个 st_dev。其中 0xff 用于提取次要部分的低8位，<< 8 将主要部分左移8位，然后将两部分合并
  	buf->st_dev          = ((statx.stx_dev_minor & 0xff)
  			       | (statx.stx_dev_major << 8)
  			       | ((statx.stx_dev_minor & ~0xff) << 12));
  	buf->st_ino          = statx.stx_ino;
  	buf->st_mode         = statx.stx_mode;
  	buf->st_nlink        = statx.stx_nlink;
  	buf->st_uid          = statx.stx_uid;
  	buf->st_gid          = statx.stx_gid;
  	buf->st_rdev         = ((statx.stx_rdev_minor & 0xff)
  			       | (statx.stx_rdev_major << 8)
  			       | ((statx.stx_rdev_minor & ~0xff) << 12));
  	buf->st_size         = statx.stx_size;
  	buf->st_blksize      = statx.stx_blksize;
  	buf->st_blocks       = statx.stx_blocks;
  	buf->st_atim.tv_sec  = statx.stx_atime.tv_sec;
  	buf->st_atim.tv_nsec = statx.stx_atime.tv_nsec;
  	buf->st_mtim.tv_sec  = statx.stx_mtime.tv_sec;
  	buf->st_mtim.tv_nsec = statx.stx_mtime.tv_nsec;
  	buf->st_ctim.tv_sec  = statx.stx_ctime.tv_sec;
  	buf->st_ctim.tv_nsec = statx.stx_ctime.tv_nsec;
  
  	return 0;
  }
  ```

+ stat中的宏定义，这些宏定义了文件类型的值，当与文件类型掩码 `__S_IFMT` 进行位与运算时，可以确定文件的具体类型。例如，`__S_IFDIR` 与 `__S_IFMT` 进行位与运算，如果结果不为零，则表示文件是一个目录。

  ```c
  # define S_IFMT		__S_IFMT //文件类型掩码，用于提取文件类型的部分
  # define S_IFDIR	__S_IFDIR
  # define S_IFCHR	__S_IFCHR
  # define S_IFBLK	__S_IFBLK
  # define S_IFREG	__S_IFREG
  ```

  ```c
  #define	__S_IFMT	0170000	/* These bits determine file type.最高三位用于确定文件类型  */
  
  /* File types.  */
  #define	__S_IFDIR	0040000	/* Directory.  */
  #define	__S_IFCHR	0020000	/* Character device.  */
  #define	__S_IFBLK	0060000	/* Block device.  */
  #define	__S_IFREG	0100000	/* Regular file.  */
  #define	__S_IFIFO	0010000	/* FIFO.  */
  #define	__S_IFLNK	0120000	/* Symbolic link.  */
  #define	__S_IFSOCK	0140000	/* Socket.  */
  ```

+ statx 联系到的系统调用内联汇编

  ```c
  #define my_syscall5(num, arg1, arg2, arg3, arg4, arg5)                        \
  ({                                                                            \
  	long _ret;                                                            \
  	
  	//register long _num __asm__ ("rax") = (num);：这行代码定义了一个 long 类型的变量 _num，并将其与 rax 寄存器绑定，作为系统调用号的输入
  	register long _num  __asm__ ("rax") = (num);                          \
      
      //register long _arg1 __asm__ ("rdi") = (long)(arg1); 到 register long _arg5 __asm__ ("r8") = (long)(arg5);：这几行代码定义了五个 long 类型的变量，分别将它们与 rdi、rsi、rdx、r10、r8 寄存器绑定，作为系统调用的参数输入
  	register long _arg1 __asm__ ("rdi") = (long)(arg1);                   \
  	register long _arg2 __asm__ ("rsi") = (long)(arg2);                   \
  	register long _arg3 __asm__ ("rdx") = (long)(arg3);                   \
  	register long _arg4 __asm__ ("r10") = (long)(arg4);                   \
  	register long _arg5 __asm__ ("r8")  = (long)(arg5);                   \
  									      \
     //这段代码是内联汇编部分。它使用 syscall 指令触发系统调用，并将返回值保存在 _ret 变量中。此外，它使用约束（constraints）将参数绑定到相应的寄存器中，以便传递给系统调用
  	__asm__ volatile (                                                    \
  		"syscall\n"                                                   \
  		: "=a"(_ret)                                                  \
  		: "r"(_arg1), "r"(_arg2), "r"(_arg3), "r"(_arg4), "r"(_arg5), \
  		  "0"(_num)                                                   \
  		: "rcx", "r11", "memory", "cc"                                \
  	);                                                                    \
  	_ret;                                                                 \
  })
  ```

  

  