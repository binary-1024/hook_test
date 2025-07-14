#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>
#include <stdarg.h>
#include <stdbool.h>
#include <dlfcn.h>
#include <spawn.h>
#include <time.h>


// 定义原始函数指针
static pid_t (*real_fork)(void) = NULL;
static int (*real_execl)(const char *path, const char *arg, ...) = NULL;
static int (*real_execlp)(const char *file, const char *arg, ...) = NULL;
static int (*real_execle)(const char *path, const char *arg, ...) = NULL;
static int (*real_execv)(const char *path, char *const argv[]) = NULL;
static int (*real_execve)(const char *path, char *const argv[], char *const envp[]) = NULL;
static int (*real_execvp)(const char *file, char *const argv[]) = NULL;
static int (*real_execvpe)(const char *file, char *const argv[], char *const envp[]) = NULL;
static int (*real_system)(const char *command) = NULL;
static pid_t (*real_wait)(int *status) = NULL;
static pid_t (*real_getpid)(void) = NULL;
static uid_t (*real_getuid)(void) = NULL;
static char* (*real_getcwd)(char *buf, size_t size) = NULL;
static int (*real_open)(const char *pathname, int flags, ...) = NULL;
static ssize_t (*real_write)(int fd, const void *buf, size_t count) = NULL;
static int (*real_close)(int fd) = NULL;
static int (*real_access)(const char *pathname, int mode) = NULL;
static unsigned int (*real_sleep)(unsigned int seconds) = NULL;
static int (*real_unlink)(const char *pathname) = NULL;
static int (*real_posix_spawn)( pid_t *pid,
                                const char *path,
                                const posix_spawn_file_actions_t *file_actions,
                                const posix_spawnattr_t *attrp,
                                char *const argv[], char *const envp[]
                            ) = NULL;
// 避免在log_syscall中产生递归的标志
static int logging_in_progress = 0;

// 简化的日志记录函数，避免调用可能被hook的函数
static void log_syscall(const char *syscall_name, const char *details) {
    if (logging_in_progress) return; // 防止递归
    logging_in_progress = 1;

    // 直接使用系统调用写入文件 utf-8 编码
    int fd = open("syscall_hook.log", O_CREAT | O_WRONLY | O_APPEND, 0644);
    if (fd >= 0) {
        char log_line[10240];
        int len = snprintf(log_line, sizeof(log_line),
                          "[PID:%d] %s: %s\n", getpid(), syscall_name, details);
        write(fd, log_line, len);
        close(fd);
    }

    // 输出到控制台
    printf("HOOK %s: %s\n", syscall_name, details);

    logging_in_progress = 0;
}

// Hook fork()
pid_t fork(void) {
    if (!real_fork) {
        real_fork = dlsym(RTLD_NEXT, "fork");
    }

    log_syscall("fork", "准备创建子进程");
    pid_t result = real_fork();

    char details[256];
    if (result == 0) {
        snprintf(details, sizeof(details), "子进程创建成功，当前在子进程中");
    } else if (result > 0) {
        snprintf(details, sizeof(details), "父进程中,子进程PID = %d", result);
    } else {
        snprintf(details, sizeof(details), "fork失败,返回 %d", result);
    }
    log_syscall("fork", details);

    return result;
}

// Hook execl() - 修复递归问题
int execl(const char *path, const char *arg, ...) {
    if (!real_execl) {
        real_execl = dlsym(RTLD_NEXT, "execl");
    }

    // 收集参数用于日志记录
    va_list args;
    va_start(args, arg);
    char cmd_details[10240];
    snprintf(cmd_details, sizeof(cmd_details), "执行命令: %s", path);

    // 收集所有参数到数组中
    const char *argv[10240]; // 假设不会超过32个参数
    int argc = 0;
    argv[argc++] = arg;

    const char *next_arg;
    while ((next_arg = va_arg(args, const char*)) != NULL && argc < 1023) {
        argv[argc++] = next_arg;
        strncat(cmd_details, " ", sizeof(cmd_details) - strlen(cmd_details) - 1);
        strncat(cmd_details, next_arg, sizeof(cmd_details) - strlen(cmd_details) - 1);
    }
    argv[argc] = NULL;
    va_end(args);

    log_syscall("execl", cmd_details);

    // 使用real_execv替代execl来避免变参问题和递归调用
    if (!real_execv) {
        real_execv = dlsym(RTLD_NEXT, "execv");
    }
    return real_execv(path, (char * const *)argv);
}

