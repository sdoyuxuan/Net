#include <sys/socket.h>
#include "Rio_io.h"
#include <assert.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <vector>
#include <algorithm>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#define SA  struct sockaddr
#define BACKLOG 10
#define SIZE 4096
using namespace std;
int startup(char * ip ,char * port)
{
		assert(ip&&port);
		int listensock = 0;
		if((listensock = socket(AF_INET,SOCK_STREAM,0))<0)
		{
						perror("socket");
						exit(1);
		}
		struct sockaddr_in address;
		bzero(&address,sizeof(address));
		if(inet_pton(AF_INET,ip,&address.sin_addr)<0)
		{
				perror("inet_pton");
				exit(2);
		}
	    address.sin_port = htons(atoi(port));
		if(bind(listensock,(SA*)&address,sizeof(address))<0)
		{
	      	perror("bind");
	   		exit(3);
    	} 
		if((listen(listensock,BACKLOG))<0) 
		{
			perror("listen");
			exit(4);
		}
		return listensock;
}
void usage()
{
	printf("please Enter ip port \n");
}
void request_html(rio_t & IoObject , const char * param)
{
      const char head[]= "HTTP1.0 200 OK\r\n";
	  const char response_head[] = "Content-Type: text/html\r\n\r\n";
	  send(IoObject.rio_fd,head,sizeof(head),0);
	  send(IoObject.rio_fd,response_head,sizeof(response_head),0);
#ifdef DEBUG
	  char path[SIZE/10] = "./wwwroot";
	  strcat(path,param);
	  int filefd = open(path,0664);
#else
	  int filefd = open("./wwwroot/html",0664);
#endif
	  if(filefd < 0 )
	  {
	    perror("open");
		pthread_exit((void*)8);
	  }
	  struct stat StatBuf;
	  if(fstat(filefd,&StatBuf)<0)
	  {
		 perror("fstat");
		 pthread_exit((void*)9);
	  }
	  if(S_ISDIR(StatBuf.st_mode))
	  {
		 close(filefd);
		 filefd = open("./wwwroot/html",0664);
	  }
	  sendfile(IoObject.rio_fd,filefd,NULL,StatBuf.st_size);
}
/* 静态写完毕，还有动态未完成*/
void execgi(rio_t & IoObject)
{

}
void request_head(vector<char> & buff,rio_t & IoObject)
{
	  vector<char>::iterator it = find_first_of(buff,' ');
	  if(it == buff.end())
	  {
	     	perror("request_head");
	    	pthread_exit((void*)6);
	  }
	  *it = 0;
	  const char * param = NULL;
	  for(size_t i = 0 ; i < SIZE ; i++)
	  {
		 if(buff[i] == 0)
		   {
			  param = &buff[i] + 1;
			  break;
		   }
	  }
	  vector<char>::iterator end = buff.end();
      it = find_first_of(buff,' ');
	  if(it == buff.end())
	  {
	     	perror("request_head");
	    	pthread_exit((void*)6);
	  }
	  *it = 0;
	  if(strcasecmp("GET",&buff[0])==0)
	   {
           request_html(IoObject,param); 	        
	   }
	   else if(strcasecmp("POST",&buff[0])==0)
	   {
	       execgi(IoObject);
	   }
	   else 
	   {
		   printf("cause other , but now not support");
		   request_err();
	   }
}
void* doit(void * clientsock)
{
	  std::vector<char> buff(SIZE,0);
	  int sz = 0;
	  int pos = 0;
	  rio_t IoObject((int)clientsock);
	  while(1)
	  {
	   if((sz=rio_readlineb(IoObject,&(buff[0]+pos),SIZE-1))>0)
		{
			if(buff[pos+sz]=='\n')
				break;
			else 
			{
					buff.resize(2*SIZE);
					pos +=sz;
			}
		}
        else 
		{
			pthread_exit((void*)5);//无请求行则终止线程
		}
	 }
	 /*请求行读取完毕*/
	 /*处理请求行逻辑开始*/
     request_head(buff,IoObject);
}
		


int main(int argc ,char * argv[])
{
		if(argc < 3 )
		{
			usage();
		    return 1;
		}
		int listensock = startup(argv[1],argv[2]);
		while(1)
		{
            struct sockaddr_in peer;
			pthread_t tid;
			char ip[INET_ADDRSTRLEN] = {0};
			socklen_t len;
		    int	clientsock = accept(listensock,(SA*)&peer,&len);
			printf("client ip : %s port : %d",inet_ntop(AF_INET,&peer.sin_addr,ip,INET_ADDRSTRLEN),ntohs(peer.sin_port));
			if(pthread_create(&tid,NULL,doit,(void*)clientsock)<0)
				{
					perror("pthread_create");
					exit(5);
				}
			pthread_detach(tid);//不管怎样必须回收要不有僵尸进程类似的东西
        }
		return 0;
}


