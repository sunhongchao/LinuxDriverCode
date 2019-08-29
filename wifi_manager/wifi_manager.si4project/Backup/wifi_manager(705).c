#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <error.h>
#include "wifi_manager.h"
#include "wifi_opr.h"


static PT_WifiOpr pWifiOpr	= NULL;
static int set_net(char *cmd ,char *buf,char *ssid ,char *psk)
{
	Net_State_t state = INACTIVE;
	int ret = 0;
	int id = 0;
	int priority = 0;
	printf("add_network\n");
	/*ret = pWifiOpr->AddNetwork(cmd,buf);
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
	ret = pWifiOpr->SetPsk(cmd,buf,id,psk);
	if(ret == -1){
		perror("set psk error\n");
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
	
	sleep(10);
	printf("get net status\n");
	ret = pWifiOpr->GetStatus(cmd,buf);
	if(ret != COMPLETED){
		perror("get net status error\n");
		return -1;
	}
	state = ret;

	if(state == COMPLETED){
		ret = pWifiOpr->SaveNetwork(cmd,buf);
		if(ret == -1){
			perror("save network error\n");
			return -1;
		}
	}*/
	return 0;
}
static int  remove_net(char *cmd,char *buf,int id)
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

int  WifiMgr_Init(PT_WifiMgr pWifiMgr)
{

	if(pWifiOpr == NULL){
		pWifiOpr = malloc(sizeof(T_WifiOpr));
		if(pWifiOpr == NULL)
			return -1;
		WifiOpr_Init(pWifiOpr);
		printf("pWifiOpr\n");
	}
	if(pWifiMgr == NULL){
		pWifiMgr = malloc(sizeof(T_WifiMgr));
		if(pWifiMgr == NULL){
			if(pWifiOpr->count == 0)
				free(pWifiOpr);
			return -1;
		} else {
			pWifiOpr->count++;
			pWifiMgr->SetNet = set_net;
			pWifiMgr->RemoveNet = remove_net;
			printf("pWifiMgr\n");
			
		}
	}
}
void WifiMgr_exit(PT_WifiMgr pWifiMgr)
{
	if(pWifiMgr != NULL){
		pWifiOpr->count--;
		if(pWifiOpr->count == 0)
			free(pWifiOpr);
		free(pWifiMgr);
	}
}






