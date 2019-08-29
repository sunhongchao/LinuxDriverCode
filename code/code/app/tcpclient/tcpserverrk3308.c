#include <netdb.h>
#include<sys/socket.h>
#include <arpa/inet.h>
#include<unistd.h>
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<memory.h>
#include<signal.h>
#include<time.h>
 
int sockfd;
//
void sig_handler(int signo)
{
    if(signo==SIGINT){
    printf("server close\n");
//步骤6
    close(sockfd);
    exit(1);
}
}
void out_addr(struct sockaddr_in *clientaddr)
{
    int port=ntohs(clientaddr->sin_port);
    char ip[16];
    memset(ip,0,sizeof(ip));
//将Ip地址从网络字节序转换成点分十进制
    inet_ntop(AF_INET,&clientaddr->sin_addr.s_addr,ip,sizeof(ip));
    printf("client:%s(%d) connected\n",ip,port);
}
//连接成功后，向客户端发送系统时间
void do_serverice(int fd)
{
    long t =time(0);
     char *s =ctime(&t);
    size_t size=strlen(s)*sizeof(char);
    if(write(fd,s,size)!=size){
        perror("write error");
    }
}
int main(int argc, char *argv[])
{
    if(argc<2){
        printf("usage:%s #port\n",argv[0]);
        exit(1);
    }
if(signal(SIGINT,sig_handler)==SIG_ERR){//注册信号处理函数
    perror("signal sigint error");
    exit(1);
}
/* 步骤1：创建socket套接字
 socket 创建在内核中*/
sockfd=socket(AF_INET,SOCK_STREAM,0);
struct sockaddr_in serveraddr;
memset(&serveraddr,0,sizeof(serveraddr));
serveraddr.sin_family=AF_INET;
//端口号要求网络字节序，命令行传递的是字符串，需要先转换为int.
serveraddr.sin_port=htons(atoi(argv[1]));
/*步骤2
一台主机有多个网络接口和ip地址，如果只关心某个地址的请求连接，可以指定一个具体的ip.要响应所有接口ip就需要一个特殊的地址INADDR_ANY.
#define INADDR_ANY (uint32_t )0x00000000
*/
serveraddr.sin_addr.s_addr=INADDR_ANY;
if(bind(sockfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr))<0){
    perror("bind error");
    exit(1);
}
/*步骤3
最多同时10个连接。相当listen于创建了一个单独的线程用来处理连接请求
*/
if(listen(sockfd,10)<0){
    perror("listen error");
    exit(1);
}
/*步骤4：调用accept函数从队列中获取一个客户端的请求连接，并返回新的socket描述符，如果没有客户端连接，调用此函数后会阻塞*/
struct sockaddr_in clientaddr;
socklen_t clientaddr_len=sizeof(clientaddr);
while(1){
    int fd=accept(sockfd,(struct sockaddr*)&clientaddr,& clientaddr_len);
if(fd<0){
        perror("accept error");
        continue;
}
/*步骤5：调用IO函数(read/write)和连接的客户端进行双向通信*/
out_addr(&clientaddr);
do_serverice(fd);
/*步骤6：通信完成，关闭和客户端连接的socket*/
close(fd);
 
}
return 0;
}

