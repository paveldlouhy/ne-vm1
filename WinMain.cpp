/*
 *  WinMain.cpp
 *  NE-VM1
 *
 *  Created by Pavel Dlouhy (LD48) on 4/23/12.
 *  Copyright 2012 @PavelDlouhy. All rights reserved.
 *
 */


#include "CRealMachine.h"


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/gl.h>


#include <stdio.h>

#define LD_TIMER_ID 1


CRealMachine *g_rm;

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void OpenGLCreate(HWND hwnd, HDC hDC, HGLRC *hRC)
{
	PIXELFORMATDESCRIPTOR pfd;
	int format;

	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 16;
	format = ChoosePixelFormat(hDC, &pfd);
	SetPixelFormat(hDC, format, &pfd);

	*hRC = wglCreateContext(hDC);
	wglMakeCurrent(hDC, *hRC);
}

void OpenGLDestroy(HGLRC hRC)
{
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hRC);
}

int CALLBACK WinMain(
  HINSTANCE hInstance,
  HINSTANCE hPrevInstance,
  LPSTR lpCmdLine,
  int nCmdShow
)
{
	WNDCLASS wc;
	int quit = 0;
	MSG msg;
	HWND hwnd;
	HDC hDC;
	HGLRC hRC;

	wc.style = CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "NE-VM";

	RegisterClass(&wc);

	hwnd = CreateWindow(
		"NE-VM", NE_VM_WIN_WINDOWNAME /*displayed in the title bar*/,
		WS_CAPTION | WS_POPUPWINDOW | WS_VISIBLE,
		0, 0, GetSystemMetrics(SM_CXFIXEDFRAME) + RSCREEN_DX
			+ GetSystemMetrics(SM_CXFIXEDFRAME),
		GetSystemMetrics(SM_CYFIXEDFRAME) + GetSystemMetrics(SM_CYCAPTION) + RSCREEN_DY
			+ GetSystemMetrics(SM_CXFIXEDFRAME),
		NULL, NULL, hInstance, NULL);
 
	hDC = GetDC(hwnd);

	OpenGLCreate(hwnd, hDC, &hRC);

	g_rm = new CRealMachine();

	SetTimer(hwnd, LD_TIMER_ID, 17, (TIMERPROC)NULL);

	while (!quit)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			// handle or dispatch messages
			if ( msg.message == WM_QUIT ) 
			{
				quit = TRUE;
			} 
			else 
			{
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
		}
		//else
		{
			Sleep(10);
			/*{
				static int c = 0;
				c++;
				if(c >= 60) {
					c = 0;
					OutputDebugString("-After Sleep\n");
				}
			}*/
		}
	}

	KillTimer(hwnd, LD_TIMER_ID);

	delete g_rm;

	OpenGLDestroy(hRC);

	DestroyWindow(hwnd);

	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(
	HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
)
{
	switch (uMsg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	case WM_KEYDOWN:
		if (!g_rm)
			return 0;
		switch ( wParam )
		{
		case VK_SPACE:
			g_rm->PushEvent(REVNT_KEYDOWN, ' ');
			break;
		case VK_UP:
			g_rm->PushEvent(REVNT_KEYDOWN, 63232);
			break;
		case VK_DOWN:
			g_rm->PushEvent(REVNT_KEYDOWN, 63233);
			break;
		case VK_LEFT:
			g_rm->PushEvent(REVNT_KEYDOWN, 63234);
			break;
		case VK_RIGHT:
			g_rm->PushEvent(REVNT_KEYDOWN, 63235);
			break;
		case VK_ESCAPE:
			//PostQuitMessage(0);
			return 0;
		}
		return 0;
	case WM_KEYUP:
		if (!g_rm)
			return 0;
		switch ( wParam )
		{
		case VK_SPACE:
			g_rm->PushEvent(REVNT_KEYUP, ' ');
			break;
		case VK_UP:
			g_rm->PushEvent(REVNT_KEYUP, 63232);
			break;
		case VK_DOWN:
			g_rm->PushEvent(REVNT_KEYUP, 63233);
			break;
		case VK_LEFT:
			g_rm->PushEvent(REVNT_KEYUP, 63234);
			break;
		case VK_RIGHT:
			g_rm->PushEvent(REVNT_KEYUP, 63235);
			break;
		case VK_ESCAPE:
			//PostQuitMessage(0);
			return 0;
		}
		return 0;
	case WM_TIMER:
		switch (wParam)
		{
		case LD_TIMER_ID:
			if (g_rm) {
				g_rm->PushEvent(REVNT_TICK);
				g_rm->Render();
				SwapBuffers(wglGetCurrentDC());
			}
			/*{
				static int c = 0;
				c++;
				if(c >= 60) {
					c = 0;
					OutputDebugString("-WM_TIMER\n");
				}
			}*/
			return 0;
		}
		return 0;
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return 0;
}


