#include <stdio.h>
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
static int HelloWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	switch (message) {
		case MSG_PAINT:
			hdc = BeginPaint (hWnd);
			TextOut (hdc, 60, 60, "Hello world!");
			EndPaint (hWnd, hdc);
		return 0;
		case MSG_CLOSE:
			DestroyMainWindow (hWnd);
			PostQuitMessage (hWnd);
		return 0;
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
	CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
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
