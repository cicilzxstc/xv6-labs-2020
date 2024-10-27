/**
 *写一个简化版本的UNIX的find程序：查找目录树中具有特定名称的所有文件，你的解决方案应该放在user/find.c
 *
 *提示：
 *
 *查看user/ls.c文件学习如何读取目录
 *使用递归允许find下降到子目录中
 *不要在“.”和“..”目录中递归
 *对文件系统的更改会在qemu的运行过程中一直保持；要获得一个干净的文件系统，请运行make clean，然后make qemu
 *你将会使用到C语言的字符串，要学习它请看《C程序设计语言》（K&R）,例如第5.5节
 *注意在C语言中不能像python一样使用“==”对字符串进行比较，而应当使用strcmp()
 *将程序加入到Makefile的UPROGS
 */

 /*
 *types.h：定义基本的类型。
 *fs.h：定义文件系统相关的数据结构，如 struct dirent（目录项）。
 *stat.h：定义文件状态的相关结构 struct stat。
 *user.h：用户层相关的库函数和系统调用。
 */
#include "kernel/types.h"
#include "kernel/fs.h"
#include "kernel/stat.h"
#include "user/user.h"

//递归搜索目录 path 下的文件，找到与 filename 匹配的文件。
void find(char *path, const char *filename)
{
  //定义缓冲区 buf 用于存储路径，指针 p 用于在路径字符串中操作。
  char buf[512], *p;
  //fd 是目录的文件描述符
  int fd;
  //struct dirent de 用于存储目录中的文件/目录项
  struct dirent de;
  //struct stat st 用于存储文件状态信息（如类型、大小、权限等）
  struct stat st;

  //尝试打开目录路径 path，如果失败（fd < 0），输出错误信息并返回
  if ((fd = open(path, 0)) < 0) {
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  //使用 fstat 获取文件描述符 fd 所指向文件或目录的状态信息，如果失败，输出错误信息并关闭文件描述符
  if (fstat(fd, &st) < 0) {
    fprintf(2, "find: cannot fstat %s\n", path);
    close(fd);
    return;
  }

  //检查传入的路径是否为目录。如果不是目录，输出用法错误，并返回。因为 find 只能在目录中查找。
  if (st.type != T_DIR) {
    fprintf(2, "usage: find <DIRECTORY> <filename>\n");
    return;
  }

  //检查路径长度是否过长，避免缓冲区溢出。如果路径长度过长，输出错误并返回
  if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
    fprintf(2, "find: path too long\n");
    return;
  }
  //将路径 path 复制到缓冲区 buf 中，并在路径末尾添加斜杠 /，以便后续拼接子文件/子目录名
  strcpy(buf, path);
  p = buf + strlen(buf);
  *p++ = '/'; //p指向最后一个'/'之后

  //通过 read 循环读取目录项（struct dirent），如果目录项的 inum 为 0，表示该项无效，跳过该项。
  while (read(fd, &de, sizeof de) == sizeof de) {
    if (de.inum == 0)
      continue;

    memmove(p, de.name, DIRSIZ); //将目录项的名称复制到 buf 路径的末尾，并
    p[DIRSIZ] = 0;               //确保字符串以 \0 结尾,字符串结束标志
    
    if (stat(buf, &st) < 0) {//使用 stat 获取拼接后的完整路径的文件状态，如果失败，输出错误并跳过
      fprintf(2, "find: cannot stat %s\n", buf);
      continue;
    }
    //不要在“.”和“..”目录中递归即 如果该项是目录，并且不是当前目录 . 或父目录 ..，则递归调用 find 函数继续在该目录中查找
    if (st.type == T_DIR && strcmp(p, ".") != 0 && strcmp(p, "..") != 0) {
      find(buf, filename);
    } else if (strcmp(filename, p) == 0)
      printf("%s\n", buf);
  }

  close(fd);
}

int main(int argc, char *argv[])
{
  //检查参数数量，如果参数数量不符合要求，输出用法提示并退出
  if (argc != 3) {
    fprintf(2, "usage: find <directory> <filename>\n");
    exit(1);
  }
  find(argv[1], argv[2]);
  exit(0);
}
