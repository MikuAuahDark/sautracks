/*
Main.cc
The GUI and the program core is in here
*/

#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <string>
#include <vector>

#include <Windows.h>

#include "UserTracks.h"

static HINSTANCE g_ModuleHandle;

//Returns the last Win32 error, in string format. Returns an empty string if there is no error.
// http://stackoverflow.com/a/17387176
inline std::string __lastErrorAsString()
{
    //Get the error message, if any.
    DWORD errorMessageID = ::GetLastError();
    if(errorMessageID == 0)
        return std::string(); //No error message has been recorded

    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 nullptr, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, nullptr);

    std::string message(messageBuffer, size);

    //Free the buffer.
    LocalFree(messageBuffer);

    return message;
}

inline void __showError(const char* message)
{
#ifndef SAUTRACKS_NOLOGS
	fprintf(stderr,"[ERROR]: %s\n",message);
#endif
	MessageBoxA(nullptr,message,"Error",MB_ICONEXCLAMATION|MB_OK);
}

inline void __showError(std::string message)
{
	__showError(message.c_str());
}

inline void __showError(const char* message1,const char* message2)
{
	char msg[512];

#ifndef SAUTRACKS_NOLOGS
	fprintf(stderr,"[ERROR]: %s: %s",message2,message1);
#endif
	sprintf(msg,"%s\n%s",message2,message1);
	MessageBoxA(nullptr,msg,"Error",MB_ICONEXCLAMATION|MB_OK);
}

inline void __showError(const char* message,const char* filename,int line)
{
	char msg[512];
	sprintf(msg,"%s line %u",filename,line);
	__showError(message,msg);
}

#ifdef SAUTRACKS_NOLOGS
#define __showInfo(message) (void)(message);
#else
#define __showInfo(message) fprintf(stdout,"[INFO]: %s\n",(message))
#endif

