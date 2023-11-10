#include "CvGameCoreDll.h"

#if defined(FASSERT_ENABLE) && defined(WIN32)

#include "FDialogTemplate.h"

#include <tchar.h>
#include <stdio.h>

namespace
{
	// These are the return values from the modal Assert Dialog
	enum
	{
		ASSERT_DLG_DEBUG		= 0x01,
		ASSERT_DLG_IGNORE		= 0x02,
		ASSERT_DLG_IGNOREALWAYS = 0x03,
		ASSERT_DLG_EXIT			= 0x04,
	};

	// This global structure is filled out by the original call to our FAssert
	// and is used by the dialog proc to display appropriate debug info
	struct AssertInfo
	{
		const char* szExpression;
		const char* szMessage;
		const char* szFileName;
		unsigned int line;

		// EIP / EBP / ESP
		CONTEXT context;
	} g_AssertInfo;

	// Use a static array since often times asserts are fired in response to problems
	// caused by being low on resources, so dynamically creating strings here might not
	// be such a hot idea
	const unsigned int MAX_ASSERT_TEXT=65536;
	char g_AssertText[MAX_ASSERT_TEXT];

	// Some Resource IDs for our dialog template
#define IDC_IGNORE_ALWAYS               1001
#define IDC_IGNORE_ONCE                 1002
#define IDC_DEBUG                       1003
#define IDC_ABORT                       1004
#define IDC_ASSERTION_TEXT              1005
#define IDC_COPY_TO_CLIPBOARD           1006

	INT_PTR CALLBACK AssertDlgProc(HWND hDlg, UINT msg,WPARAM wParam, LPARAM lParam)
	{
		switch(msg)
		{
		case WM_INITDIALOG:
			{
				char modulePath[MAX_PATH];
				GetModuleFileName(NULL, modulePath, MAX_PATH);

				const char* moduleName = strrchr(modulePath, '\\');
				moduleName = moduleName ? moduleName+1 : modulePath;

				char title[MAX_PATH + 20];
				sprintf(title, "Assert Failed: %s", moduleName);
				SetWindowText(hDlg, title);

				sprintf( g_AssertText, "Assert Failed\r\n\r\n"
					"File:  %s\r\n"
					"Line:  %d\r\n"
					"Expression:  %s\r\n" 
					"Message:  %s\r\n"
					"\r\n"
					"----------------------------------------------------------\r\n",
					g_AssertInfo.szFileName,
					g_AssertInfo.line,
					g_AssertInfo.szExpression,
					g_AssertInfo.szMessage ? g_AssertInfo.szMessage : "" );

				::SetWindowText( ::GetDlgItem(hDlg, IDC_ASSERTION_TEXT), g_AssertText );
				::SetFocus( ::GetDlgItem(hDlg, IDC_DEBUG) );

				break;
			}
		case WM_COMMAND:
			{
				switch(LOWORD(wParam))
				{
				case IDC_DEBUG:
					EndDialog(hDlg, ASSERT_DLG_DEBUG);
					return TRUE;

				case IDC_IGNORE_ONCE:
					EndDialog(hDlg, ASSERT_DLG_IGNORE);
					return TRUE;

				case IDC_IGNORE_ALWAYS:
					EndDialog(hDlg, ASSERT_DLG_IGNOREALWAYS);
					return TRUE;

				case IDC_ABORT:
					EndDialog(hDlg, ASSERT_DLG_EXIT);
					return TRUE;
				}
			}
			break;
		}

		return FALSE;
	}

	DWORD DisplayAssertDialog()
	{
		CDialogTemplate dialogTemplate(_T("Assert Failed!"), 
			DS_SETFONT | DS_CENTER | DS_MODALFRAME | DS_FIXEDSYS | WS_POPUP | WS_CAPTION | WS_SYSMENU,
			0, 0, 379, 166, _T("MS Shell Dlg"), 8);

		dialogTemplate.AddButton( _T("Ignore Always"), WS_VISIBLE, 0, 
			157,145,64,14, IDC_IGNORE_ALWAYS );

		dialogTemplate.AddButton( _T("&Ignore Once"), WS_VISIBLE, 0,
			82,145,64,14, IDC_IGNORE_ONCE );

		dialogTemplate.AddButton( _T("&Debug"), WS_VISIBLE, 0,
			307,145,64,14, IDC_DEBUG );

		dialogTemplate.AddButton( _T("&Abort"), WS_VISIBLE, 0,
			232,145,64,14, IDC_ABORT );

		dialogTemplate.AddEditBox( _T(""), ES_MULTILINE | ES_AUTOVSCROLL | 
			ES_AUTOHSCROLL | ES_READONLY | WS_VSCROLL | WS_HSCROLL | WS_VISIBLE, WS_EX_STATICEDGE,
			7,7,365,130, IDC_ASSERTION_TEXT );

		int res = DialogBoxIndirect(GetModuleHandle(0), dialogTemplate.GetDialogTemplate(), NULL, (DLGPROC)AssertDlgProc);
		return res;
	}

} // end anonymous namespace

//SpyFanatic: small stacktrace in assert dialog, for whom does not own a debugger
//https://github.com/JochenKalmbach/StackWalker
//https://jpassing.com/2008/03/12/walking-the-stack-of-the-current-thread/
#include <dbghelp.h>
enum
{
	STACKWALK_MAX_NAMELEN = 1024
}; // max name length for found symbols

