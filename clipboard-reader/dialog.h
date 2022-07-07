// Copyright (c) Andrew Butson.
// Licensed under the MIT License.

#pragma once

#include <locale>
#include <queue>

#include "taskbar-icon.h"
#include "speech.h"

#define	WM_ICON_ANIMATE_START (WM_APP + 1)

#define AWHK_RUN_KEY			L"Software\\Microsoft\\Windows\\CurrentVersion\\Run"
#define AWHK_RUN_ENTRY_NAME		L"arb.clipboard.reader"

class c_main_dlg final : public CDialogImpl<c_main_dlg>, public CUpdateUI<c_main_dlg>,
                       public CMessageFilter, public CIdleHandler
{
	//used in OnHotkey
	ATOM m_hot_key_{};
	DWORD m_dw_old_tick_ = 0;
	DWORD m_dw_current_tick_ = 0;
	int m_hotkeyValid = 0;
	bool m_autorun_ = false;

	std::vector<HICON> m_animate_icons_;
	int m_animate_item_ = 0;

	CTaskBarIcon m_ti_;
	std::queue<std::string> m_messages_;
		
public:
	enum { IDD = IDD_MAINDLG };

	BOOL PreTranslateMessage(MSG* p_msg) override
	{
		return CWindow::IsDialogMessage(p_msg);
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
		CHAIN_MSG_MAP_MEMBER(m_ti_)
		TASKBAR_MESSAGE_HANDLER(m_ti_, WM_LBUTTONDOWN, OnTaskIconClick)
		COMMAND_HANDLER(IDC_AUTORUN, BN_CLICKED, OnAutorunClick)		
	END_MSG_MAP()


	LRESULT OnAutorunClick(const WORD w_notify_code, const WORD w_id, const HWND h_wnd_ctl, BOOL& b_handled)
	{		
		m_autorun_ = IsDlgButtonChecked(IDC_AUTORUN) == BST_CHECKED;
		
		if (m_autorun_)
			EnableAutorun();
		else
			DisableAutorun();

		AtlTrace("wNotifyCode: %i, w_id: %i, hWndCtl: %i, m_autorun: %i\n", w_notify_code, w_id, h_wnd_ctl, m_autorun_);
		return 0;
	}
		
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*b_handled*/)
	{		
		// unregister message filtering and idle updates
		CMessageLoop* p_loop = app_module.GetMessageLoop();
		ATLASSERT(p_loop != NULL);
		p_loop->RemoveMessageFilter(this);
		p_loop->RemoveIdleHandler(this);

		return 0;
	}

	LRESULT OnOK(WORD /*wNotifyCode*/, WORD w_id, HWND /*hWndCtl*/, BOOL& /*b_handled*/)
	{
		ShowWindow(SW_HIDE);
		return 0;
	}

	LRESULT OnAbout(WORD /*wNotifyCode*/, WORD /*w_id*/, HWND /*hWndCtl*/, BOOL& /*b_handled*/)
	{
		ShowWindow(SW_SHOW);
		return TRUE;
	}
	LRESULT OnExit(WORD /*wNotifyCode*/, const WORD w_id, HWND /*hWndCtl*/, BOOL& /*b_handled*/)
	{
		CloseDialog(w_id);
		return TRUE;
	}

	LRESULT OnTimer(UINT /*uMsg*/, WPARAM w_param, LPARAM /*lParam*/, BOOL& b_handled)
	{
		if (m_animate_item_ >= m_animate_icons_.size())
			m_animate_item_ = 0;

		const auto icn = CloneIcon(m_animate_icons_[m_animate_item_]);

		m_ti_.ChangeIcon(icn);
		m_animate_item_++;

		return TRUE;
	}

	LRESULT OnHotkey(UINT /*uMsg*/, const WPARAM w_param, LPARAM /*lParam*/, BOOL& b_handled)
	{
		if (w_param == m_hot_key_)
		{
			m_dw_current_tick_ = GetTickCount();
			if (m_dw_current_tick_ - m_dw_old_tick_ <= 350)
				m_hotkeyValid++;
			else
				m_hotkeyValid = 0;

			if (m_hotkeyValid == 2)
				ReadAndQueueClipboard();

			m_dw_old_tick_ = m_dw_current_tick_;
		}

		b_handled = FALSE;
		return TRUE;
	}


	LRESULT OnSize(UINT /*uMsg*/, const WPARAM w_param, LPARAM /*lParam*/, BOOL& b_handled)
	{
		if (w_param == SIZE_MINIMIZED) ShowWindow(SW_HIDE);
		b_handled = FALSE;
		return 0;
	}

	LRESULT OnTaskIconClick(LPARAM /*uMsg*/, BOOL& /*b_handled*/)
	{
		ShowWindow(SW_SHOW);
		return TRUE;
	}

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*w_param*/, LPARAM /*lParam*/, BOOL& /*b_handled*/)
	{
		// center the dialog on the screen
		CenterWindow();

		m_autorun_ = IsAutorunEnabled();
		CheckDlgButton(IDC_AUTORUN, m_autorun_ ? BST_CHECKED : BST_UNCHECKED);
		

		//Install taskbar
		HICON h_animate_icon = AtlLoadIconImage(IDI_ICON1, LR_DEFAULTCOLOR, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON));
		m_animate_icons_.push_back(h_animate_icon);
		h_animate_icon = AtlLoadIconImage(IDI_ICON2, LR_DEFAULTCOLOR, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON));
		m_animate_icons_.push_back(h_animate_icon);
		h_animate_icon = AtlLoadIconImage(IDI_ICON3, LR_DEFAULTCOLOR, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON));
		m_animate_icons_.push_back(h_animate_icon);
		h_animate_icon = AtlLoadIconImage(IDI_ICON4, LR_DEFAULTCOLOR, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON));
		m_animate_icons_.push_back(h_animate_icon);
		m_ti_.Install(m_hWnd, 1, IDR_TASKBAR);
				

		// set icons
		const auto h_icon = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON));
		SetIcon(h_icon, TRUE);
		const auto h_icon_small = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, GetSystemMetrics(SM_CXSMICON),
		                                          GetSystemMetrics(SM_CYSMICON));
		SetIcon(h_icon_small, FALSE);

		// register object for message filtering and idle updates
		CMessageLoop* pLoop = app_module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->AddMessageFilter(this);
		pLoop->AddIdleHandler(this);

		UIAddChildWindowContainer(m_hWnd);

		RegisterHotkey();
		
		//Fire and forget backgroup thread to read
		std::thread([this]
		{
			bool reset = false;
			while (true)
			{
				while (!m_messages_.empty())
				{
					SetTimer(WM_ICON_ANIMATE_START, 500, nullptr);

					const auto msg = GetNextMessage();
					CSpeech spk(msg);
					spk.TextToSpeech();

					KillTimer(WM_ICON_ANIMATE_START);
					reset = true;
				}
				Sleep(500);
				if (reset == true)
				{
					const auto icn = AtlLoadIconImage(IDR_TASKBAR, LR_DEFAULTCOLOR, GetSystemMetrics(SM_CXICON),
					                                   GetSystemMetrics(SM_CYICON));
					m_ti_.ChangeIcon(icn);
					reset = false;
				}

			}
		}).detach();

		return TRUE;
	}
	
	void CloseDialog(const int n_val)
	{
		DestroyWindow();
		PostQuitMessage(n_val);
	}


