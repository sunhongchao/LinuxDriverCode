#ifndef __WIFI_OPR_H
#define __WIFI_OPR_H

typedef enum 
{
	INACTIVE	=	0,
	SCANNING 	=	1,
	FAIL 		=	2,
	COMPLETED  	=	3,
}Net_State_t;

typedef struct WifiOpr{
	int (*AddNetwork)(char *cmd,char *buf);
	int (*SetSsid)(char *cmd,char *buf,int id,char *ssid);
	int (*SetPsk)(char *cmd,char *buf,int id,char *psk);
	int (*SetPriority)(char *cmd,char *buf,int id,int priority);
	int (*SelectNetwork)(char *cmd,char *buf,int id);
	int (*SaveNetwork)(char *cmd,char *buf);
	int (*GetStatus)(char *cmd,char *buf);
	int (*ListNetwork)(char *cmd,char *buf);
	int (*DisNetwork)(char *cmd,char *buf,int id);
	int (*RemoveNetwork)(char *cmd,char *buf,int id);
}T_WifiOpr, *PT_WifiOpr;

void WifiOpr_Init(PT_WifiOpr pWifiOpr);
#endif






