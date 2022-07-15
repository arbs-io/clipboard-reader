// Copyright (c) Andrew Butson.
// Licensed under the MIT License.

#include "stdafx.h"


CTaskBarIcon::CTaskBarIcon() : h_menu_(nullptr)
{
	::ZeroMemory(&notify_icon_, sizeof notify_icon_);
	notify_icon_.cbSize = sizeof notify_icon_;
	notify_icon_.uCallbackMessage = ::RegisterWindowMessage(TEXT("TaskbarNotifyMsg"));
	taskbar_msg_ = ::RegisterWindowMessage(TEXT("TaskbarCreated"));
}

CTaskBarIcon::~CTaskBarIcon()
{
	Uninstall();
}

auto CTaskBarIcon::Install(const HWND h_wnd, const UINT i_id, const HICON h_icon, const HMENU h_menu,
	const LPTSTR lpsz_tip) -> BOOL
{
	ATLASSERT(::IsWindow(h_wnd));
	ATLASSERT(h_menu_ == NULL);
	ATLASSERT(notify_icon_.hIcon == NULL);
	notify_icon_.hWnd = h_wnd;
	notify_icon_.uID = i_id;
	notify_icon_.hIcon = h_icon;
	::lstrcpyn(notify_icon_.szTip, lpsz_tip != nullptr ? lpsz_tip : _T(""), sizeof notify_icon_.szTip / sizeof(TCHAR));
	h_menu_ = h_menu;
	return AddTaskBarIcon();
}

auto CTaskBarIcon::Install(const HWND h_wnd, const UINT i_id, const UINT n_res) -> BOOL
{
	ATLASSERT(::IsWindow(h_wnd));
	ATLASSERT(h_menu_ == NULL);
	ATLASSERT(notify_icon_.hIcon == NULL);
	notify_icon_.hWnd = h_wnd;
	notify_icon_.uID = i_id;
	notify_icon_.hIcon = static_cast<HICON>(::LoadImage(app_module.GetResourceInstance(), MAKEINTRESOURCE(n_res), IMAGE_ICON,
		GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR));
	notify_icon_.szTip[0] = '\0';
	::LoadString(app_module.GetResourceInstance(), n_res, notify_icon_.szTip, sizeof notify_icon_.szTip / sizeof(TCHAR));
	h_menu_ = ::LoadMenu(app_module.GetResourceInstance(), MAKEINTRESOURCE(n_res));
	return AddTaskBarIcon();
}

auto CTaskBarIcon::Uninstall() -> BOOL
{
	BOOL res = TRUE;
	if (notify_icon_.hWnd != nullptr) res = DeleteTaskBarIcon();
	notify_icon_.hWnd = nullptr;
	if (notify_icon_.hIcon != nullptr) DestroyIcon(notify_icon_.hIcon);
	notify_icon_.hIcon = nullptr;
	if (h_menu_ != nullptr) DestroyMenu(h_menu_);
	h_menu_ = nullptr;
	return res;
}

auto CTaskBarIcon::IsInstalled() const -> BOOL
{
	return notify_icon_.hWnd != nullptr;
}

auto CTaskBarIcon::SetIcon(const HICON h_icon) -> void
{
	ATLASSERT(notify_icon_.hIcon == NULL);
	notify_icon_.hIcon = h_icon;
}

auto CTaskBarIcon::SetMenu(const HMENU h_menu) -> void
{
	ATLASSERT(h_menu_ == NULL);
	ATLASSERT(::IsMenu(h_menu));
	h_menu_ = h_menu;
}

auto CTaskBarIcon::AddTaskBarIcon() -> BOOL
{
	ATLASSERT(::IsWindow(notify_icon_.hWnd));
	notify_icon_.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	const BOOL res = ::Shell_NotifyIcon(NIM_ADD, &notify_icon_);
	return res;
}

auto CTaskBarIcon::ChangeIcon(const HICON h_icon) -> BOOL
{
	// NOTE: The class takes ownership of the icon!
	ATLASSERT(::IsWindow(notify_icon_.hWnd));
	if (notify_icon_.hIcon != nullptr) DestroyIcon(notify_icon_.hIcon);
	notify_icon_.uFlags = NIF_ICON;
	notify_icon_.hIcon = h_icon;
	const BOOL res = ::Shell_NotifyIcon(NIM_MODIFY, &notify_icon_);
	return res;
}

auto CTaskBarIcon::ChangeTooltip(const LPTSTR lpsz_tip) -> BOOL
{
	// NOTE: The class takes ownership of the icon!
	ATLASSERT(::IsWindow(notify_icon_.hWnd));
	notify_icon_.uFlags = NIF_TIP;
	::lstrcpyn(notify_icon_.szTip, lpsz_tip != nullptr ? lpsz_tip : _T(""), sizeof notify_icon_.szTip / sizeof(TCHAR));
	const BOOL res = ::Shell_NotifyIcon(NIM_MODIFY, &notify_icon_);
	return res;
}

auto CTaskBarIcon::DeleteTaskBarIcon() -> BOOL
{
	return ::Shell_NotifyIcon(NIM_DELETE, &notify_icon_);
}

auto CTaskBarIcon::OnTaskbarRestart(UINT, WPARAM, LPARAM, BOOL& b_handled) -> LRESULT
{
	AddTaskBarIcon();
	b_handled = FALSE;
	return 0;
}

auto CTaskBarIcon::OnTaskbarContextMenu(LPARAM, BOOL& b_handled) -> LRESULT
{
	if (!IsMenu(h_menu_)) {
		b_handled = FALSE;
		return 0;
	}
	const auto h_sub_menu = GetSubMenu(h_menu_, 0);
	ATLASSERT(::IsMenu(h_sub_menu));
	// Make first menu-item the default (bold font)
	SetMenuDefaultItem(h_sub_menu, 0, TRUE);
	// Display the menu at the current mouse location.
	POINT pt;
	pt = { 0 };
	GetCursorPos(&pt);
	SetForegroundWindow(notify_icon_.hWnd);       // Fixes Win95 bug; see Q135788
	TrackPopupMenu(h_sub_menu, 0, pt.x, pt.y, 0, notify_icon_.hWnd, nullptr);
	::PostMessage(notify_icon_.hWnd, WM_NULL, 0, 0); // Fixes another Win95 bug
	return 0;
}

