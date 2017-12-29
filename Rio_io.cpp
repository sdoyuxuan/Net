#include"Rio_io.h"

rio_t::rio_t(int fd)
    :rio_fd(fd),rio_cnt(0),rio_buf(""),rio_bufptr(rio_buf)
	{}
rio_t::rio_t(const rio_t & rio_obj)
   :rio_fd(rio_obj.rio_fd),rio_cnt(rio_obj.rio_cnt)
   {
	    memmove(rio_buf,rio_obj.rio_buf,RIO_BUFSIZE);
		rio_bufptr = rio_buf+rio_cnt;
   }
/*
rio_t& rio_t::operator=(const rio_t & rio_obj)
{
      rio_fd = rio_obj.rio_fd;
	  rio_cnt = rio_obj.rio_cnt;
	  memmove(rio_buf,rio_obj.rio_buf,RIO_BUFSIZE);
	  rio_bufptr = rio_buf+rio_cnt;
}
*/	   

ssize_t rio_readn(int fd , void * usrbuf,size_t n)
{
	 size_t require = n;
	 ssize_t readn = 0;
	 char * buf =(char*) usrbuf;
	 while(require>0)
	{
        if((readn = read(fd,buf,require))<0)
		{
			if(errno == EINTR)
				continue;
			else
			{
		        perror("rio_read");
				return -1;
			}
		}
		else if(readn == 0)
		{
			break;
		}
		require-=readn;
		buf+=readn;
	}
	return n - readn; // 因为到这里的执行流可能是遇到了EOF，所以不一定需求多少就读多少
}

ssize_t rio_writen(int fd , void * usrbuf,size_t n)
{
		size_t require = n;
        size_t writen=0;
		char * buf = (char*)usrbuf;
		while(require>0)
		{
			if((writen = write(fd,buf,require)<0))
			{
				if(errno == EINTR)
					continue;
				else
					perror("rio_write");
					return -1;
			}
			require -= writen;
			buf +=writen;
		}
		return n;
}

static ssize_t rio_read(rio_t * rp , char * usrbuf , size_t n) //static 局部绑定只在当前源文件有效，这个跟链接有关系哦
{
        int cnt = 0;
		while(rp->rio_cnt<=0)
	    {
            rp->rio_cnt = read(rp->rio_fd,rp->rio_buf,RIO_BUFSIZE);
			if(rp->rio_cnt == -1 )
			{
				if(errno !=EINTR)
					perror("rio_read");
				return -1;
			}
			else if(rp->rio_cnt == 0 )
			{
                 printf("In rio_read EOF\n");
				 return 0;
			}
		}
		if(n==0)
        {
	       *usrbuf = *rp->rio_bufptr;
		   return 1;
		} 
		cnt = n;
		if(rp->rio_cnt < n)
			cnt = rp->rio_cnt;
		memmove(rp->rio_buf,usrbuf,cnt);
		rp->rio_bufptr +=cnt;
		rp->rio_cnt -=cnt;
        return cnt;
}

ssize_t rio_readlineb(rio_t * rp,void * usrbuf,size_t maxlen)
{
		int n , rc;
		char c , * buf = (char*)usrbuf;
		for(n = 1 ; n < maxlen ;n++)
		{
			if((rc = rio_read(rp,&c,1)==1))
			{
				*buf++=c;
				if(c=='\r')
				{
					n++;
					if((rc=rio_read(rp,&c,0))>0&&c != '\n')
					 {
						break;
					 }
					 else if(c == '\n')
					 {
	//n++; 这里把/r/n 的情况合并为同一个字符 \n 故n就不加加了。
						rio_read(rp,buf-1,1);
						break;
					 }
					 else
					 {
						if(rc == 0)
						  break;
						return rc; // 处理 -1 的情况
					 }
				}
				if(c=='\n')
				{
                   n++;
				   break;
				}
		   }
			else if(rc == 0 )
			{
				break;
			}
			else 
			    return -1;
		}
		*buf = 0;
		return n-1;
}


