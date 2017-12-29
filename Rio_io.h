#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#define RIO_BUFSIZE 8192
struct rio_t
{
rio_t(int fd);
rio_t(const rio_t & rio_obj);
rio_t& operator= (const rio_t & rio_obj)=delete;
const int rio_fd;
int rio_cnt;//1.从read函数中读到缓冲区中的数据还有多少没有被用户读走，即缓冲区还有多少未读数据。
char rio_buf[RIO_BUFSIZE];//缓冲区，指通过read函数从Linux内核中读取到的数据。
char * rio_bufptr;//指向当前未处理的数据。
};//对write/read 做了层封装 ，这个东西相当于标准i/o的FILE结构体

ssize_t rio_readn(int fd , void * usrbuf,size_t n);
ssize_t rio_write(int fd , void * usrbuf,size_t n);
ssize_t rio_readlineb(rio_t * rp,void * usrbuf,size_t maxlen);

