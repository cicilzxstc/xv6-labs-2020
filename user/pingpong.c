/**
编写一个使用UNIX系统调用的程序来在两个进程之间“ping-pong”一个字节，请使用两个管道，每个方向一个。父进程应该向子进程发送一个字节;子进程应该打印“<pid>: received ping”，其中<pid>是进程ID，并在管道中写入字节发送给父进程，然后退出;父级应该从读取从子进程而来的字节，打印“<pid>: received pong”，然后退出。您的解决方案应该在文件user/pingpong.c中。

提示：
使用pipe来创造管道

使用fork创建子进程

使用read从管道中读取数据，并且使用write向管道中写入数据

使用getpid获取调用进程的pid

将程序加入到Makefile的UPROGS

xv6上的用户程序有一组有限的可用库函数。您可以在user/user.h中看到可调用的程序列表；源代码（系统调用除外）位于user/ulib.c、user/printf.c和user/umalloc.c中。
 */
#include "kernel/types.h"
#include "user/user.h"

//定义了常量 RD 和 WR，分别代表管道的读取端和写入端。管道的索引值 0 用于读取端，1 用于写入端
#define RD 0
#define WR 1

int main(int argc, char const *argv[]) {
    char buf = 'P'; //用于传送的字节

    /**
        两个整型数组 fd_c2p 和 fd_p2c，每个数组包含两个文件描述符，用于表示管道的读端和写端。
        d_c2p 是子进程到父进程的数据传输管道，fd_p2c 是父进程到子进程的管道
    */
    int fd_c2p[2]; //子进程->父进程
    int fd_p2c[2]; //父进程->子进程
    //pipe 函数创建一个管道，用于进程间的通信，并在数组中存储管道的两个文件描述符。
    pipe(fd_c2p);
    pipe(fd_p2c);

    //fork() 调用创建一个新进程(子进程)。pid 保存 fork() 返回的值。
    int pid = fork();
    int exit_status = 0;

    //如果 pid < 0，表示子进程创建失败
    if (pid < 0) {
        fprintf(2, "fork() error!\n");
        //关闭所有打开的管道端口，以释放资源
        close(fd_c2p[RD]);
        close(fd_c2p[WR]);
        close(fd_p2c[RD]);
        close(fd_p2c[WR]);
        exit(1);
    } else if (pid == 0) {//如果 pid == 0，表示子进程创建成功，且当前是子进程
        //关闭子进程中不需要的管道端。子进程只会从 fd_p2c 读取数据并向 fd_c2p 写入数据，因此关闭 fd_p2c[WR] 和 fd_c2p[RD]
        close(fd_p2c[WR]);
        close(fd_c2p[RD]);

        //从管道 fd_p2c[RD] 读取一个字节，并将其存储到 buf 中。如果读取失败或读取的字节数不正确，代码进入错误处理。
        if (read(fd_p2c[RD], &buf, sizeof(char)) != sizeof(char)) {
            fprintf(2, "child read() error!\n");
            exit_status = 1; //标记出错
        } else {
            //如果读取成功，输出 "received ping" 消息，getpid() 获取当前进程的 ID。
            fprintf(1, "%d: received ping\n", getpid());
        }

        //将 buf 中的数据写入到管道 fd_c2p[WR]，即将数据传回父进程。如果写入失败，进入错误处理
        if (write(fd_c2p[WR], &buf, sizeof(char)) != sizeof(char)) {
            fprintf(2, "child write() error!\n");
            exit_status = 1;
        }

        //子进程完成工作后，关闭剩下的端口，释放资源
        close(fd_p2c[RD]);
        close(fd_c2p[WR]);

        exit(exit_status);
    } else { //如果 pid > 0，表示当前仍然是父进程
        //关闭父进程中不需要的管道端。父进程只会向 fd_p2c 写入数据，并从 fd_c2p 读取数据，因此关闭 fd_p2c[RD] 和 fd_c2p[WR]
        close(fd_p2c[RD]);
        close(fd_c2p[WR]);

        //将 buf 中的数据写入到 fd_p2c[WR] 管道，传递给子进程。如果写入失败，进入错误处理
        if (write(fd_p2c[WR], &buf, sizeof(char)) != sizeof(char)) {
            fprintf(2, "parent write() error!\n");
            exit_status = 1;
        }

        //从 fd_c2p[RD] 管道读取子进程返回的数据。如果读取失败，进入错误处理
        if (read(fd_c2p[RD], &buf, sizeof(char)) != sizeof(char)) {
            fprintf(2, "parent read() error!\n");
            exit_status = 1; //标记出错
        } else {
            //如果读取成功，输出 "received pong" 消息，getpid() 获取当前进程 ID。
            fprintf(1, "%d: received pong\n", getpid());
        }

        //父进程完成工作后，关闭剩下的端口，释放资源
        close(fd_p2c[WR]);
        close(fd_c2p[RD]);

        exit(exit_status);
    }
}
