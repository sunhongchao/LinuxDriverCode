#ifndef __WIFI_MANAGER_H
#define __WIFI_MANAGER_H
#include "wifi_opr.h"


typedef struct WifiMgr{
	int  (*SetNetwork)(char *cmd ,char *buf,char *ssid ,char *psk);
	int  (*RemoveNetwork)(char *cmd,char *buf,int id);
} T_WifiMgr, *PT_WifiMgr;

void WifiMgr_Init(PT_WifiMgr pWifiMgr);

#endif