// Hook execv()
int execv(const char *path, char *const argv[]) {
    if (!real_execv) {
        real_execv = dlsym(RTLD_NEXT, "execv");
    }

    // 记录命令和参数
    char cmd_details[10240];
    snprintf(cmd_details, sizeof(cmd_details), "执行命令: %s", path);

    if (argv) {
        for (int i = 0; argv[i] != NULL; i++) {
            strncat(cmd_details, " ", sizeof(cmd_details) - strlen(cmd_details) - 1);
            strncat(cmd_details, argv[i], sizeof(cmd_details) - strlen(cmd_details) - 1);
        }
    }

    log_syscall("execv", cmd_details);
    return real_execv(path, argv);
}

// 引用全局变量 environ
extern char **environ;

// 修复函数签名和逻辑
char **copy_env_with_additions(char *const extra_vars[], char *const envp[]) {
    int env_count = 0;
    while (environ[env_count] != NULL) {
        env_count++;
    }

    // 正确计算 extra_vars 数组的长度
    int extra_count = 0;
    while (extra_vars[extra_count] != NULL) {
        extra_count++;
    }

    int envp_count = 0;
    while (envp[envp_count] != NULL) {
        envp_count++;
    }

    // 分配新数组：原有变量 + 新变量 + NULL
    char **new_env = malloc(sizeof(char *) * (env_count + extra_count + envp_count + 1));

    if (!new_env) {
        perror("malloc");
        exit(1);
    }

    // 首先复制 extra_vars
    memcpy(new_env, extra_vars, extra_count * sizeof(char *));

    // 然后复制原有环境变量
    memcpy(new_env + extra_count, environ, env_count * sizeof(char *));

    // 最后处理 envp 中的变量，避免重复
    int tmp_count = 0;
    for (int i = 0; i < envp_count; i++) {
        // 获取环境变量名部分（到 = 为止）
        char* equals = strchr(envp[i], '=');
        if (equals == NULL) continue;

        int name_len = equals - envp[i];
        char env_prefix[256];
        snprintf(env_prefix, sizeof(env_prefix), "%.*s=", name_len, envp[i]);

        bool found = false;

        // 检查是否在 extra_vars 中已存在
        for (int j = 0; j < extra_count; j++) {
            if (strncmp(extra_vars[j], env_prefix, strlen(env_prefix)) == 0) {
                found = true;
                break;
            }
        }

        // 检查是否在 environ 中已存在
        if (!found) {
            for (int j = 0; j < env_count; j++) {
                if (strncmp(environ[j], env_prefix, strlen(env_prefix)) == 0) {
                    found = true;
                    break;
                }
            }
        }

        if (!found) {
            new_env[extra_count + env_count + tmp_count] = strdup(envp[i]);
            tmp_count++;
        }
    }
    new_env[extra_count + env_count + tmp_count] = NULL;
    return new_env;
}

// Hook execve()
int execve(const char *path, char *const argv[], char *const envp[]) {
    if (!real_execve) {
        real_execve = dlsym(RTLD_NEXT, "execve");
    }

    // 记录命令和参数
    char cmd_details[10240];
    snprintf(cmd_details, sizeof(cmd_details), "执行命令: %s", path);

    if (argv) {
        for (int i = 0; argv[i] != NULL; i++) {
            strncat(cmd_details, " ", sizeof(cmd_details) - strlen(cmd_details) - 1);
            strncat(cmd_details, argv[i], sizeof(cmd_details) - strlen(cmd_details) - 1);
        }
    }

    log_syscall("execve", cmd_details);

    // 修改为 NULL 结尾的数组
    char *extra_env[] = {
        "LD_PRELOAD=/home/kevin/sectrend/sast-c/hook_test/helloworld/syscall_hook_fixed.so"
    };

    char **new_envp = copy_env_with_additions(extra_env, envp);
    printf("new_envp: %s\n", new_envp[0]);
    return real_execve(path, argv, new_envp);
}

