#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/times.h>
#include<sys/select.h>

#define FIFO_CLEAR 0X1
#define BUFFER_LEN 20
int main(void)
{
	int fd,num;
	char rd_ch[BUFFER_LEN];
	fd_set rfds,wfds;
	fd=open("/dev/globalfifo",O_RDWR|O_NONBLOCK);
	if(fd==-1)
	{
		printf("Device open failure\n");
	}
	else 
	{
		if(ioctl(fd,FIFO_CLEAR,0)<0)
		{
			printf("ioctl command failed\n");

		}
		while(1)
		{
			FD_ZERO(&rfds);
			FD_ZERO(&wfds);
			FD_SET(fd,&rfds);
			FD_SET(fd,&wfds);
			select(fd+1,&rfds,&wfds,NULL,NULL);
			if(FD_ISSET(fd,&rfds))
			{
				printf("poll monitor:cnan be read\n");
			}
			if(FD_ISSET(fd,&wfds))
			{
				printf("poll monitor:cnan be written\n");
			}
			sleep(2);
		}
	}
	close(fd);
	return 0;
}
