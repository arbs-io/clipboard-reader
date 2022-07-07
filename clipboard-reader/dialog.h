// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <locale>
#include <queue>

#include "taskbar-icon.h"
#include "speech.h"

#define	WM_ICON_ANIMATE_START (WM_APP + 1)

#define AWHK_RUN_KEY			L"Software\\Microsoft\\Windows\\CurrentVersion\\Run"
#define AWHK_RUN_ENTRY_NAME		L"arb.clipboard.reader"

class CMainDlg final : public CDialogImpl<CMainDlg>, public CUpdateUI<CMainDlg>,
                       public CMessageFilter, public CIdleHandler
{
private:
	//used in OnHotkey
	ATOM m_hotKey{};
	DWORD m_dwOldTick = 0;
	DWORD m_dwCurrentTick = 0;
	int m_hotkeyValid = 0;
	bool m_autorun = false;

	std::vector<HICON> m_animate_icons;
	int m_animate_item = 0;

	CTaskBarIcon m_ti;
	std::queue<std::string> m_messages;
		
public:
	enum { IDD = IDD_MAINDLG };

	BOOL PreTranslateMessage(MSG* pMsg) override
	{
		return CWindow::IsDialogMessage(pMsg);
	}

	BOOL OnIdle() override
	{
		UIUpdateChildWindows();
		return FALSE;
	}

	BEGIN_UPDATE_UI_MAP(CMainDlg)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_HOTKEY, OnHotkey)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)				
		COMMAND_ID_HANDLER(ID_TASKBAR_EXIT, OnExit)
		COMMAND_ID_HANDLER(ID_ABOUT, OnAbout)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		CHAIN_MSG_MAP_MEMBER(m_ti)
		TASKBAR_MESSAGE_HANDLER(m_ti, WM_LBUTTONDOWN, OnTaskIconClick)
		COMMAND_HANDLER(IDC_AUTORUN, BN_CLICKED, OnAutorunClick)		
	END_MSG_MAP()


	LRESULT OnAutorunClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{		
		m_autorun = IsDlgButtonChecked(IDC_AUTORUN) == BST_CHECKED;
		
		if (m_autorun)
			EnableAutorun();
		else
			DisableAutorun();

		AtlTrace("wNotifyCode: %i, wID: %i, hWndCtl: %i, m_autorun: %i\n", wNotifyCode, wID, hWndCtl, m_autorun);
		return 0;
	}
		
// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{		
		// unregister message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->RemoveMessageFilter(this);
		pLoop->RemoveIdleHandler(this);

		return 0;
	}

	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		ShowWindow(SW_HIDE);
		return 0;
	}

	LRESULT OnAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		ShowWindow(SW_SHOW);
		return TRUE;
	}
	LRESULT OnExit(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CloseDialog(wID);
		return TRUE;
	}

	LRESULT OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if (m_animate_item >= m_animate_icons.size())
			m_animate_item = 0;

		auto icn = CloneIcon(m_animate_icons[m_animate_item]);

		m_ti.ChangeIcon(icn);
		m_animate_item++;

		return TRUE;
	}

	LRESULT OnHotkey(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if (wParam == m_hotKey)
		{
			m_dwCurrentTick = GetTickCount();
			if (m_dwCurrentTick - m_dwOldTick <= 350)
				m_hotkeyValid++;
			else
				m_hotkeyValid = 0;

			if (m_hotkeyValid == 2)
				ReadAndQueueClipboard();

			m_dwOldTick = m_dwCurrentTick;
		}

		bHandled = FALSE;
		return TRUE;
	}


	LRESULT OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if (wParam == SIZE_MINIMIZED) ShowWindow(SW_HIDE);
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnTaskIconClick(LPARAM /*uMsg*/, BOOL& /*bHandled*/)
	{
		ShowWindow(SW_SHOW);
		return TRUE;
	}

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// center the dialog on the screen
		CenterWindow();

		m_autorun = IsAutorunEnabled();
		CheckDlgButton(IDC_AUTORUN, m_autorun ? BST_CHECKED : BST_UNCHECKED);
		

		//Install taskbar
		HICON hAnimateIcon = AtlLoadIconImage(IDI_ICON1, LR_DEFAULTCOLOR, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON));
		m_animate_icons.push_back(hAnimateIcon);
		hAnimateIcon = AtlLoadIconImage(IDI_ICON2, LR_DEFAULTCOLOR, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON));
		m_animate_icons.push_back(hAnimateIcon);
		hAnimateIcon = AtlLoadIconImage(IDI_ICON3, LR_DEFAULTCOLOR, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON));
		m_animate_icons.push_back(hAnimateIcon);
		hAnimateIcon = AtlLoadIconImage(IDI_ICON4, LR_DEFAULTCOLOR, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON));
		m_animate_icons.push_back(hAnimateIcon);
		m_ti.Install(m_hWnd, 1, IDR_TASKBAR);
				

		// set icons
		HICON hIcon = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON));
		SetIcon(hIcon, TRUE);
		HICON hIconSmall = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, GetSystemMetrics(SM_CXSMICON),
		                                    GetSystemMetrics(SM_CYSMICON));
		SetIcon(hIconSmall, FALSE);

		// register object for message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->AddMessageFilter(this);
		pLoop->AddIdleHandler(this);

		UIAddChildWindowContainer(m_hWnd);

		RegisterHotkey();
		
		//Fire and forget backgroup thread to read
		std::thread([this]()
		{
			bool reset = false;
			while (true)
			{
				while (!m_messages.empty())
				{
					SetTimer(WM_ICON_ANIMATE_START, 500, nullptr);

					auto msg = GetNextMessage();
					CSpeech spk(msg);
					spk.TextToSpeech();

					KillTimer(WM_ICON_ANIMATE_START);
					reset = true;
				}
				Sleep(500);
				if (reset == true)
				{
					HICON icn = AtlLoadIconImage(IDR_TASKBAR, LR_DEFAULTCOLOR, GetSystemMetrics(SM_CXICON),
					                             GetSystemMetrics(SM_CYICON));
					m_ti.ChangeIcon(icn);
					reset = false;
				}

			}
		}).detach();

		return TRUE;
	}
	
	void CloseDialog(int nVal)
	{
		DestroyWindow();
		PostQuitMessage(nVal);
	}