LRESULT __stdcall WindowProc(HWND hWnd,UINT msg,WPARAM wParameter,LPARAM lParameter)
{
	static HWND listBoxHWnd;
	static HWND addButtonHWnd;
	static HWND delButtonHWnd;
	static HWND loadButtonHWnd;
	static HWND saveButtonHWnd;
	static HFONT hFont;

	switch(msg)
	{
		case WM_CREATE:
		{
			__showInfo("Initializing controls");
			SendMessageA(hWnd,WM_SETFONT,WPARAM(hFont=HFONT(GetStockObject(DEFAULT_GUI_FONT))),LPARAM(true));
			SendMessageA(addButtonHWnd=CreateWindowExA(0,"BUTTON","Add",WS_TABSTOP|WS_VISIBLE|WS_CHILD|BS_DEFPUSHBUTTON,23,40,90,24,hWnd,nullptr,g_ModuleHandle,nullptr),WM_SETFONT,WPARAM(hFont),LPARAM(true));
			SendMessageA(delButtonHWnd=CreateWindowExA(0,"BUTTON","Remove",WS_TABSTOP|WS_VISIBLE|WS_CHILD|BS_DEFPUSHBUTTON,23,74,90,24,hWnd,nullptr,g_ModuleHandle,nullptr),WM_SETFONT,WPARAM(hFont),LPARAM(true));
			SendMessageA(loadButtonHWnd=CreateWindowExA(0,"BUTTON","Load",WS_TABSTOP|WS_VISIBLE|WS_CHILD|BS_DEFPUSHBUTTON,23,181,90,24,hWnd,nullptr,g_ModuleHandle,nullptr),WM_SETFONT,WPARAM(hFont),LPARAM(true));
			SendMessageA(saveButtonHWnd=CreateWindowExA(0,"BUTTON","Save",WS_TABSTOP|WS_VISIBLE|WS_CHILD|BS_DEFPUSHBUTTON,23,215,90,24,hWnd,nullptr,g_ModuleHandle,nullptr),WM_SETFONT,WPARAM(hFont),LPARAM(true));
			SendMessageA(listBoxHWnd=CreateWindowExA(WS_EX_CLIENTEDGE,"LISTBOX","",WS_VISIBLE|WS_CHILD|WS_VSCROLL|ES_AUTOVSCROLL|LBS_EXTENDEDSEL,130,40,400,225,hWnd,nullptr,g_ModuleHandle,nullptr),WM_SETFONT,WPARAM(hFont),LPARAM(true));
			return 0;
		}
		case WM_CLOSE:
		{
			__showInfo("Closing window");
			DestroyWindow(hWnd);
			return 0;
		}
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}
		case WM_PAINT:
		{
			PAINTSTRUCT* ps=new PAINTSTRUCT;
			HDC hDc=BeginPaint(hWnd,ps);

			SelectObject(hDc,HGDIOBJ(hFont));
			FillRect(hDc,&(ps->rcPaint),HBRUSH(COLOR_WINDOW+1));
			// Track List
			TextOutA(hDc,130,18,"Track List",10);
			// Count: %d
			{
				char temp[64];
				sprintf(temp,"Count: %d",SendMessageA(listBoxHWnd,LB_GETCOUNT,0,0));
				TextOutA(hDc,23,132,temp,strlen(temp));
			}
		}
		case WM_DROPFILES:
		{

			HDROP hDrop=HDROP(wParameter);
			int count=DragQueryFileA(hDrop,-1,nullptr,0);
			if(count==0) return 0;

			__showInfo("Drag & Drop file list");
			for(int i=0;i<count;i++)
			{
				char temp[MAX_PATH];
				char* ext=nullptr;

				DragQueryFileA(hDrop,i,temp,MAX_PATH);
				ext=strrchr(temp,'.');
				if(!ext || (strcmpi(ext,".ogg") && strcmpi(ext,".wav") && strcmpi(ext,".mp3") && strcmpi(ext,".aac") && strcmpi(ext,".m4a"))) continue;

				SendMessageA(listBoxHWnd,LB_ADDSTRING,0,LPARAM(temp));
				InvalidateRect(hWnd,nullptr,true);
				__showInfo(temp);
			}

			DragFinish(hDrop);
			return 0;
		}
		case WM_COMMAND:
		{
			if(HIWORD(wParameter)==BN_CLICKED)
			{
				HWND target=HWND(lParameter);
				if(target==addButtonHWnd)
				{
					__showInfo("Add button clicked");

					char temp[2048];
					OPENFILENAMEA ofn;
					memset(temp,0,2048);
					memset(&ofn,0,sizeof(OPENFILENAMEA));

					ofn.lStructSize=sizeof(OPENFILENAMEA);
					ofn.hwndOwner=hWnd;
					ofn.lpstrFilter="Supported Audio Files (*.ogg;*.wav;*.mp3;*.aac;*.m4a)\0*.ogg;*.wav;*.mp3;*.aac;*.m4a\0Ogg Vorbis (*.ogg)\0*.ogg\0WAVE Audio (*.wav)\0*.wav\0MPEG-Layer 3 (*.mp3)\0*.mp3\0AAC (*.aac;*.m4a)\0*.aac;*.m4a\0All Files (*.*)\0*.*\0\0";
					ofn.lpstrFile=temp;
					ofn.nMaxFile=2048;
					ofn.Flags=OFN_ALLOWMULTISELECT|OFN_EXPLORER|OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY|OFN_LONGNAMES|OFN_NONETWORKBUTTON;
					if(GetOpenFileNameA(&ofn)==false)
					{
						if(CommDlgExtendedError()!=0)
							__showError("Error in open file dialog");
						return 0;
					}

					// Parse
					{
						int strLength;
						char* filePtr=temp+strlen(temp)+1;
						while((strLength=strlen(filePtr))>0)
						{
							char filePath[MAX_PATH];
							filePath[0]=0;
							strcat(filePath,temp);
							strcat(filePath,"\\");
							strcat(filePath,filePtr);
							SendMessageA(listBoxHWnd,LB_ADDSTRING,0,LPARAM(filePath));
							filePtr+=strLength+1;
						}
						InvalidateRect(hWnd,nullptr,true);
					}
					return 0;
				}
				else if(target==delButtonHWnd)
				{
					__showInfo("Remove button clicked");

					int* itemsList;
					int itemCount=SendMessageA(listBoxHWnd,LB_GETCOUNT,0,0);
					int selectedCount=0;

					if(itemCount==0) return 0;
					itemsList=new int[itemCount];
					selectedCount=SendMessageA(listBoxHWnd,LB_GETSELITEMS,itemCount,LPARAM(itemsList))-1;
					for(int i=selectedCount;i>=0;i--)
						SendMessageA(listBoxHWnd,LB_DELETESTRING,WPARAM(itemsList[i]),0);
					
					InvalidateRect(hWnd,nullptr,true);
					return 0;
				}
				else if(target==loadButtonHWnd)
				{
					__showInfo("Load button clicked");

					std::vector<std::string> trackList;
					try
					{
						trackList=UserTracksLoad();
					}
					catch(std::exception e)
					{
						__showError(e.what());
						return 0;
					}
					SendMessageA(listBoxHWnd,LB_RESETCONTENT,0,0);

					for(auto i=trackList.begin();i!=trackList.end();i++)
						SendMessageA(listBoxHWnd,LB_ADDSTRING,0,LPARAM((*i).c_str()));

					
					InvalidateRect(hWnd,nullptr,true);
					// Show message
					{
						char msg[512];
						sprintf(msg,"%u user tracks in-game",trackList.size());
						__showInfo(msg);
						MessageBoxA(nullptr,msg,"Info",MB_ICONINFORMATION|MB_OK);
					}

					return 0;
				}
				else if(target==saveButtonHWnd)
				{
					__showInfo("Save button clicked");

					int itemsSize=SendMessageA(listBoxHWnd,LB_GETCOUNT,0,0);
					std::vector<std::string> fileList;

					for(int i=0;i<itemsSize;i++)
					{
						char temp[MAX_PATH];

						SendMessageA(listBoxHWnd,LB_GETTEXT,WPARAM(i),LPARAM(temp));
						fileList.push_back(std::string(temp));
					}

					auto err=UserTracksSave(fileList);
					if(err>0) __showError(strerror(err));
					else
					{
						__showInfo("Saved");
						MessageBoxA(nullptr,"Saved","Info",MB_ICONINFORMATION|MB_OK);
					}
					return 0;
				}
			}
		}
		default:
			return DefWindowProcA(hWnd,msg,wParameter,lParameter);
	}
	return 0;
}