private:
	std::string GetNextMessage()
	{
		auto msg = m_messages_.front();
		m_messages_.pop();

		UpdateTaskbarTooltip();
		
		return msg;
	}

	LRESULT UpdateTaskbarTooltip()
	{
		const std::wstring tskbar = L"Items Queued: " + std::to_wstring(m_messages_.size());
		m_ti_.ChangeTooltip(const_cast<LPTSTR>(tskbar.c_str()));

		const auto dbgstr = tskbar + L"\n";
		OutputDebugString(dbgstr.c_str());

		return TRUE;
	}

	LRESULT RegisterHotkey()
	{
		m_hot_key_ = GlobalAddAtom(L"ReadAssistAtom");
		if (m_hot_key_)
		{
			RegisterHotKey(m_hWnd, m_hot_key_, MOD_CONTROL, 0);
		}

		return TRUE;
	}

	LRESULT ReadAndQueueClipboard()
	{
		std::string fromClipboard;
		if (OpenClipboard())
		{
			const auto h_data = GetClipboardData(CF_TEXT);
			const auto buffer = static_cast<char*>(GlobalLock(h_data));
			fromClipboard = buffer;
			GlobalUnlock(h_data);
			CloseClipboard();
		}

		m_messages_.push(fromClipboard);
		UpdateTaskbarTooltip();

		return TRUE;
	}

	static HICON CloneIcon(const HICON original_icon)
	{
		return DuplicateIcon(nullptr, original_icon); //first parameter is unused
	}

	bool IsAutorunEnabled()
	{
		HKEY h_key;
		if (::RegOpenKeyEx(HKEY_CURRENT_USER, AWHK_RUN_KEY, 0, KEY_QUERY_VALUE, &h_key) != ERROR_SUCCESS)
			return false;
		
		if(::RegQueryValueEx(h_key, AWHK_RUN_ENTRY_NAME, nullptr, nullptr, nullptr, nullptr) == ERROR_FILE_NOT_FOUND)
			return false;

		RegCloseKey(h_key);
		return true;
	}

	BOOL DisableAutorun()
	{
		HKEY h_key;
		if (::RegOpenKeyEx(HKEY_CURRENT_USER, AWHK_RUN_KEY, 0, KEY_SET_VALUE, &h_key) != ERROR_SUCCESS)
			return FALSE;

		::RegDeleteValue(h_key, AWHK_RUN_ENTRY_NAME);

		return ERROR_SUCCESS;
	}
	BOOL EnableAutorun()
	{
		//Auto startup
		TCHAR szStartupPath[_MAX_PATH + 1];
		GetModuleFileName(nullptr, szStartupPath, MAX_PATH);
		std::wstring regvalue(&szStartupPath[0]);
		regvalue = L"\"" + regvalue + L"\"";

		HKEY h_key;
		if (::RegOpenKeyEx(
			HKEY_CURRENT_USER,
			AWHK_RUN_KEY,
			0,
			KEY_SET_VALUE,
			&h_key) != ERROR_SUCCESS)
		{
			return FALSE;
		}

		LSTATUS ret = ::RegSetValueEx(
			h_key,
			AWHK_RUN_ENTRY_NAME,
			0,
			REG_SZ,
			reinterpret_cast<BYTE const*>(regvalue.c_str()),
			static_cast<DWORD>(regvalue.size()) * 2);
				
		RegCloseKey(h_key);

		return ERROR_SUCCESS;
	}
};
