# 函数指针分类说明

## 概述
`syscall_hook_fixed.c` 中定义了 **19个函数指针**，用于保存原始系统调用函数的地址。这些函数指针按功能可以分为以下几个类别：

---

## 📂 **1. 进程管理类 (Process Management)**
> 用于进程的创建、执行、等待和信息获取

```c
// 进程创建
static pid_t (*real_fork)(void) = NULL;

// 进程执行 - exec系列函数 (7个)
static int (*real_execl)(const char *path, const char *arg, ...) = NULL;
static int (*real_execlp)(const char *file, const char *arg, ...) = NULL;
static int (*real_execle)(const char *path, const char *arg, ...) = NULL;
static int (*real_execv)(const char *path, char *const argv[]) = NULL;
static int (*real_execve)(const char *path, char *const argv[], char *const envp[]) = NULL;
static int (*real_execvp)(const char *file, char *const argv[]) = NULL;
static int (*real_execvpe)(const char *file, char *const argv[], char *const envp[]) = NULL;

// 系统命令执行
static int (*real_system)(const char *command) = NULL;

// 进程等待
static pid_t (*real_wait)(int *status) = NULL;

// 进程信息获取
static pid_t (*real_getpid)(void) = NULL;
```

**功能说明：**
- `fork()`: 创建子进程
- `exec*()`: 用新程序替换当前进程映像
- `system()`: 执行shell命令
- `wait()`: 等待子进程结束
- `getpid()`: 获取当前进程ID

---

## 📁 **2. 文件操作类 (File Operations)**
> 用于文件和目录的操作

```c
// 文件基本操作
static int (*real_open)(const char *pathname, int flags, ...) = NULL;
static ssize_t (*real_write)(int fd, const void *buf, size_t count) = NULL;
static int (*real_close)(int fd) = NULL;

// 文件权限和状态
static int (*real_access)(const char *pathname, int mode) = NULL;

// 文件系统操作
static int (*real_unlink)(const char *pathname) = NULL;
static char* (*real_getcwd)(char *buf, size_t size) = NULL;
```

**功能说明：**
- `open()`: 打开文件或创建文件
- `write()`: 向文件描述符写入数据
- `close()`: 关闭文件描述符
- `access()`: 检查文件的访问权限
- `unlink()`: 删除文件
- `getcwd()`: 获取当前工作目录

---

## 👤 **3. 用户信息类 (User Information)**
> 用于获取用户和权限信息

```c
// 用户身份
static uid_t (*real_getuid)(void) = NULL;
```

**功能说明：**
- `getuid()`: 获取当前进程的真实用户ID

---

## ⏰ **4. 系统时间类 (System Timing)**
> 用于时间控制和延迟

```c
// 时间延迟
static unsigned int (*real_sleep)(unsigned int seconds) = NULL;
```

**功能说明：**
- `sleep()`: 使当前进程休眠指定秒数

---

## 📊 **统计总结**

| 类别 | 函数数量 | 主要用途 |
|------|----------|----------|
| **进程管理** | 10个 | 进程创建、执行、管理 |
| **文件操作** | 6个 | 文件I/O、文件系统操作 |
| **用户信息** | 1个 | 用户身份和权限 |
| **系统时间** | 1个 | 时间控制 |
| **总计** | **18个** | 系统调用Hook |

## 🎯 **核心作用**

这些函数指针的主要作用是：

1. **保存原始函数地址**: 通过 `dlsym(RTLD_NEXT, "function_name")` 获取
2. **实现透明Hook**: 在记录日志后调用原始函数
3. **避免递归调用**: 防止Hook函数调用自己
4. **完整性监控**: 覆盖程序执行的关键系统调用

## ⚠️ **注意事项**

- 所有函数指针初始值为 `NULL`
- 在首次调用时通过 `dlsym()` 动态获取真实函数地址
- Hook函数会先记录日志，再调用原始函数
- 特别重要的是 **exec系列函数**，用于捕获程序执行链