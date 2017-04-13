#include "iostream"
#include "netdb.h"
#include "stdio.h"
#include "stdlib.h"
#include "sys/socket.h"
#include "unistd.h"
#include "arpa/inet.h"
#include "string.h"
#include "memory.h"
#include "signal.h"
#include "time.h"
#include "msg.h"
#include "errno.h"
#include <pthread.h>

int sockfd;

void sig_handler(int signo)
{
    if(signo==SIGINT)
    {
        printf("Server close \n");
        close(sockfd);
        exit(1);
    }
}

void do_service(int fd)
{
    /*和客户端进行读写操作，双向通信*/
    char buff[512];
    while(1)
    {
        memset(buff,0,sizeof(buff));
        size_t size;
        if((size=read_msg(fd,
                    buff,sizeof(buff)))<0)
        {
            perror("protocal error");
            break;
        }else if(size==0)
        {
            break;
        }else
        {
            printf("%s\n",buff);
            if(write_msg(fd,buff,sizeof(buff))<0)
            {
                if(errno==EPIPE)
                    break;
                perror("protacal error");
            }
        }
                
    }
}

void out_fd(int fd)
{
    struct sockaddr_in addr;
    socklen_t len=sizeof(addr);
    //从fd中获得连接的客户端信息并放置到sockaddr_in中
    if(getpeername(fd,
                (struct sockaddr*)&addr,&len)<0)
    {
        perror("getpeername error");
        return;
    }
    char ip[16];
    memset(ip,0,sizeof(ip));
    int port=ntohs(addr.sin_port);
    inet_ntop(AF_INET,
            &addr.sin_addr.s_addr,ip,sizeof(ip));
    printf("%16s(%5d) closed!\n",ip,port);
}

void* th_fn(void *arg)
{
    int fd=(int)(size_t)arg;
    do_service(fd);
    out_fd(fd);
    close(fd);

    return (void*)0;
}

int main(int argc,char *argv[])
{
	if(argc<2)
	{
        printf("usage:%s #port\n",argv[0]);
        exit(1);
	}

    if(signal(SIGINT,sig_handler)==SIG_ERR)
    {
        perror("signal sigint error");
        exit(1);
    }
    
    /*1. 创建socket
     AF_INT:ipv4
     SOCK_STREAM:tcp协议
    */
    sockfd=socket(AF_INET,SOCK_STREAM,0);
    if(sockfd<0){
        perror("socket error");
        exit(1);    
    }

    /*2:调用bind函数绑定socket和地址*/

    struct sockaddr_in serveraddr;
    memset(&serveraddr,0,sizeof(serveraddr));
    //往地址中填入ip，port，internet类型
    serveraddr.sin_family=AF_INET;  //ipv4
    serveraddr.sin_port=htons(atoi(argv[1]));  //htons主机字节序转成网络字节序

    serveraddr.sin_addr.s_addr=INADDR_ANY;

    if(bind(sockfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr))<0)
    {
        perror("bind error");
        exit(1);
        
    }

    /*调用listen函数监听（指定port监听）
    通知操作系统区接受来自客户顿的连接请求
    第二个参数：指定队列长度
    */
    
    if(listen(sockfd,10)<0)
    {
        perror("listen error");
    
    }

    /*调用accept函数从队列中获得一个客户端的请求连接
    */
    
    struct sockaddr_in clientaddr;
    socklen_t clientaddr_len=sizeof(clientaddr);
    
    //设置线程的分离属性
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,
                   PTHREAD_CREATE_DETACHED);

    while(1){
        //主控线程负责调用accept去获得客户端连接
        int fd=accept(sockfd,NULL,NULL);
        if(fd<0){
            perror("accept error");
            continue;
        }

        /*5.启动子线程去调用IO函数（read/write）和
            连接的客户端进行双向通信
        */
        pthread_t th;
        int err;
        if((err=pthread_create(&th,&attr,
                        th_fn,(void*)fd))!=0)
        {
            perror("pthread create error");
        }
        pthread_attr_destroy(&attr);

    }

    return 0;
}
