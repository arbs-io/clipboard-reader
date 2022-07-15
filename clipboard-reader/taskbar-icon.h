// Copyright (c) Andrew Butson.
// Licensed under the MIT License.

#pragma once

#define TASKBAR_MESSAGE_HANDLER(ti, msg, func) \
   if(uMsg==(ti).notify_icon_.uCallbackMessage && wParam==(ti).notify_icon_.uID && lParam==(msg)) \
   { \
      bHandled = TRUE; \
      lResult = func(lParam, bHandled); \
      if(bHandled) \
         return TRUE; \
   }

class CTaskBarIcon
{
public:
    NOTIFYICONDATA notify_icon_{};

	CTaskBarIcon();
	~CTaskBarIcon();

	BEGIN_MSG_MAP(CTaskBarIcon)
	   MESSAGE_HANDLER(taskbar_msg_, OnTaskbarRestart)
	  TASKBAR_MESSAGE_HANDLER((*this), WM_RBUTTONDOWN, OnTaskbarContextMenu)
	END_MSG_MAP()

	auto Install(const HWND h_wnd, const UINT i_id, const HICON h_icon, const HMENU h_menu,
	            const LPTSTR lpsz_tip = nullptr) -> BOOL;
	auto Install(const HWND h_wnd, const UINT i_id, const UINT n_res) -> BOOL;
	auto ChangeIcon(const HICON h_icon)->BOOL;
	auto ChangeTooltip(const LPTSTR lpsz_tip = nullptr)->BOOL;

private:    
	HMENU h_menu_;
	UINT taskbar_msg_;

	auto Uninstall() -> BOOL;
	auto IsInstalled() const -> BOOL;
	auto SetIcon(const HICON h_icon) -> void;
	auto SetMenu(const HMENU h_menu) -> void;
	auto AddTaskBarIcon() -> BOOL;
	auto DeleteTaskBarIcon() -> BOOL;   
	auto OnTaskbarRestart(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& b_handled) -> LRESULT;
	auto OnTaskbarContextMenu(LPARAM /*uMsg*/, BOOL& b_handled) -> LRESULT;
};
