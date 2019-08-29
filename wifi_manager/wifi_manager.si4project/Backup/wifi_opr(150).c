#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "wifi_opr.h"

static int add_network(char *cmd,char *buf)
{	
	FILE *fp;
	int id = 0;
	sprintf(cmd,"wpa_cli -i wlan0 add_network");
	if((fp = popen(cmd,"r")) == NULL)
		return -1;
	if(fgets(buf,128,fp) == NULL){
		pclose(fp);
		return -1;
	}
	id = atoi(buf);
	pclose(fp);
	return id;
}

static int set_ssid(char *cmd,char *buf,int id,char *ssid)
{
	FILE *fp;
	sprintf(cmd,"wpa_cli -i wlan0 set_network %d ssid '\"%s\"'",id,ssid);
	if((fp = popen(cmd,"r")) == NULL)
		return -1;
	if(fgets(buf,128,fp) == NULL){
		pclose(fp);
		return -1;
	} 
	if(!strcmp(buf,"OK")){
		pclose(fp);
		return -1;
	}
	pclose(fp);
	return 0;
}

static int set_psk(char *cmd,char *buf,int id,char *psk)
{
	FILE *fp;
	sprintf(cmd,"wpa_cli -i wlan0 set_network %d psk '\"%s\"'",id,psk);
	if((fp = popen(cmd,"r")) == NULL)
		return -1;
	if(fgets(buf,128,fp) == NULL){
		pclose(fp);
		return -1;
	}
	if(!strcmp(buf,"OK")){
		pclose(fp);
		return -1;
	}	
	pclose(fp);
	return 0;
}

static int set_priority(char *cmd,char *buf,int id,int priority)
{
	FILE *fp;
	sprintf(cmd,"wpa_cli -i wlan0 set_network %d priority %d",id,priority);
	if((fp = popen(cmd,"r")) == NULL)
		return -1;
	if(fgets(buf,128,fp) == NULL){
		pclose(fp);
		return -1;
	}
	if(!strcmp(buf,"OK")){
		pclose(fp);
		return -1;
	}
	pclose(fp);
	return 0;
}

//select network
static int select_network(char *cmd,char *buf,int id)
{
	FILE *fp;
	sprintf(cmd,"wpa_cli -i wlan0 select_network %d",id);
	if((fp = popen(cmd,"r")) == NULL)
		return -1;	
	if(fgets(buf,128,fp) == NULL){
		pclose(fp);
		return -1;
	}
	if(!strcmp(buf,"OK")){	
		pclose(fp);
		return -1;
	}
	pclose(fp);
	return 0;

}
// save current linked network
static int save_network(char *cmd,char *buf)
{
	FILE *fp;
	sprintf(cmd,"wpa_cli -i wlan0 save_config");
	if((fp = popen(cmd,"r")) == NULL)
		return -1;
	if(fgets(buf,128,fp) == NULL){
		pclose(fp);
		return -1;
	}
	if(!strcmp(buf,"OK")){
		pclose(fp);
		return -1;
	}
	pclose(fp);
	return 0;
}

static int disable_network(char *cmd,char *buf,int id)
{
	FILE *fp;
	int ret = 0;
	sprintf(cmd,"wpa_cli -i wlan0 disable_network %d",id);
	if((fp = popen(cmd,"r")) == NULL)
		return -1;
	if(fgets(buf,128,fp) == NULL){
		pclose(fp);
		return -1;
	}
	if(!strcmp(buf,"OK")){
		pclose(fp);
		return -1;
	}	
	pclose(fp);
}
static int remove_network(char *cmd,char *buf,int id)
{
	FILE *fp;
	sprintf(cmd,"wpa_cli -i wlan0 remove_network %d",id);
	if((fp = popen(cmd,"r")) == NULL)
		return -1;
	if(fgets(buf,128,fp) == NULL){
		pclose(fp);
		return -1;
	}	
	if(!strcmp(buf,"OK")){
		pclose(fp);
		return -1;
	}	
	pclose(fp);
}
static int list_network(char *cmd,char *buf)
{
	FILE *fp;
	sprintf(cmd,"wpa_cli -i wlan0 list_network");
	if((fp = popen(cmd,"r")) == NULL)
		return -1;
	while(fgets(buf,128,fp) != NULL){
		printf("%s",buf);
	}
	pclose(fp);
	return 0;
}

static int get_status(char *cmd,char *buf)
{
	FILE *fp;
	int ret = 0;
	Net_State_t state = INACTIVE;
	sprintf(cmd,"wpa_cli -i wlan0 status");
	if((fp = popen(cmd,"r")) == NULL)
		return -1;
	if(fgets(buf,128,fp) == NULL){
		pclose(fp);
		return -1;
	}
	do{
		printf("%s",buf);
		if(strstr(buf,"wpa_state")){
			if(strstr(buf+strlen("wpa_state")+1,"COMPLETED")){
				ret = COMPLETED;
			} else if(strstr(buf+strlen("wpa_state")+1,"INACTIVE")){
				ret = INACTIVE;
			} if(strstr(buf+strlen("wpa_state")+1,"SCANNING")){
				ret = SCANNING;
			}
		}
	}
	while(fgets(buf,128,fp) != NULL);
	pclose(fp);
	return ret;
}

void WifiOpr_Init(PT_WifiOpr pWifiOpr)
{

	pWifiOpr->AddNetwork		=	add_network;
	pWifiOpr->SetSsid   		=	set_ssid;
	pWifiOpr->SetPsk   		=		set_psk;
	pWifiOpr->SetPriority		=	set_priority;
	pWifiOpr->SelectNetwork		=	select_network;
	pWifiOpr->GetStatus			=	get_status;
	pWifiOpr->SaveNetwork  		=	save_network;
	pWifiOpr->ListNetwork		=	list_network;
	pWifiOpr->DisNetwork		=  	disable_network;
	pWifiOpr->RemoveNetwork		=	remove_network;
}



