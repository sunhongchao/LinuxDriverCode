#include <stdio.h>
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#define IDC_MYBUTTON    100
static int HelloWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	
	static int num=0;
    char buf[20]="";
	switch (message) {
		
		case MSG_CREATE:
			printf("MSG_CREATE\n");
			CreateWindow (CTRL_BUTTON, "", WS_VISIBLE | WS_CHILD, IDC_MYBUTTON, 
                10, 80, 200, 40, hWnd, 0);
        SetWindowText(GetDlgItem(hWnd,IDC_MYBUTTON),"normal:0");
		break;
		case MSG_MOUSEMOVE:
			printf("MSG_MOUSEMOVE\n");
			message = MSG_LBUTTONDOWN;
		case MSG_LBUTTONDOWN:
			printf("MSG_LBUTTONDOWN\n");
			
		break;
		case MSG_LBUTTONUP:
			printf("MSG_LBUTTONUP\n");
			message = MSG_COMMAND;
		break;
		case MSG_COMMAND:
   		 {
			int id   = LOWORD(wParam);
			int nc = HIWORD(wParam);
		    printf(" %d %d\n",id,nc);
			printf("MSG_COMMANDn");
		    if(id == IDC_MYBUTTON && nc == BN_CLICKED) {			//控件的消息处理
           		++num;
           		sprintf(buf,"normal:%d",num);
           		SetWindowText(GetDlgItem(hWnd,IDC_MYBUTTON),buf);
        	 }
   		 }
		case MSG_PAINT:
			hdc = BeginPaint (hWnd);	      
        	EndPaint(hWnd, hdc);
        return 0;
		break;
		case MSG_CLOSE:
			DestroyAllControls (hWnd);
        	DestroyMainWindow (hWnd);
        	PostQuitMessage (hWnd);
		break;
	}
	return DefaultMainWinProc (hWnd, message, wParam, lParam);
}
int MiniGUIMain (int argc, const char* argv[])
{
	MSG Msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;
	#ifdef _MGRM_PROCESSES
	JoinLayer (NAME_DEF_LAYER , "helloworld" , 0 , 0);
	#endif
	CreateInfo.dwStyle = WS_VISIBLE|WS_BORDER;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = "HelloWorld";
	CreateInfo.hMenu = 0;
	CreateInfo.hCursor = GetSystemCursor (0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = HelloWinProc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = 320;
	CreateInfo.by = 240;
	CreateInfo.iBkColor = COLOR_lightwhite;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = HWND_DESKTOP;
	hMainWnd = CreateMainWindow (&CreateInfo);
	if (hMainWnd == HWND_INVALID)
	return -1;
		ShowWindow (hMainWnd, SW_SHOWNORMAL);
	while (GetMessage (&Msg, hMainWnd)) {
		TranslateMessage (&Msg);
		DispatchMessage (&Msg);
	}
	MainWindowThreadCleanup (hMainWnd);
	return 0;
}
#ifndef _MGRM_PROCESSES
#include <minigui/dti.c>
#endif
