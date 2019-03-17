#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/times.h>
#include<sys/select.h>
#include<signal.h>
#define FIFO_CLEAR 0X1
#define BUFFER_LEN 20
struct dev_t
{
	int fd;
	int oflags;
	fd_set rfds;
	fd_set wfds;
};
struct dev_t  dev;
static void signalio_handler(int signum)
{
	
	printf("receive a signal,signalnum:%d\n",signum);
	FD_ZERO(&dev.rfds);
	FD_ZERO(&dev.wfds);
	FD_SET(dev.fd,&dev.rfds);
	FD_SET(dev.fd,&dev.wfds);
	select(dev.fd+1,&dev.rfds,&dev.wfds,NULL,NULL);
	if(FD_ISSET(dev.fd,&dev.wfds))
	{
		printf("poll monitor:can be written\n");
	}
	if(FD_ISSET(dev.fd,&dev.rfds))
	{
		printf("poll monitor:can be read\n");
	}
	
	
}
int main(void)
{

	
	dev.fd=open("/dev/globalfifo",O_RDWR|O_NONBLOCK,S_IRUSR|S_IWUSR);
	if(dev.fd!=-1)
	{	
		if(ioctl(dev.fd,FIFO_CLEAR,0)<0)
		{
			printf("ioctl command failed\n");

		}
		signal(SIGIO,signalio_handler);
		fcntl(dev.fd,F_SETOWN,getpid());
		dev.oflags=fcntl(dev.fd,F_GETFL);
		fcntl(dev.fd,F_SETFL,dev.oflags|FASYNC);
		while(1)
		{
			sleep(100);
		}
		
	}	
	else
	{
		printf("device open failure\n");
	}
	close(dev.fd);
	return 0;
}
