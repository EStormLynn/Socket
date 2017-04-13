#include "msg.h"
#include "stdio.h"
#include "unistd.h"
#include "string.h"
#include "memory.h"
#include "sys/types.h"

//计算效验码
static unsigned char msg_check(Msg *message)
{
    unsigned char s=0;
    int i;
    for(i=0;i<sizeof(message->head);i++)
    {
        s+=message->head[i];
    }
    for(i=0;i<sizeof(message->buff);i++)
    {
        s+=message->buff[i];
    }
    return s;
}

//发送一个基于自定义协议的message，发送的数据存放再buff中
int write_msg(int sockfd,
                    char *buff,size_t len)
{
    Msg message;
    memset(&message,0,sizeof(message));
    strcpy(message.head,"Estorm2017");
    memcpy(message.buff,buff,len);
    message.checknum=msg_check(&message);
    printf("-------writeMsg\n"); //ffffffffffffffffffffffffff

    printf("----write return  %d \n",write(sockfd,&message,sizeof(message))); //ffffffffffffffffffffffffff

    printf("----sizeof messgae  %d \n",sizeof(message)); //ffffffffffffffffffffffffff
    if(write(sockfd,&message,
                sizeof(message))!=sizeof(message))
    {
        return -1;
    }
}
//读取一个基于自定义协议的message，读取数据存放再buff中
int read_msg(int sockfd,
                    char *buff,size_t len)
{
    Msg message;
    memset(&message,0,sizeof(message));
    size_t size;
    if((size=read(sockfd,
                &message,sizeof(message)))<0)
    {
        return -1;
    }else if(size==0)
    {
        return 0;
    }

    //进行校验码验证，判断接收到的message是否完整
    unsigned char s=msg_check(&message);
    printf("+meg check size %c\n",s);
    printf("+meg checknume %c\n",message.checknum);

    printf("%s",message.head);
    if((strcmp("Estorm2017",message.head))==0)
        printf("true\n");
    else
        printf("false\n");
    if((s==(unsigned char)message.checknum)
            ||(!strcmp("Estorm2017",message.head)))//这个地方不知道为什么打印输出，read Msg 的head会比write的时候多一个符号
    {
        printf("message size  read\n");//message size
        memcpy(buff,message.buff,len);
        return sizeof(message);
    }
    return -1;
}
