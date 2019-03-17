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
static void signalio_handler(int signum)
{
	
	printf("receive a signal from globalfifo,signalnum:%d\n",signum);
	
}
int main(void)
{

	int fd,oflags;
	fd=open("/dev/globalfifo",O_RDWR,S_IRUSR|S_IWUSR);
	if(fd!=-1)
	{	
		
		signal(SIGIO,signalio_handler);
		fcntl(fd,F_SETOWN,getpid());
		oflags=fcntl(fd,F_GETFL);
		fcntl(fd,F_SETFL,oflags|FASYNC);
		while(1)
		{
			sleep(100);
		}
		
	}	
	else
	{
		printf("device open failure\n");
	}
	close(fd);
	return 0;
}
