#include "kernel/types.h"
#include "user/user.h"

int main() {
    int parent_fd[2], child_fd[2];
    char buffer[32];
    long length = sizeof(buffer);
    // 创建父进程、子进程
    pipe(parent_fd);
    pipe(child_fd);
    // 子进程
    if (fork() == 0) {
        // 关闭无用的父进程写，子进程读
        close(parent_fd[1]);
        close(child_fd[0]);
        // 子进程从parent读取字符ping
        if (read(parent_fd[0], buffer, length) != length) {
            printf("error: prt ---> cld read error!\n");
            exit();
        }
        printf("%d: received ping\n", getpid());
        // 子进程向child写入字符
        if (write(child_fd[1], buffer, length) != length) {
            printf("error: cld ---> prt write error!\n");
            exit();
        }
        exit();
    }
    // 父进程
    // 关闭无用的父进程读，子进程写
    close(parent_fd[0]);
    close(child_fd[1]);
    // 父进程向parent写入字符
    if (write(parent_fd[1], buffer, length) != length) {
        printf("error: prt ---> cld write error!\n");
        exit();
    }
    // 父进程从child读取字符pong
    if (read(child_fd[0], buffer, length) != length) {
        printf("error: cld ---> prt read error!\n");
        exit();
    }
    printf("%d: received pong\n", getpid());
    // 等待子进程退出
    wait();
    exit();
}