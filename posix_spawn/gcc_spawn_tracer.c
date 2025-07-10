/* gcc_spawn_tracer.c
 * A dynamic library that hooks posix_spawn and execve to trace gcc internal stages
 * Compile with: gcc -shared -fPIC -o gcc_spawn_tracer.so gcc_spawn_tracer.c -ldl
 * Use with: LD_PRELOAD=./gcc_spawn_tracer.so your_build_command
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include <spawn.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdarg.h>
#include <time.h>

extern char **environ;

static int (*real_posix_spawn)( pid_t *pid, 
                                const char *path,
                                const posix_spawn_file_actions_t *file_actions,
                                const posix_spawnattr_t *attrp,
                                char *const argv[], char *const envp[]
                            ) = NULL;

static int (*real_execve)(const char *pathname, char *const argv[], char *const envp[]) = NULL;

static void log_spawn(const char *label, const char *path, char *const argv[]) {
    FILE *logf = fopen("/tmp/gcc_trace.log", "a");
    if (!logf) return;

    time_t now = time(NULL);
    fprintf(logf, "[%ld] [%s] Executing: %s\n", now, label, path);

    for (int i = 0; argv && argv[i]; i++) {
        fprintf(logf, "  argv[%d] = %s\n", i, argv[i]);
    }
    fprintf(logf, "\n");
    fclose(logf);
}

int posix_spawn(pid_t *pid, 
                const char *path,
                const posix_spawn_file_actions_t *file_actions,
                const posix_spawnattr_t *attrp,
                char *const argv[], char *const envp[]
                ) {
    if (!real_posix_spawn) {
        real_posix_spawn = dlsym(RTLD_NEXT, "posix_spawn");
    }
    log_spawn("posix_spawn", path, argv);
    return real_posix_spawn(pid, path, file_actions, attrp, argv, envp);
}

int execve(const char *pathname, char *const argv[], char *const envp[]) {
    if (!real_execve) {
        real_execve = dlsym(RTLD_NEXT, "execve");
    }
    log_spawn("execve", pathname, argv);
    return real_execve(pathname, argv, envp);
}


/* hook gcc 编译器： LD_PRELOAD=./gcc_spawn_tracer.so gcc -v -O2 -c posix_spawn_test.c > gcc_spawn_tracer.log 2>&1 */
/* hook make 构建器： LD_PRELOAD=./gcc_spawn_tracer.so make */