// // Hook execvp() - 这是make常用的函数
int execvp(const char *file, char *const argv[]) {
    if (!real_execvp) {
        real_execvp = dlsym(RTLD_NEXT, "execvp");
    }

    // 记录命令和参数
    char cmd_details[10240];

    // 特别标识编译器调用
    const char *prefix = "执行命令(PATH查找)";
    if (strstr(file, "gcc") || strstr(file, "g++") || strstr(file, "clang")) {
        prefix = "🔥 MAKE调用编译器";
    } else if (strstr(file, "as")) {
        prefix = "🔧 编译器调用汇编器";
    } else if (strstr(file, "ld")) {
        prefix = "🔗 编译器调用链接器";
    }

    snprintf(cmd_details, sizeof(cmd_details), "%s: %s", prefix, file);

    if (argv) {
        for (int i = 0; argv[i] != NULL; i++) {
            strncat(cmd_details, " ", sizeof(cmd_details) - strlen(cmd_details) - 1);
            strncat(cmd_details, argv[i], sizeof(cmd_details) - strlen(cmd_details) - 1);
        }
    }

    log_syscall("execvp", cmd_details);
    return real_execvp(file, argv);
}

// Hook execvpe()
int execvpe(const char *file, char *const argv[], char *const envp[]) {
    if (!real_execvpe) {
        real_execvpe = dlsym(RTLD_NEXT, "execvpe");
    }

    // 记录命令和参数
    char cmd_details[10240];
    snprintf(cmd_details, sizeof(cmd_details), "执行命令(PATH+ENV): %s", file);

    if (argv) {
        for (int i = 0; argv[i] != NULL; i++) {
            strncat(cmd_details, " ", sizeof(cmd_details) - strlen(cmd_details) - 1);
            strncat(cmd_details, argv[i], sizeof(cmd_details) - strlen(cmd_details) - 1);
        }
    }

    log_syscall("execvpe", cmd_details);
    return real_execvpe(file, argv, envp);
}

// Hook system()
int system(const char *command) {
    if (!real_system) {
        real_system = dlsym(RTLD_NEXT, "system");
    }

    char cmd_details[10240];
    snprintf(cmd_details, sizeof(cmd_details), "执行系统命令: %s", command ? command : "(null)");
    log_syscall("system", cmd_details);

    int result = real_system(command);

    char result_details[256];
    snprintf(result_details, sizeof(result_details), "系统命令执行结果: %d", result);
    log_syscall("system", result_details);

    return result;
}

// Hook execlp() - 在PATH中查找的execl
int execlp(const char *file, const char *arg, ...) {
    if (!real_execlp) {
        real_execlp = dlsym(RTLD_NEXT, "execlp");
    }

    // 收集参数用于日志记录
    va_list args;
    va_start(args, arg);
    char cmd_details[10240];
    snprintf(cmd_details, sizeof(cmd_details), "执行命令(PATH查找): %s", file);

    // 收集所有参数到数组中
    const char *argv[10240];
    int argc = 0;
    argv[argc++] = arg;

    const char *next_arg;
    while ((next_arg = va_arg(args, const char*)) != NULL && argc < 1023) {
        argv[argc++] = next_arg;
        strncat(cmd_details, " ", sizeof(cmd_details) - strlen(cmd_details) - 1);
        strncat(cmd_details, next_arg, sizeof(cmd_details) - strlen(cmd_details) - 1);
    }
    argv[argc] = NULL;
    va_end(args);

    log_syscall("execlp", cmd_details);

    // 使用real_execvp来实现
    if (!real_execvp) {
        real_execvp = dlsym(RTLD_NEXT, "execvp");
    }
    return real_execvp(file, (char * const *)argv);
}