private:
	std::string GetNextMessage()
	{
		auto msg = m_messages.front();
		m_messages.pop();

		UpdateTaskbarTooltip();
		
		return msg;
	}

	LRESULT UpdateTaskbarTooltip()
	{
		std::wstring tskbar = L"Items Queued: " + std::to_wstring(m_messages.size());
		m_ti.ChangeTooltip(const_cast<LPTSTR>(tskbar.c_str()));
	
		auto dbgstr = tskbar + L"\n";
		OutputDebugString(dbgstr.c_str());

		return TRUE;
	}

	LRESULT RegisterHotkey()
	{
		m_hotKey = GlobalAddAtom(L"ReadAssistAtom");
		if (m_hotKey)
		{
			RegisterHotKey(m_hWnd, m_hotKey, MOD_CONTROL, 0);
		}

		return TRUE;
	}

	LRESULT ReadAndQueueClipboard()
	{
		std::string fromClipboard;
		if (OpenClipboard())
		{
			HANDLE hData = GetClipboardData(CF_TEXT);
			auto buffer = (char*)GlobalLock(hData);
			fromClipboard = buffer;
			GlobalUnlock(hData);
			CloseClipboard();
		}

		m_messages.push(fromClipboard);
		UpdateTaskbarTooltip();

		return TRUE;
	}

	HICON CloneIcon(HICON OriginalIcon)
	{
		return DuplicateIcon(nullptr, OriginalIcon); //first parameter is unused
	}

	bool IsAutorunEnabled()
	{
		HKEY hKey;
		if (::RegOpenKeyEx(HKEY_CURRENT_USER, AWHK_RUN_KEY, 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
			return false;
		
		if(::RegQueryValueEx(hKey, AWHK_RUN_ENTRY_NAME, nullptr, nullptr, nullptr, nullptr) == ERROR_FILE_NOT_FOUND)
			return false;

		RegCloseKey(hKey);
		return true;
	}

	BOOL DisableAutorun()
	{
		HKEY hKey;
		if (::RegOpenKeyEx(HKEY_CURRENT_USER, AWHK_RUN_KEY, 0, KEY_SET_VALUE, &hKey) != ERROR_SUCCESS)
			return FALSE;

		::RegDeleteValue(hKey, AWHK_RUN_ENTRY_NAME);

		return ERROR_SUCCESS;
	}
	BOOL EnableAutorun()
	{
		//Auto startup
		TCHAR szStartupPath[_MAX_PATH + 1];
		GetModuleFileName(nullptr, szStartupPath, MAX_PATH);
		std::wstring regvalue(&szStartupPath[0]);
		regvalue = L"\"" + regvalue + L"\"";

		HKEY hKey;
		if (::RegOpenKeyEx(
			HKEY_CURRENT_USER,
			AWHK_RUN_KEY,
			0,
			KEY_SET_VALUE,
			&hKey) != ERROR_SUCCESS)
		{
			return FALSE;
		}

		LSTATUS ret;
		ret = ::RegSetValueEx(
			hKey,
			AWHK_RUN_ENTRY_NAME,
			0,
			REG_SZ,
			(BYTE*)regvalue.c_str(),
			(DWORD)regvalue.size() * 2);

		RegCloseKey(hKey);

		return ERROR_SUCCESS;
	}
};
