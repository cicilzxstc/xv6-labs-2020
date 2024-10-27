/**
 *使用管道编写prime sieve(筛选素数)的并发版本。这个想法是由Unix管道的发明者Doug McIlroy提出的。请查看这个网站(翻译在下面)，该网页中间的图片和周围的文字解释了如何做到这一点。您的解决方案应该在user/primes.c文件中。
 *
 *您的目标是使用pipe和fork来设置管道。第一个进程将数字2到35输入管道。对于每个素数，您将安排创建一个进程，该进程通过一个管道从其左邻居读取数据，并通过另一个管道向其右邻居写入数据。由于xv6的文件描述符和进程数量有限，因此第一个进程可以在35处停止。
 *
 *提示：
 *
 *请仔细关闭进程不需要的文件描述符，否则您的程序将在第一个进程达到35之前就会导致xv6系统资源不足。
 *
 *一旦第一个进程达到35，它应该使用wait等待整个管道终止，包括所有子孙进程等等。因此，主primes进程应该只在打印完所有输出之后，并且在所有其他primes进程退出之后退出。
 *
 *提示：当管道的write端关闭时，read返回零。
 *
 *最简单的方法是直接将32位（4字节）int写入管道，而不是使用格式化的ASCII I/O。
 *
 *您应该仅在需要时在管线中创建进程。
 *
 *将程序添加到Makefile中的UPROGS
 */
#include "kernel/types.h"
#include "user/user.h"

//代表管道的读取端和写入端
#define RD 0
#define WR 1

//定义常量 INT_LEN 为 int 类型的长度，用于表示整数的字节大小。
const uint INT_LEN = sizeof(int);

/**
 * @brief 从左邻居的管道中读取第一个整数数据。
 * @param int lpipe[2]表示一个含有两个整数的管道数组，其中lpipe[0]等同于lpipe[RD]，表示管道的读端。
 * @param dst用于存储读取的整数
 * @return 如果读取成功，返回 0，并输出该素数。如果没有读取到数据，返回 -1
 */
int lpipe_first_data(int lpipe[2], int *dst)
{
  //如果读取第一个整数数据成功，打印出来
  if (read(lpipe[RD], dst, sizeof(int)) == sizeof(int)) {
    printf("prime %d\n", *dst);
    return 0;
  }
  return -1;
}

/**
 * @brief 从左邻居读取数据，将不能被 first 整除的数写入右邻居的管道中
 * @param int lpipe[2] 左邻居的管道符
 * @param int rpipe[2] 右邻居的管道符
 * @param first 左邻居的第一个数据
 */
void transmit_data(int lpipe[2], int rpipe[2], int first)
{
  int data;
  // 从左管道读取数据
  while (read(lpipe[RD], &data, sizeof(int)) == sizeof(int)) {
    // 将无法整除的数据传递入右管道
    if (data % first)
      write(rpipe[WR], &data, sizeof(int));
  }
  close(lpipe[RD]);
  close(rpipe[WR]);
}

/**
 * @brief 寻找素数
 * @param int lpipe[2] 左邻居管道
 */
void primes(int lpipe[2])
{
  //关闭写端 lpipe[WR]，因为该进程只从管道中读取数据
  close(lpipe[WR]);
  //调用 lpipe_first_data() 获取左邻居的第一个素数
  int first;
  if (lpipe_first_data(lpipe, &first) == 0) {
    //创建新的管道 p，并调用 transmit_data() 将数据传递到右邻居
    int p[2];
    pipe(p); // 当前的管道
    transmit_data(lpipe, p, first);

    //通过 fork() 创建新进程，并递归调用 primes()，每个进程都处理一个素数
    if (fork() == 0) {
      primes(p);    // 递归的思想，但这将在一个新的进程中调用
    } else {
      close(p[RD]);
      wait(0);
    }
  }
  exit(0);
}

int main(int argc, char const *argv[])
{
  int p[2];
  pipe(p);

  for (int i = 2; i <= 35; ++i) //写入初始数据
    write(p[WR], &i, INT_LEN);

  if (fork() == 0) {
    primes(p);
  } else {
    close(p[WR]);
    close(p[RD]);
    wait(0);
  }

  exit(0);
}
