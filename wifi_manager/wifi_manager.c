#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <error.h>
#include "wifi_manager.h"
#include "wifi_opr.h"


static PT_WifiOpr pWifiOpr	= NULL;
int set_net(char *cmd ,char *buf,char *ssid ,char *psk)
{
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
        }
	
	return 0;
}
int  remove_net(char *cmd,char *buf,int id)
{
	//save network
	int ret = 0;
        ret = pWifiOpr->DisNetwork(cmd,buf,id);
	if(ret == -1){
            perror("Disable network error\n");
            return -1;
	}
        ret = pWifiOpr->RemoveNetwork(cmd,buf,id);
	if(ret == -1){
            perror("remove network error\n");
            return -1;
	}
	ret = pWifiOpr->SaveNetwork(cmd,buf);
	if(ret == -1){
            perror("save network error\n");
            return -1;
	}
	return 0;
}

int  WifiMgr_Init(PT_WifiMgr *ppWifiMgr)
{

	if(pWifiOpr == NULL){
            pWifiOpr = malloc(sizeof(T_WifiOpr));
            if(pWifiOpr == NULL)
		return -1;
            WifiOpr_Init(pWifiOpr);
            
	}
	if((*ppWifiMgr) == NULL){
		(*ppWifiMgr) = malloc(sizeof(T_WifiMgr));
		if((*ppWifiMgr) == NULL){
			if(pWifiOpr->count == 0)
				free(pWifiOpr);
			return -1;
		} else {
                        pWifiOpr->count++;
			(*ppWifiMgr)->SetNet = set_net;
			(*ppWifiMgr)->RemoveNet = remove_net;	
		}
	}
   
        return 0;
}

void WifiMgr_exit(PT_WifiMgr *ppWifiMgr)
{
	if((*ppWifiMgr) != NULL){
		pWifiOpr->count--;
		if(pWifiOpr->count == 0)
			free(pWifiOpr);
		free(*ppWifiMgr);
	}
}






