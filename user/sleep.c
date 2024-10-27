#include "kernel/types.h"
#include "user/user.h"

/**
argc: 表示命令行参数的数量。
argv[]: 表示具体的命令行参数，其中 argv[0] 是程序的名称，argv[1] 是用户传递的第一个参数，也就是要睡眠的时间。
 */
int main(int argc, char const *argv[])
{
  if (argc != 2) { //检查是否正确地传递了两个参数（程序名和睡眠时间）
    fprintf(2, "usage: sleep <time>\n");//标准错误流
    exit(1);
  }
  sleep(atoi(argv[1]));//atoi(argv[1]) 将传入的字符串参数转换为整数
  exit(0);
}