int __stdcall WinMain(HINSTANCE hInstance,HINSTANCE uiv,LPSTR args,int nCmdShow)
{
	UNREFERENCED_PARAMETER(uiv);

	WNDCLASSA wc;
	HWND hWnd;
	MSG* msg=new MSG;

	g_ModuleHandle=hInstance;

	wc.style=CS_HREDRAW|CS_VREDRAW|CS_SAVEBITS;
	wc.lpfnWndProc=&WindowProc;
	wc.cbClsExtra=0;
	wc.cbWndExtra=0;
	wc.hInstance=hInstance;
	wc.hIcon=nullptr;
	wc.hCursor=nullptr;
	wc.hbrBackground=HBRUSH(COLOR_WINDOW+1);
	wc.lpszMenuName=nullptr;
	wc.lpszClassName="sautracks";
	if(!RegisterClassA(&wc))
	{
		__showError(__lastErrorAsString().c_str(),__FILE__,__LINE__);
		return EXIT_FAILURE;
	}

	__showInfo("Creating window");
	hWnd=CreateWindowExA(WS_EX_CLIENTEDGE,"sautracks","SA User Tracks Generator",WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,CW_USEDEFAULT,CW_USEDEFAULT,550,300,nullptr,nullptr,hInstance,nullptr);
	if(hWnd==nullptr)
	{
		__showError(__lastErrorAsString().c_str(),__FILE__,__LINE__);
		return EXIT_FAILURE;
	}
	ShowWindow(hWnd,nCmdShow);
	UpdateWindow(hWnd);
	DragAcceptFiles(hWnd,true);

	while(GetMessageA(msg,nullptr,0,0)>0)
	{
		TranslateMessage(msg);
		DispatchMessageA(msg);
	}
	return 0;
}