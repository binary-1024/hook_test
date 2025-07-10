#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

extern char **environ;

int main() {
    pid_t pid;
    char *argv[] = {"/bin/ls", "-l", NULL};

    if (posix_spawn(&pid, "/bin/ls", NULL, NULL, argv, environ) != 0) {
        perror("posix_spawn failed");
        return 1;
    }

    // 等待子进程结束
    waitpid(pid, NULL, 0);
    return 0;
}
