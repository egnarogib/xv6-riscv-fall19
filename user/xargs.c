#include "kernel/types.h"
#include "user/user.h"
#include "kernel/param.h"

int main(int argc, char *argv[]) {
    char buf[MAXARG], blk[MAXARG], *p = buf, *cmd[MAXARG];
    int i, j = 0, k, l, m = 0;
    // 记录在 cmd 当中
    for(i = 1; i < argc; i++) {
        cmd[j++] = argv[i];
    }
    // 如果 read 的长度小于0（ctrl+d），则结束循环退出程序
    while((k = read(0, blk, sizeof(blk))) > 0) {
        for(l = 0; l < k; l++) {
            // 如果读到' '，切词，参数存储
            // 如果读到'\n'，表示本行读取结束，对子进程执行 exec()
            // 其它则存入 buf
            if(blk[l] == ' ') {
                buf[m++] = 0;
                cmd[j++] = p;
                p = &buf[m];
            } else if(blk[l] == '\n') {
                buf[m] = 0; m = 0;
                cmd[j++] = p;
                p = buf;
                cmd[j] = 0; j = argc - 1;
                if(fork() == 0) {
                    exec(argv[1], cmd);
                }
            } else {
                buf[m++] = blk[l];
            }
        }
    }
    exit();
}