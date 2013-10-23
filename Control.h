#ifndef Control_h__2013_8_1__9_24
#define Control_h__2013_8_1__9_24

#include "../df/df.h"
#include <commctrl.h>

#ifdef _MSC_VER
#pragma comment(lib, "comctl32")
#pragma comment(lib, "Msimg32")
#pragma comment(lib, "gdiplus")
#endif // _MSC_VER



namespace sdf
{
	class Window;

	struct WinHandle;

	class Control
	{
	protected:
		HWND handle_;
		uint resourceID_;

		friend class Tray;
		friend struct WinHandle;
		//最近一次创建的窗口,用于OnInit
		static THREAD_LOCAL_VAR HWND currentHandle_;
		static Window * currentWindow_;

		//按钮焦点句柄
		static HWND mouseHandle_;
		static LONG_PTR buttonPro_;

		void InitUserData() const
		{
			::SetWindowLongPtr(handle_, GWLP_USERDATA, (LONG_PTR)this);
		}
	public:

		Control()
			: handle_(NULL)
			, resourceID_(0)
		{

		}

		Control(Control && r)
		{
			handle_ = r.handle_;
			resourceID_ = r.resourceID_;
			r.handle_ = 0;
		}
		virtual ~Control()
		{
			ReleaseUserData();
		}

		//在鼠标位置弹出菜单
		static bool PopMenu(int menuId, WinHandle hWnd);

		inline void ReleaseUserData()
		{
			if (handle_)
			{
				::SetWindowLongPtr(handle_, GWLP_USERDATA, 0);
				handle_ = 0;
			}

		}

		inline static Control * GetUserData(HWND wnd)
		{
			return (Control *) ::GetWindowLongPtr(wnd, GWLP_USERDATA);
		}

		inline HWND GetHandle() const
		{
			return handle_;
		}
		///用资源标识符初始化
		inline void Init(int id)
		{
			handle_ = ::GetDlgItem(currentHandle_, id);
			resourceID_ = id;
			InitUserData();
		}


		inline void Hide() const
		{
			MY_ASSERT(handle_ != NULL);
			::ShowWindow(handle_, 0);
		}
		inline void Show() const
		{
			MY_ASSERT(handle_ != NULL);
			::ShowWindow(handle_, 1);
		}

		///是否启用控件
		inline void Enable(BOOL bo) const
		{
			MY_ASSERT(handle_ != NULL);
			::EnableWindow(handle_, bo);
		}

		//设置显示位置
		inline BOOL SetPos(int x, int y) const
		{
			MY_ASSERT(handle_ != NULL);
			return ::SetWindowPos(handle_, 0, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOCOPYBITS);
		}

		static inline BOOL SetPos(WinHandle handle, int x, int y, int w, int h);

		inline void SetLimitText(int maxLen)
		{
			MY_ASSERT(handle_ != NULL);
			::SendMessage(handle_, EM_SETLIMITTEXT, maxLen, 0);
		}

		//设置滚动条
		inline void SetProgressPos(int val)
		{
			MY_ASSERT(handle_ != NULL);
			MY_ASSERT(val >= 0);
			::PostMessage(handle_, PBM_SETPOS, static_cast<WPARAM>(val), 0);
		}

		void SetActive()
		{
			::SetActiveWindow(handle_);
		}
		void SetFocus()
		{
			//::SetActiveWindow(handle_);
			::SetFocus(handle_);
		}

		//获取id选框状态
		bool GetCheck()
		{
			return SendMessage(handle_, BM_GETCHECK, 0, 0)==1;
		}

		void SetCheck(BOOL Check)
		{
			SendMessage(handle_, BM_SETCHECK, Check, 0);
		}

		SS GetText() const
		{
			MY_ASSERT(handle_ != NULL);
			int res = ::GetWindowTextLength(handle_) + 1;
			SS text(res);
			text.strLength_ = GetWindowText(handle_, text.GetBuffer(), text.GetBufferSize());
			return text;
		}


		void GetText(SS & text) const
		{
			MY_ASSERT(handle_ != NULL);
			int res = ::GetWindowTextLength(handle_) + 1;
			text.SetBufSizeNoCopy(res);
			text.strLength_ = GetWindowText(handle_, text.GetBuffer(), text.GetBufferSize());
		}
		void SetText(const CC & str) const
		{
			MY_ASSERT(handle_ != NULL);
			::SetWindowText(handle_, str.char_);
		}

		void SetLimitText(UINT nMax)
		{
			::SendMessage(handle_, EM_SETLIMITTEXT, nMax, 0);
		}

		void SetBitmap(HBITMAP hBitmap)
		{
			::SendMessage(handle_, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);
		}
		HBITMAP GetBitmap() const
		{
			return (HBITMAP)::SendMessage(handle_, STM_GETIMAGE, IMAGE_BITMAP, 0L);
		}

		void Message(const CC & con, const CC & tit = tcc_("消息"))
		{
			MessageBox(handle_, con.char_, tit.char_, MB_ICONINFORMATION);
		}

		bool MessageOK(const CC & con, const CC & tit = tcc_("消息"))
		{
			return MessageBox(handle_, con.char_, tit.char_, MB_OKCANCEL | MB_ICONQUESTION) == IDOK;
		}

		void MessageERR(const CC & con, const CC & tit = tcc_("错误"))
		{
			MessageBox(handle_, con.char_, tit.char_, MB_ICONERROR);
		}

		//*******************************************
		// Summary : 返回false取消此消息
		// Returns - bool : 
		//*******************************************
		virtual bool ControlProc(HWND, UINT, WPARAM, LPARAM)
		{
			return true;
		}

		virtual bool ControlNotify(LPNMHDR)
		{
			return true;
		}


	protected:


	private:
		Control(const Control &);
		Control& operator=(const Control &);

	};

	struct WinHandle
	{
		HWND handle_;
		WinHandle(HWND han)
			: handle_(han)
		{
		}
		WinHandle(Control * hp)
		{
			MY_ASSERT(hp != nullptr);
			handle_ = hp->handle_;
		}

		WinHandle(Control & hp)
			: handle_(hp.handle_)
		{
		}

	};

	BOOL Control::SetPos(WinHandle handle, int x, int y, int w, int h)
	{
		return ::SetWindowPos(handle.handle_, 0, x, y, w, h, SWP_NOZORDER | SWP_NOCOPYBITS | SWP_NOACTIVATE);
	}



	//使用智能指针创建托管窗口
	template<class T>
	inline df::IntoPtr<T> NewWindow()
	{
		df::IntoPtr<T> obj(new T);
		obj->PtrIncRef();
		return obj;
	}

}

#include "Font.h"

#include "Gdi.h"
#include "gdiP.h"
#include "Bitmap.h"


#include "Button.h"
#include "TextBox.h"
#include "ListBox.h"
#include "ComBox.h"
#include "ChoiceBox.h"
#include "ImageList.h"
#include "ListView.h"
#include "Tray.h"
#include "TabControl.h"
#include "Window.h"


#endif // Control_h__2013_8_1__9_24
