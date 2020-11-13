#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"
#define MAXARGS 10
#define MAXWORD 30
void execPipe(int argc, char* argv[]);
char whitespace[] = " \t\r\n\v";
char args[MAXARGS][MAXWORD];

void
panic(char *s)
{
  fprintf(2, "%s\n", s);
  exit(-1);
}
/* 创建shell进程副本 */
int
fork1(void)
{
  int pid;

  pid = fork();
  if(pid == -1)
    panic("fork");
  return pid;
}

void 
setargs(char* cmd, int* argc, char* argv[])
{
  // 让argv的每一个元素都指向args的每一行
  for(int i = 0; i < MAXARGS; i++){
    argv[i] = &args[i][0];
  }
  int i = 0, j = 0;
  for(; cmd[j] != '\n' && cmd[j] != '\0'; j++){
    // 跳过命令前的空格
    while(strchr(whitespace, cmd[j]))
      j++;
    // argv[i]指向本次循环word的头
    argv[i++] = cmd + j;
    // 寻找下一个空格，改为'\0'作为结束符
    while(strchr(whitespace, cmd[j]) == 0)
      j++;
    cmd[j] = '\0';
  }
  argv[i] = 0;
  *argc = i;
}

void 
runcmd(int argc, char* argv[])
{
  // 如果遇到'|'，则为pipe，进入execPipe进行迭代
  for(int i = 1; i < argc;i++){
    if(!strcmp(argv[i], "|"))
      execPipe(argc, argv);
  }
  // 判断是否为重定向
  for(int i = 1; i < argc; i++){
    // 输出重定向
    if(!strcmp(argv[i], ">")){
      close(1);
      if(open(argv[i + 1], O_CREATE|O_WRONLY) < 0){
        fprintf(2, "open %s failed\n", argv[i + 1]);
        exit(-1);
      }
      argv[i] = 0;
    }
    // 输入重定向
    if(!strcmp(argv[i], "<")){
      close(0);
      if(open(argv[i + 1], O_RDONLY) < 0){
        fprintf(2, "open %s failed\n", argv[i + 1]);
        exit(-1);
      }
      argv[i] = 0;
    }
  }
  // 调用exec加载函数
  if(argv[0] == 0)
      exit(-1);
  exec(argv[0], argv);
  fprintf(2, "exec %s failed\n", argv[0]);

  exit(0);
}

void 
execPipe(int argc, char* argv[])
{
  int i = 0;
  // 确定为重定向，故寻找'|'，然后换成'\0'
  for(; i < argc; i++){
    if(!strcmp(argv[i], "|")){
        argv[i] = 0;
        break;
    }
  }
  // 仿照sh.c的PIPE
  int p[2];
  if(pipe(p) < 0)
      panic("pipe");
    if(fork1() == 0){
      close(1);
      dup(p[1]);
      close(p[0]);
      close(p[1]);
      runcmd(i, argv);
    }
    if(fork1() == 0){
      close(0);
      dup(p[0]);
      close(p[0]);
      close(p[1]);
      runcmd(argc - i - 1, argv + i + 1);
    }
    close(p[0]);
    close(p[1]);
    wait(0);
    wait(0);

    exit(0);
}

int
getcmd(char *buf, int nbuf)
{
  fprintf(2, "@ ");
  memset(buf, 0, nbuf);
  gets(buf, nbuf);
  if(buf[0] == 0) // EOF
    return -1;
  return 0;
}

int
main(void)
{
  static char buf[100];

  // Read and run input commands.
  while(getcmd(buf, sizeof(buf)) >= 0){
    if(buf[0] == 'c' && buf[1] == 'd' && buf[2] == ' '){
      // Chdir must be called by the parent, not the child.
      buf[strlen(buf)-1] = 0;  // chop \n
      if(chdir(buf+3) < 0)
        fprintf(2, "cannot cd %s\n", buf+3);
      continue;
    }
    if(fork1() == 0){
      int argc = -1;
      char* argv[MAXARGS];
      setargs(buf, &argc, argv);
      runcmd(argc, argv);
    }
    wait(0);
  }
  exit(0);
}
