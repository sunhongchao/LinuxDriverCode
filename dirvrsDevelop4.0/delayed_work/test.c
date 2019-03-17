#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/times.h>
#include<sys/select.h>
#include<signal.h>
int main(void)
{
	int fd;
	int counter=0;
	fd=open("/dev/second",O_RDONLY);
	if(fd!=-1){
		while(1)
		{
			read(fd,&counter,sizeof(unsigned int));//读目前经历的秒数
			printf("seconds after open /dev/second:%d\n",counter);
			sleep(1);
		}
		
	}else{
		printf("device open failure\n");
	}
	close(fd);
	return 0;
}








