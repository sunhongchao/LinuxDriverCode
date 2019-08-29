#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "wifi_manager.h"
#include "wifi_opr.h"

static T_WifiOpr WifiOpr;
static PT_WifiOpr pWifiOpr	=	&WifiOpr;
static int set_network(char *cmd ,char *buf,char *ssid ,char *psk)
{
	FILE *fp;
	Net_State_t state = INACTIVE;
	int ret = 0;
	int id = 0;
	int priority = 0;
	printf("add_network\n");
	ret = pWifiOpr->AddNetwork(cmd,buf);
	if(ret == -1){
		perror("add_network error\n");
		return -1;
	} else {
		id = ret;
		printf("get netid:%d\n",id);
	}
	printf("set_ssid\n");
	ret = pWifiOpr->SetSsid(cmd,buf,id,ssid);
	if(ret == -1){
		perror("set ssid error\n");
		return -1;
	} 
	printf("set_psk\n");
	ret = pWifiOpr->SetPsk(cmd,buf,id,ssid);
	if(ret == -1){
		perror("set ssid error\n");
		return -1;
	} 
    printf("set priority\n");
	priority = id +1;
	ret = pWifiOpr->SetPriority(cmd,buf,id,priority);
	if(ret == -1){
		perror("set priority error\n");
		return -1;
	} 
	 printf(" select network\n");
	ret = pWifiOpr->SelectNetwork(cmd,buf,id);
	if(ret == -1){
		perror("select network error\n");
		return -1;
	} 
	//get net status
	sleep(10);
	printf("get net status\n");
	ret = pWifiOpr->GetStatus(cmd,buf);
	if(ret != COMPLETED){
		perror("get net status error\n");
		return -1;
	}
	state = ret;
	//save network
	if(state == COMPLETED){
		ret = pWifiOpr->SaveNetwork(cmd,buf);
		if(ret == -1){
			perror("save network error\n");
			return -1;
		}
	}
	return 0;
}
static int  remove_network(char *cmd,char *buf,int id)
{
	//save network
	int ret = 0;
	ret = pWifiOpr->SaveNetwork(cmd,buf);
	if(ret == -1){
		perror("save network error\n");
		return -1;
	}
	return 0;
}

void WifiMgr_Init(PT_WifiMgr pWifiMgr)
{
	WifiOpr_Init(pWifiOpr);
	pWifiMgr->SetNetwork		=	set_network;
	pWifiMgr->RemoveNetwork		=	remove_network;
}





