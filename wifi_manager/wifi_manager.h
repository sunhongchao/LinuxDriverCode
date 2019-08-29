#ifndef __WIFI_MANAGER_H
#define __WIFI_MANAGER_H
#include "wifi_opr.h"


typedef struct WifiMgr{
	int  (*SetNet)(char *cmd ,char *buf,char *ssid ,char *psk);
	int  (*RemoveNet)(char *cmd,char *buf,int id);
} T_WifiMgr, *PT_WifiMgr;

int WifiMgr_Init(PT_WifiMgr *pWifiMgr);
void WifiMgr_exit(PT_WifiMgr *ppWifiMgr);

#endif


