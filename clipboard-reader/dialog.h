// Copyright (c) Andrew Butson.
// Licensed under the MIT License.

#pragma once

#define	WM_ICON_ANIMATE_START (WM_APP + 1)

#define AWHK_RUN_KEY			L"Software\\Microsoft\\Windows\\CurrentVersion\\Run"
#define AWHK_RUN_ENTRY_NAME		L"arb.clipboard.reader"

class CMainDlg final : public CDialogImpl<CMainDlg>, public CUpdateUI<CMainDlg>,
                       public CMessageFilter, public CIdleHandler
{
public:
	enum { IDD = IDD_MAINDLG };

	auto PreTranslateMessage(MSG* p_msg) -> BOOL override;
	auto OnIdle() -> BOOL override;
	auto OnAutorunClick(const WORD w_notify_code, const WORD w_id, const HWND h_wnd_ctl, BOOL& b_handled) -> LRESULT;
	auto OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*b_handled*/) -> LRESULT;
	auto OnOK(WORD /*wNotifyCode*/, WORD w_id, HWND /*hWndCtl*/, BOOL& /*b_handled*/) -> LRESULT;
	auto OnAbout(WORD /*wNotifyCode*/, WORD /*w_id*/, HWND /*hWndCtl*/, BOOL& /*b_handled*/) -> LRESULT;
	auto OnExit(WORD /*wNotifyCode*/, const WORD w_id, HWND /*hWndCtl*/, BOOL& /*b_handled*/) -> LRESULT;
	auto OnTimer(UINT /*uMsg*/, WPARAM w_param, LPARAM /*lParam*/, BOOL& b_handled) -> LRESULT;
	auto OnHotkey(UINT /*uMsg*/, const WPARAM w_param, LPARAM /*lParam*/, BOOL& b_handled) -> LRESULT;
	auto OnSize(UINT /*uMsg*/, const WPARAM w_param, LPARAM /*lParam*/, BOOL& b_handled) -> LRESULT;
	auto OnTaskIconClick(LPARAM /*uMsg*/, BOOL& /*b_handled*/) -> LRESULT;
	auto OnInitDialog(UINT /*uMsg*/, WPARAM /*w_param*/, LPARAM /*lParam*/, BOOL& /*b_handled*/) -> LRESULT;
	auto CloseDialog(const int n_val) -> void;

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
		CHAIN_MSG_MAP_MEMBER(taskbar_icon_)
		TASKBAR_MESSAGE_HANDLER(taskbar_icon_, WM_LBUTTONDOWN, OnTaskIconClick)
		COMMAND_HANDLER(IDC_AUTORUN, BN_CLICKED, OnAutorunClick)
	END_MSG_MAP()

private:
	//used in OnHotkey
	ATOM hot_key_{};
	DWORD previous_tick_ = 0;
	DWORD current_tick_ = 0;
	int hotkey_is_valid_ = 0;
	bool reg_autorun_ = false;

	std::vector<HICON> animate_icons_;
	int animate_item_ = 0;

	CTaskBarIcon taskbar_icon_;
	std::queue<std::string> messages_queue_;

	auto GetNextMessage() -> std::string;
	auto UpdateTaskbarTooltip() -> LRESULT;
	auto RegisterHotkey() -> LRESULT;
	auto ReadAndQueueClipboard() -> LRESULT;
	static auto CloneIcon(const HICON original_icon) -> HICON;
	auto IsAutorunEnabled() -> bool;
	auto DisableAutorun() -> BOOL;
	auto EnableAutorun() -> BOOL;
};
