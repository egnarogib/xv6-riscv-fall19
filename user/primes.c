#include "kernel/types.h"
#include "user/user.h"

void findPrimes(int *list, int numOfList) {
    if (numOfList == 1) {
        printf("prime %d\n", *list);
        return;
    }
    int p[2],i;
    int prime = *list;
    int temp;
    printf("prime %d\n", prime);
    pipe(p);
    if (fork() == 0) {
        // 将剩下的数写入到子进程当中
        for (i = 0; i < numOfList; i++) {
            temp = *(list + i);
            write(p[1], (char *)(&temp), 4);
        }
        exit();
    }
    // 写完毕，关闭写入端
    close(p[1]);
    if (fork() == 0) {
        int count = 0;
        char buffer[4];
        // 寻找不能整除当前质数的数，记录并作为迭代的list
        // 通过此方法即可找到全部质数
        while (read(p[0], buffer, 4) != 0) {
            temp = *((int *)buffer);
            if (temp % prime != 0) {
                *list = temp;
                list += 1;
                count++;
            }
        }
        findPrimes(list - count, count);
        exit();
    }
    wait();
    wait();
}

int main() {
    int list[34];
    int i;
    for (i = 0; i < 34; i++) {
        list[i] = i + 2;
    }
    findPrimes(list, 34);
    exit();
}