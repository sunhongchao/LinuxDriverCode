#include <netdb.h>
#include<sys/socket.h>
#include <arpa/inet.h>
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<memory.h>
#include<unistd.h>
int main(int argc, char *argv[])
{
    if(argc<3){
        printf("usage:%s ip port\n",argv[0]);
        exit(1);
    }
    /*步骤1*/
     int sockfd=socket(AF_INET,SOCK_STREAM,0);
         if(sockfd<0){
        perror("socket error");
        exit(1);
    }     
    struct sockaddr_in serveraddr;
    memset(&serveraddr,0,sizeof(serveraddr));
    serveraddr.sin_family=AF_INET;
    //端口号要求网络字节序，命令行传递的是字符串，需要先转换为int.
    serveraddr.sin_port=htons(atoi(argv[2]));
    //将ip地址转化为网络字节序
    inet_pton(AF_INET,argv[1],&serveraddr.sin_addr.s_addr);
    /*步骤2：客户端调用connect函数连接到服务器端*/
    if(connect(sockfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr))<0)
    {
        perror("connect error");
        exit(1);
    }
    /*步骤3：读写*/
    char buffer[1024];
    memset(buffer,0,sizeof(buffer));
    size_t size;
	while(1){
		if((size = read(sockfd,buffer,sizeof(buffer))) <0 ){
		    perror("read error");      
		 }
		 if(size > 0){
			if(write(STDOUT_FILENO,buffer,size)!=size){
		    	perror("write error"); 
			}
		}
		sleep(5);
	}
    /*步骤4：关闭socket*/
    close(sockfd);
    return 0;
 
 
}

