/*
WinMainWrapper.cc
Calls WinMain() from main() if it was bulit as Console Application
*/

#include <csignal>
#include <Windows.h>
#include <CommCtrl.h>

void sigint_handler(int v)
{
	MessageBoxA(GetConsoleWindow(),"Do not press Ctrl+C in the console","Warning",MB_ICONEXCLAMATION|MB_OK);
	signal(SIGINT,sigint_handler);
}

int main(int argc,char* argv[])
{
	INITCOMMONCONTROLSEX icc;
	LPSTR lpCmdLine=new char[1024];
	HINSTANCE hInstance=GetModuleHandle(nullptr);
	int nCmdShow=SW_SHOWDEFAULT;
	STARTUPINFOA startupInfo;

	// Enable visual styles
	icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icc.dwICC = ICC_STANDARD_CLASSES;
	InitCommonControlsEx(&icc);

	// Converts argv to lpCmdLine
	memset(lpCmdLine,0,1024);
	strcpy(lpCmdLine,argv[0]);
	for(int i=1;i<argc;i++)
	{
		strcat(lpCmdLine," ");
		strcat(lpCmdLine,argv[i]);
	};

	// get nCmdShow
	GetStartupInfoA(&startupInfo);
	if(startupInfo.dwFlags&STARTF_USESHOWWINDOW)
		nCmdShow=startupInfo.wShowWindow;

	// Warnings user not to press Ctrl+C
	signal(SIGINT,&sigint_handler);

	return WinMain(hInstance,nullptr,lpCmdLine,nCmdShow);
}