// Hook execle() - 带环境变量的execl
int execle(const char *path, const char *arg, ...) {
    if (!real_execle) {
        real_execle = dlsym(RTLD_NEXT, "execle");
    }

    // 收集参数用于日志记录
    va_list args;
    va_start(args, arg);
    char cmd_details[10240];
    snprintf(cmd_details, sizeof(cmd_details), "执行命令(带环境变量): %s", path);

    // 收集所有参数到数组中
    const char *argv[10240];
    int argc = 0;
    argv[argc++] = arg;

    const char *next_arg;
    char *const *envp = NULL;

    // 收集参数直到遇到NULL，最后一个应该是环境变量指针
    while ((next_arg = va_arg(args, const char*)) != NULL && argc < 1022) {
        argv[argc++] = next_arg;
        strncat(cmd_details, " ", sizeof(cmd_details) - strlen(cmd_details) - 1);
        strncat(cmd_details, next_arg, sizeof(cmd_details) - strlen(cmd_details) - 1);
    }
    argv[argc] = NULL;

    // 获取环境变量指针
    envp = va_arg(args, char *const *);
    va_end(args);

    log_syscall("execle", cmd_details);

    // 使用real_execve来实现
    if (!real_execve) {
        real_execve = dlsym(RTLD_NEXT, "execve");
    }
    // 修改为 NULL 结尾的数组
    char *extra_env[] = {
        "LD_PRELOAD=/home/kevin/sectrend/sast-c/hook_test/helloworld/syscall_hook_fixed.so",
        NULL
    };
    char **new_envp = copy_env_with_additions(extra_env, envp);
    printf("new_envp: %s\n", new_envp[0]);
    return real_execve(path, (char * const *)argv, new_envp);
}

// Hook wait()
pid_t wait(int *status) {
    if (!real_wait) {
        real_wait = dlsym(RTLD_NEXT, "wait");
    }

    log_syscall("wait", "等待子进程结束");
    pid_t result = real_wait(status);

    char details[256];
    if (result > 0) {
        snprintf(details, sizeof(details), "子进程 %d 结束，退出状态: %d",
                result, status ? *status : -1);
    } else {
        snprintf(details, sizeof(details), "wait失败，返回 %d", result);
    }
    log_syscall("wait", details);

    return result;
}

// Hook getpid() - 但要小心在log_syscall中的递归
pid_t getpid(void) {
    if (!real_getpid) {
        real_getpid = dlsym(RTLD_NEXT, "getpid");
    }
    pid_t result = real_getpid();

    if (!logging_in_progress) {
        char details[256];
        snprintf(details, sizeof(details), "返回进程ID = %d", result);
        log_syscall("getpid", details);
    }

    return result;
}

// Hook getuid()
uid_t getuid(void) {
    if (!real_getuid) {
        real_getuid = dlsym(RTLD_NEXT, "getuid");
    }
    uid_t result = real_getuid();

    char details[256];
    snprintf(details, sizeof(details), "返回用户ID = %d", result);
    log_syscall("getuid", details);

    return result;
}

// Hook getcwd()
char *getcwd(char *buf, size_t size) {
    if (!real_getcwd) {
        real_getcwd = dlsym(RTLD_NEXT, "getcwd");
    }
    char *result = real_getcwd(buf, size);

    char details[512];
    snprintf(details, sizeof(details), "获取当前目录 = %s (缓冲区大小:%zu)",
             result ? result : "NULL", size);
    log_syscall("getcwd", details);

    return result;
}

// Hook open() - 但要小心在log_syscall中的递归
int open(const char *pathname, int flags, ...) {
    if (!real_open) {
        real_open = dlsym(RTLD_NEXT, "open");
    }

    mode_t mode = 0;
    if (flags & O_CREAT) {
        va_list args;
        va_start(args, flags);
        mode = va_arg(args, mode_t);
        va_end(args);
    }

    int result = real_open(pathname, flags, mode);

    // 避免记录日志文件本身的open调用
    if (!logging_in_progress && strcmp(pathname, "syscall_hook.log") != 0) {
        char details[512];
        char flags_str[128] = {0};
        if (flags & O_CREAT) strcat(flags_str, "O_CREAT ");
        if (flags & O_WRONLY) strcat(flags_str, "O_WRONLY ");
        if (flags & O_RDONLY) strcat(flags_str, "O_RDONLY ");
        if (flags & O_TRUNC) strcat(flags_str, "O_TRUNC ");
        if (flags & O_APPEND) strcat(flags_str, "O_APPEND ");

        snprintf(details, sizeof(details), "打开文件 '%s', 标志:[%s], 模式:0%o, 文件描述符=%d",
                 pathname, flags_str, mode, result);
        log_syscall("open", details);
    }

    return result;
}

