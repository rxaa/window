#pragma once

namespace sdf
{
	class Window;

	typedef df::IntoPtr<Window> WinPtr;

	class Window :
		public Control
	{
	protected:
		PTR_REF_COUNT;
		bool isModal_;
		std::function<void()> onClose_;
		Gdi gdi_;
		static Bitmap backGround_;
		static Brush backBrush_;
		static int mouseX_ , mouseY_;

		std::map<int,std::function<void()>> commandMap_;
	public:

		Window(void);
		virtual ~Window(void);

		//获取窗口字体
		inline static HFONT GetFont(HWND handle)
		{
			return (HFONT)::SendMessage(handle , WM_GETFONT, 0, 0);
		}
		//设置窗口图标
		inline static void SetIcon(HWND h , int id)
		{
			::SendMessage(h, WM_SETICON, TRUE, (LPARAM)LoadIcon(df::Global::progressInstance_ , MAKEINTRESOURCE(id)) );
		}

		inline void SetIcon(int id)
		{
			::SendMessage(handle_, WM_SETICON, TRUE, (LPARAM)LoadIcon(df::Global::progressInstance_ , MAKEINTRESOURCE(id)) );
		}
		inline static void GetMousePos( LPARAM lParam )
		{
			mouseX_=LOWORD (lParam);
			mouseY_=HIWORD (lParam);
		}

		//初始化背景图片
		static void InitImage(int backGround);

		///非阻塞,显示非模态对话框,parent:父窗口句柄,
		void Open(HWND parent = NULL, bool show = true);
		void Open(Control & parent, bool show = true)
		{
			Open(parent.GetHandle(), show);
		}
		//阻塞,显示模态对话框
		void OpenModal(HWND parent);
		void OpenModal(Control & parent)
		{
			OpenModal(parent.GetHandle());
		}
		void OpenModal(Control * parent)
		{
			MY_ASSERT(parent!=nullptr);
			OpenModal(parent->GetHandle());
		}
		//关闭窗口
		void Close(bool exit = false);

		//判断窗口是否已经关闭
		bool IsClosed()
		{
			return handle_ == 0;
		}

		//显示窗口,并开始消息循环
		void Run(bool show = true);

		template<class T>
		void AddEvent(int id, T cb)
		{
			commandMap_[id]=cb;
		}

		static int GetScreenWidth()
		{
			return ::GetSystemMetrics(SM_CXSCREEN);
		}

		static int GetScreenHeight()
		{
			return ::GetSystemMetrics(SM_CYSCREEN);
		}

		static void PopMessage(const CC & msg,int time=1200);
		static LRESULT  __stdcall PopMessageProc(HWND hDlg, uint message, WPARAM wParam, LPARAM lParam);

		//////////////////////////////////事件////////////////////////////////////////
		virtual void OnInit();
		virtual HBRUSH OnDrawBackground();
		virtual void OnPaint();
		//参数为ture :按下 false 抬起
		virtual void OnMouseLeft(bool ){}
		virtual void OnMouseRight(bool ){}

		//消息循环
		static void MessageLoop();
	private:
		void InitWinData()
		{
			gdi_.Init(handle_);
			try
			{
				OnInit();
			}CATCH_SEH;
		}

		void Release();

	
		//模态化窗口消息处理
		static intptr_t __stdcall ModalProc(HWND hDlg, uint message, WPARAM wParam, LPARAM lParam);
		//模态化与非模态话共用
		static intptr_t __stdcall WndProc(HWND hDlg, uint message, WPARAM wParam, LPARAM lParam);
	};


}


