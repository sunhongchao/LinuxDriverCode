#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "wifi_manager.h"
/*
 Wifi_Opr对应wpa命令，是基本的操作方法，以后可以扩展iw工具操作方法
 Wifi_Mgr对wifi操作的封装，一个Wifi_Mgr类似一个对象，对应于一个网卡
 网络状态采用定时器，每10s执行一次获取网络状态函数
*/

static void print_usage(char *file)
{
    printf("Usage:\n");
    printf("%s set net_name net_password\n",file);
	printf("%s unset id\n",file);
}
// add a network


int main(int argc ,char *argv[])
{
	char ssid[20];
	char psk[20];
	char cmd[128];
	char buf[128];
	int ret = 0;
	int priority = 0;
	int id = 0;
	PT_WifiMgr pWifiMgr = NULL;
	memset(ssid,0,20);
	memset(psk,0,20);
	memset(cmd,0,128);
	memset(buf,0,128);
	
	if(argc < 3){
		print_usage(argv[0]);
		goto error;	
	} 
	 WifiMgr_Init(pWifiMgr);
	if(!strcmp(argv[1],"set")){
		strcpy(ssid,argv[2]);
		strcpy(psk,argv[3]);
		printf("ssid:%s;password:%s\n",ssid,psk);
		ret = pWifiMgr->SetNetwork(cmd,buf,ssid,psk);
		if(ret == -1){
			perror("set network error\n");
			goto error;
		}
	} else if(!strcmp(argv[1],"unset")){
		id = atoi(argv[2]);
		if( pWifiMgr->RemoveNetwork(cmd,buf,id) == -1){
			perror("remove netowrk error\n");
			goto error;
		}
	} else {
		print_usage(argv[0]);
	}
	return 0;
error:
	return -1;
 	
}