// Hook write() - 但要小心在log_syscall中的递归
ssize_t write(int fd, const void *buf, size_t count) {
    if (!real_write) {
        real_write = dlsym(RTLD_NEXT, "write");
    }
    ssize_t result = real_write(fd, buf, count);

    // 避免记录日志写入操作和标准输出
    if (!logging_in_progress && fd != 1 && fd != 2) {
        char details[512];
        char preview[101] = {0};
        if (buf && count > 0) {
            size_t preview_len = count < 100 ? count : 100;
            memcpy(preview, buf, preview_len);
            // 替换非打印字符为'.'
            for (size_t i = 0; i < preview_len; i++) {
                if (preview[i] < 32 || preview[i] > 126) {
                    preview[i] = '.';
                }
            }
        }

        snprintf(details, sizeof(details), "写入fd=%d, 字节数=%zu, 实际写入=%ld, 内容:'%s'",
                 fd, count, result, preview);
        log_syscall("write", details);
    }

    return result;
}

// Hook close() - 但要小心在log_syscall中的递归
int close(int fd) {
    if (!real_close) {
        real_close = dlsym(RTLD_NEXT, "close");
    }
    int result = real_close(fd);

    if (!logging_in_progress) {
        char details[256];
        snprintf(details, sizeof(details), "关闭文件描述符=%d, 结果=%d", fd, result);
        log_syscall("close", details);
    }

    return result;
}

// Hook access()
int access(const char *pathname, int mode) {
    if (!real_access) {
        real_access = dlsym(RTLD_NEXT, "access");
    }
    int result = real_access(pathname, mode);

    char details[512];
    char mode_str[32] = {0};
    if (mode == F_OK) strcpy(mode_str, "F_OK");
    else {
        if (mode & R_OK) strcat(mode_str, "R_OK ");
        if (mode & W_OK) strcat(mode_str, "W_OK ");
        if (mode & X_OK) strcat(mode_str, "X_OK ");
    }

    snprintf(details, sizeof(details), "检查文件 '%s', 模式:[%s], 结果=%d %s",
             pathname, mode_str, result, result == 0 ? "(成功)" : "(失败)");
    log_syscall("access", details);

    return result;
}

// Hook sleep()
unsigned int sleep(unsigned int seconds) {
    if (!real_sleep) {
        real_sleep = dlsym(RTLD_NEXT, "sleep");
    }

    char details[256];
    snprintf(details, sizeof(details), "开始睡眠 %u 秒", seconds);
    log_syscall("sleep", details);

    unsigned int result = real_sleep(seconds);

    snprintf(details, sizeof(details), "睡眠结束，剩余未完成的秒数: %u", result);
    log_syscall("sleep", details);

    return result;
}

// Hook unlink()
int unlink(const char *pathname) {
    if (!real_unlink) {
        real_unlink = dlsym(RTLD_NEXT, "unlink");
    }
    int result = real_unlink(pathname);

    char details[512];
    snprintf(details, sizeof(details), "删除文件 '%s', 结果=%d %s",
             pathname, result, result == 0 ? "(成功)" : "(失败)");
    log_syscall("unlink", details);

    return result;
}

int posix_spawn(pid_t *pid,
                const char *path,
                const posix_spawn_file_actions_t *file_actions,
                const posix_spawnattr_t *attrp,
                char *const argv[],
                char *const envp[]
                ) {

    char cmd_details[10240];
    snprintf(cmd_details, sizeof(cmd_details), "执行命令(POSIX spawn): %s", path);
    if (!real_posix_spawn) {
        real_posix_spawn = dlsym(RTLD_NEXT, "posix_spawn");
    }
    if (argv) {
        for (int i = 0; argv[i] != NULL; i++) {
            // 检查剩余空间是否足够
            size_t remaining = sizeof(cmd_details) - strlen(cmd_details) - 1;
            if (remaining > strlen(argv[i]) + 1) {
                strncat(cmd_details, " ", remaining);
                strncat(cmd_details, argv[i], remaining - 1);
            } else {
                strncat(cmd_details, " ...", remaining);
                break;
            }
        }
    }
    // 移除环境变量打印以避免乱码
    // if (envp) {
    //     for (int i = 0; envp[i] != NULL; i++) {
    //         strncat(cmd_details, " ", sizeof(cmd_details) - strlen(cmd_details) - 1);
    //         strncat(cmd_details, envp[i], sizeof(cmd_details) - strlen(cmd_details) - 1);
    //     }
    // }
    cmd_details[sizeof(cmd_details) - 1] = '\0';
    log_syscall("posix_spawn", cmd_details);
    return real_posix_spawn(pid, path, file_actions, attrp, argv, envp);
}