# Exec系列函数Hook总结

## 新添加的Hook函数

本次更新为 `syscall_hook_fixed.c` 添加了完整的exec系列函数hook，以确保能够捕获make工具调用gcc/g++编译器的过程。

### 1. execv()
```c
int execv(const char *path, char *const argv[])
```
- **作用**: 执行指定路径的程序，参数以数组形式传递
- **重要性**: execl()函数内部会调用此函数，需要hook以避免遗漏

### 2. execve()
```c
int execve(const char *path, char *const argv[], char *const envp[])
```
- **作用**: 最底层的exec函数，可以指定环境变量
- **重要性**: 很多高级exec函数最终会调用此函数

### 3. execvp() ⭐**最重要**
```c
int execvp(const char *file, char *const argv[])
```
- **作用**: 在PATH环境变量中查找程序并执行
- **重要性**: **make工具最常使用此函数**来调用gcc/g++等编译器

### 4. execvpe()
```c
int execvpe(const char *file, char *const argv[], char *const envp[])
```
- **作用**: 在PATH中查找程序并执行，可指定环境变量
- **重要性**: 一些需要特定环境变量的程序调用会使用此函数

### 5. execlp()
```c
int execlp(const char *file, const char *arg, ...)
```
- **作用**: 在PATH中查找程序并执行，参数以可变参数形式传递
- **重要性**: 某些脚本或程序可能使用此函数

### 6. execle()
```c
int execle(const char *path, const char *arg, ...)
```
- **作用**: 执行指定路径程序，可指定环境变量，参数以可变参数形式传递
- **重要性**: 需要特定环境变量的程序调用

### 7. system()
```c
int system(const char *command)
```
- **作用**: 通过shell执行系统命令
- **重要性**: 一些构建脚本可能使用此函数调用编译器

## 修复的问题

### 1. 递归调用问题
- **问题**: 原来的execl()函数调用了execv()，但execv()没有被hook，导致实际调用被绕过
- **解决**: 修改execl()调用real_execv()而不是execv()

### 2. 覆盖不全问题
- **问题**: 只hook了execl()一个函数，遗漏了make常用的execvp()等函数
- **解决**: 添加了完整的exec系列函数hook

## 测试方法

运行以下命令测试新的hook功能：

```bash
# 测试make调用gcc的hook
./test_make_hook.sh

# 或者手动测试
export LD_PRELOAD="$(pwd)/syscall_hook_fixed.so"
make
unset LD_PRELOAD
```

## 预期结果

执行测试后，应该能在日志中看到：

1. **fork调用**: make创建子进程
2. **execvp调用**: make调用g++编译器
3. **完整的命令参数**: `g++ -std=c++17 -Wall -Wextra -Wpedantic -g -O2 -o hello hello.cpp`
4. **编译过程的系统调用**: g++编译器执行时的文件访问等操作

## 常见问题排查

如果仍然无法捕获make→gcc调用：

1. **检查make版本**: 某些make实现可能使用特殊机制
2. **检查vfork()**: 有些系统使用vfork()而不是fork()
3. **检查posix_spawn()**: 现代系统可能使用posix_spawn()创建进程
4. **检查内置命令**: 某些shell可能有内置的编译器调用机制

## 文件说明

- `syscall_hook_fixed.c`: 更新后的hook实现
- `test_make_hook.sh`: 专门测试make调用的脚本
- `syscall_hook_fixed.sh`: 原有的测试脚本
- `exec_hook_summary.md`: 本说明文件