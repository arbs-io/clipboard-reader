// clipboard-reader.cpp : main source file for clipboard-reader.exe
//

#include "stdafx.h"

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>

#include "resource.h"

#include "dialog.h"

CAppModule app_module;


HRESULT Run(LPTSTR /*lpstrCmdLine*/ = nullptr, const int n_cmd_show = SW_SHOWDEFAULT)
{
	CMessageLoop message_loop;
	app_module.AddMessageLoop(&message_loop);

	c_main_dlg dlg_main;

	if(dlg_main.Create(nullptr) == nullptr)
	{
		ATLTRACE(_T("Main dialog creation failed!\n"));
		return 0;
	}

	dlg_main.ShowWindow(n_cmd_show);

	const HRESULT results_handle = message_loop.Run();

	app_module.RemoveMessageLoop();
	return results_handle;
}

int WINAPI _tWinMain(const HINSTANCE h_instance, HINSTANCE /*hPrevInstance*/, const LPTSTR lpstr_cmd_line, const int n_show_cmd)
{
	HRESULT results_handle = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	ATLASSERT(SUCCEEDED(results_handle));

	//NOTE: This can be DOS attacked, but who would do that...
	//		Check if the application is already running (one instance only)
	::CreateMutex(nullptr, FALSE, L"Local\\arb.clipboard.reader");	// try to create a named mutex
	if (GetLastError() == ERROR_ALREADY_EXISTS)							// did the mutex already exist?
		return -1;														// quit; mutex is released automatically
		
	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(nullptr, 0, 0, 0L);

	AtlInitCommonControls(ICC_BAR_CLASSES);	// add flags to support other controls

	results_handle = app_module.Init(nullptr, h_instance);
	ATLASSERT(SUCCEEDED(results_handle));

	results_handle = Run(lpstr_cmd_line, n_show_cmd);

	app_module.Term();
	CoUninitialize();

	return results_handle;
}
