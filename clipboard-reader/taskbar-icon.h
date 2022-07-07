#pragma once

#include <string>

#define TASKBAR_MESSAGE_HANDLER(ti, msg, func) \
   if(uMsg==(ti).m_nid.uCallbackMessage && wParam==(ti).m_nid.uID && lParam==(msg)) \
   { \
      bHandled = TRUE; \
      lResult = func(lParam, bHandled); \
      if(bHandled) \
         return TRUE; \
   }


class CTaskBarIcon
{
public:
   NOTIFYICONDATA m_nid{}; 
   HMENU m_h_menu;
   UINT m_iTaskbarRestartMsg;

   CTaskBarIcon() : m_h_menu(nullptr)
   {
      ::ZeroMemory(&m_nid, sizeof m_nid);
      m_nid.cbSize = sizeof m_nid; 
      m_nid.uCallbackMessage = ::RegisterWindowMessage(TEXT("TaskbarNotifyMsg"));
      m_iTaskbarRestartMsg = ::RegisterWindowMessage(TEXT("TaskbarCreated"));
   }
   ~CTaskBarIcon()
   {
      Uninstall();
   }

   BEGIN_MSG_MAP(CTaskBarIcon)
      MESSAGE_HANDLER(m_iTaskbarRestartMsg, OnTaskbarRestart)
      TASKBAR_MESSAGE_HANDLER((*this), WM_RBUTTONDOWN, OnTaskbarContextMenu)
   END_MSG_MAP()

   BOOL Install(const HWND h_wnd, const UINT i_id, const HICON h_icon, const HMENU h_menu, const LPTSTR lpsz_tip = nullptr)
   {
      ATLASSERT(::IsWindow(h_wnd));
      ATLASSERT(m_h_menu==NULL);
      ATLASSERT(m_nid.hIcon==NULL);
      m_nid.hWnd = h_wnd;
      m_nid.uID = i_id;
      m_nid.hIcon = h_icon; 
      ::lstrcpyn(m_nid.szTip, lpsz_tip != nullptr ? lpsz_tip : _T(""), sizeof m_nid.szTip/sizeof(TCHAR)); 
      m_h_menu = h_menu;
      return AddTaskBarIcon();
   }
   BOOL Install(const HWND h_wnd, const UINT i_id, const UINT n_res)
   {
      ATLASSERT(::IsWindow(h_wnd));
      ATLASSERT(m_h_menu==NULL);
      ATLASSERT(m_nid.hIcon==NULL);
      m_nid.hWnd = h_wnd;
      m_nid.uID = i_id;
      m_nid.hIcon = static_cast<HICON>(::LoadImage(app_module.GetResourceInstance(), MAKEINTRESOURCE(n_res), IMAGE_ICON,
                                                   GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR));
      m_nid.szTip[0] = '\0';
      ::LoadString(app_module.GetResourceInstance(), n_res, m_nid.szTip, sizeof m_nid.szTip/sizeof(TCHAR));
      m_h_menu = ::LoadMenu(app_module.GetResourceInstance(), MAKEINTRESOURCE(n_res));
      return AddTaskBarIcon();
   }
   BOOL Uninstall()
   {
      BOOL res = TRUE;
      if( m_nid.hWnd != nullptr) res = DeleteTaskBarIcon();
      m_nid.hWnd = nullptr;
      if( m_nid.hIcon != nullptr) DestroyIcon(m_nid.hIcon);
      m_nid.hIcon = nullptr;
      if( m_h_menu != nullptr) DestroyMenu(m_h_menu);
      m_h_menu = nullptr;
      return res;
   }
   BOOL IsInstalled() const
   {
      return m_nid.hWnd!= nullptr;
   }
   void SetIcon(const HICON h_icon) 
   { 
      ATLASSERT(m_nid.hIcon==NULL);
      m_nid.hIcon = h_icon; 
   }
   void SetMenu(const HMENU h_menu) 
   { 
      ATLASSERT(m_h_menu==NULL);
      ATLASSERT(::IsMenu(h_menu));
      m_h_menu = h_menu; 
   }
   BOOL AddTaskBarIcon()
   {
      ATLASSERT(::IsWindow(m_nid.hWnd));
      m_nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
      const BOOL res = ::Shell_NotifyIcon(NIM_ADD, &m_nid); 
      return res;
   }
   BOOL ChangeIcon(const HICON h_icon)
   {
	   // NOTE: The class takes ownership of the icon!
	   ATLASSERT(::IsWindow(m_nid.hWnd));
	   if (m_nid.hIcon != nullptr) DestroyIcon(m_nid.hIcon);
	   m_nid.uFlags = NIF_ICON;
	   m_nid.hIcon = h_icon;
	   const BOOL res = ::Shell_NotifyIcon(NIM_MODIFY, &m_nid);
	   return res;
   }
   BOOL ChangeTooltip(const LPTSTR lpsz_tip = nullptr)
   {
	   // NOTE: The class takes ownership of the icon!
	   ATLASSERT(::IsWindow(m_nid.hWnd));
	   m_nid.uFlags = NIF_TIP;
	   ::lstrcpyn(m_nid.szTip, lpsz_tip != nullptr ? lpsz_tip : _T(""), sizeof m_nid.szTip / sizeof(TCHAR));
	   const BOOL res = ::Shell_NotifyIcon(NIM_MODIFY, &m_nid);
	   return res;
   }
   BOOL DeleteTaskBarIcon() 
   { 
       return ::Shell_NotifyIcon(NIM_DELETE, &m_nid); 
   } 

   // Message handlers

   LRESULT OnTaskbarRestart(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& b_handled)
   {
      AddTaskBarIcon();
      b_handled = FALSE;
      return 0;
   }
   LRESULT OnTaskbarContextMenu(LPARAM /*uMsg*/, BOOL& b_handled)
   {
      if( !IsMenu(m_h_menu) ) {
         b_handled = FALSE;
         return 0;
      }
      const auto h_sub_menu = GetSubMenu(m_h_menu, 0);   
      ATLASSERT(::IsMenu(h_sub_menu));
      // Make first menu-item the default (bold font)
      SetMenuDefaultItem(h_sub_menu, 0, TRUE); 
      // Display the menu at the current mouse location.
      POINT pt;
      pt = {0};
      GetCursorPos(&pt);
      SetForegroundWindow(m_nid.hWnd);       // Fixes Win95 bug; see Q135788
      TrackPopupMenu(h_sub_menu, 0, pt.x, pt.y, 0, m_nid.hWnd, nullptr);
      ::PostMessage(m_nid.hWnd, WM_NULL, 0,0); // Fixes another Win95 bug
      return 0;
   }
};

