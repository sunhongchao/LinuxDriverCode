#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "wifi_manager.h"
/*
 Wifi_Opr��Ӧwpa����ǻ����Ĳ����������Ժ������չiw���߲�������
 Wifi_Mgr��wifi�����ķ�װ��һ��Wifi_Mgr����һ�����󣬶�Ӧ��һ������
 ����״̬���ö�ʱ����ÿ10sִ��һ�λ�ȡ����״̬����
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

