#include <unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#define BUFFER_LEN 4096


int main(int argc,char *argv[])
{
	int fd;
	char buffer[BUFFER_LEN];
    ssize_t size;
	char *ssid;
	char *psk;
	char *key_mgmt;
	char  str[128]; 
    if(argc!=5){
        fprintf(stderr,"usage:%s srcfile \n",argv[0]);
        exit(1);
    }
    sprintf(str,"network={\n    ssid=\"%s\"\n    psk=\"%s\"\n    key_mgmt=%s\n}",argv[2],argv[3],argv[4]);

    fd=open(argv[1],O_RDWR);
    if(fd<0){
        fprintf(stderr,"open error:%s\n",strerror(errno));
        exit(1);        
    }else {
        printf("open file:%d\n",fd);     
    }
  
    while(size=read(fd,buffer,BUFFER_LEN)>0){
		if(NULL ==  strstr(buffer,"ssid")){
					 lseek(fd,0,SEEK_END);
					 write(fd,str,strlen(str));
		}
	}
   
    if(size<0){	//读错误
       //errno是系统设置的错误编码，是上次系统调用产生错误时设置
         fprintf(stderr,"read error:%s\n",strerror(errno));
         exit(1);
    }
    close(fd);
    return 0;
}