typedef struct CallstackEntry
{
	DWORD64 offset; // if 0, we have no valid entry
	CHAR    name[STACKWALK_MAX_NAMELEN];
	CHAR    undName[STACKWALK_MAX_NAMELEN];
	CHAR    undFullName[STACKWALK_MAX_NAMELEN];
	DWORD64 offsetFromSmybol;
	DWORD   offsetFromLine;
	DWORD   lineNumber;
	CHAR    lineFileName[STACKWALK_MAX_NAMELEN];
	DWORD   symType;
	LPCSTR  symTypeString;
	CHAR    moduleName[STACKWALK_MAX_NAMELEN];
	DWORD64 baseOfImage;
	CHAR    loadedImageName[STACKWALK_MAX_NAMELEN];
} CallstackEntry;

TCHAR szStacktrace [2048];

void OnCallstackEntry(CallstackEntry& entry)
{
	sprintf(
		szStacktrace
		,"%s\n%s (%d): %s"
		,g_AssertInfo.szMessage
		,entry.lineFileName
		,entry.lineNumber
		,entry.name
	);
	g_AssertInfo.szMessage = szStacktrace;
}

void onError(const char* func, STACKFRAME64 s, char buf[STACKWALK_MAX_NAMELEN])
{
	sprintf(
		szStacktrace
		,"%s\nERROR: %s, GetLastError: %d (Address: %p)\nSearch Path: %s\n"
		,g_AssertInfo.szMessage
		,func
		,GetLastError()
		,s.AddrPC.Offset
		,buf
	);
	g_AssertInfo.szMessage = szStacktrace;
}
void printCallStack()
{
	HANDLE hThread = GetCurrentThread();
	HANDLE hProcess = GetCurrentProcess();

	IMAGEHLP_LINE64 Line;
	memset(&Line, 0, sizeof(Line));
	Line.SizeOfStruct = sizeof(Line);

	CONTEXT c;
	memset(&c, 0, sizeof(CONTEXT));
	RtlCaptureContext(&c);

	STACKFRAME64 s;
	memset(&s, 0, sizeof(s));
	s.AddrPC.Offset = c.Eip;
	s.AddrPC.Mode = AddrModeFlat;
	s.AddrFrame.Offset = c.Ebp;
	s.AddrFrame.Mode = AddrModeFlat;
	s.AddrStack.Offset = c.Esp;
	s.AddrStack.Mode = AddrModeFlat;

	SymSetOptions(SYMOPT_UNDNAME | SYMOPT_LOAD_LINES);
	SymInitialize(hProcess, NULL, TRUE);
	char buf[STACKWALK_MAX_NAMELEN] = {0};
      	if (!SymGetSearchPath(hProcess, buf, STACKWALK_MAX_NAMELEN))
	{
		onError("SymGetSearchPath",s,buf);
		return;
	}

	if (!SymLoadModuleEx(hProcess,    	// target process 
		            NULL,        	// handle to image - not used
		            "CvGameCoreDLL.dll",// name of image file
		            NULL,        	// name of module - not required
		            0,		 	// base address - not required
		            0,           	// size of image - not required
		            NULL,        	// MODLOAD_DATA used for special cases 
		            0))          	// flags - not required
	{
		onError("SymLoadModuleEx",s,buf);
		return;
	}

	for (int frameNum = 0;; ++frameNum)
	{
		if (!StackWalk64(IMAGE_FILE_MACHINE_I386,hProcess,hThread,&s,&c,NULL,SymFunctionTableAccess64,SymGetModuleBase64,NULL ) )
		{
			// Maybe it failed, maybe we have finished walking the stack.
			break;
		}

		CallstackEntry csEntry;
		if (s.AddrPC.Offset != 0)
		{
			csEntry.offset = s.AddrPC.Offset;
			csEntry.name[0] = 0;
			csEntry.undName[0] = 0;
			csEntry.undFullName[0] = 0;
			csEntry.offsetFromSmybol = 0;
			csEntry.offsetFromLine = 0;
			csEntry.lineFileName[0] = 0;
			csEntry.lineNumber = 0;
			csEntry.loadedImageName[0] = 0;
			csEntry.moduleName[0] = 0;

			// show line number info, NT5.0-method (SymGetLineFromAddr64())
			if(SymGetLineFromAddr64(hProcess, s.AddrPC.Offset, &(csEntry.offsetFromLine),&Line) != FALSE)
			{
				csEntry.lineNumber = Line.LineNumber;
				strncpy(csEntry.lineFileName, Line.FileName, STACKWALK_MAX_NAMELEN);
			}
			else
			{
				//This means its end of stack for which we have debug symbols
				//onError("SymGetLineFromAddr64",s,buf);
				return;

			}

			OnCallstackEntry(csEntry);
			// show module info (SymGetModuleInfo64())
		}
		else
		{
			OnCallstackEntry(csEntry);
			break;
		}
	}
}


bool FAssertDlg( const char* szExpr, const char* szMsg, const char*szFile, unsigned int line, bool& bIgnoreAlways )
{
//	FILL_CONTEXT( g_AssertInfo.context );

	g_AssertInfo.szExpression = szExpr;
	g_AssertInfo.szMessage = szMsg;
	g_AssertInfo.szFileName = szFile;
	g_AssertInfo.line = line;

	printCallStack();

	DWORD dwResult = DisplayAssertDialog();

	switch( dwResult )
	{
	case ASSERT_DLG_DEBUG:
		return true;

	case ASSERT_DLG_IGNORE:
		return false;

	case ASSERT_DLG_IGNOREALWAYS:
		bIgnoreAlways = true;
		return false;

	case ASSERT_DLG_EXIT:
		exit(0);
		break;
	}

	return true;
}

#endif // FASSERT_ENABLE && WIN32
