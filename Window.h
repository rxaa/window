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
		
		//是否为模态话对话框
		bool isModal_;
		
		//关闭事件
		std::function<void()> onClose_;
		
		Gdi gdi_;

		//标题边框大小
		int16_t titleHeight_ = 0;
		
		//左边框大小
		int16_t borderSize_ = 0;

		//所有成员控件列表
		List<Control*> memberList_;

		static Bitmap backGround_;
		static Brush backBrush_;
		static int mouseX_, mouseY_;

		std::map<int, std::function<void()>> commandMap_;
	public:

		Window(void);
		virtual ~Window(void);

		int16_t GetTitleHeight() const { return titleHeight_; }

		int16_t GetBorderSize() const { return borderSize_; }

		//获取窗口字体
		inline static HFONT GetFont(HWND handle)
		{
			return (HFONT)::SendMessage(handle, WM_GETFONT, 0, 0);
		}

		inline HFONT GetFont()
		{
			return GetFont(handle_);
		}
		//设置窗口图标
		inline static void SetIcon(HWND h, int id)
		{
			::SendMessage(h, WM_SETICON, TRUE, (LPARAM)LoadIcon(df::Global::progressInstance_, MAKEINTRESOURCE(id)));
		}

		inline void SetIcon(int id)
		{
			::SendMessage(handle_, WM_SETICON, TRUE, (LPARAM)LoadIcon(df::Global::progressInstance_, MAKEINTRESOURCE(id)));
		}
		inline static void GetMousePos(LPARAM lParam)
		{
			mouseX_ = LOWORD(lParam);
			mouseY_ = HIWORD(lParam);
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
			MY_ASSERT(parent != nullptr);
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
			commandMap_[id] = cb;
		}

		void SetTimer(uint id, uint time)
		{
			::SetTimer(handle_, id, time, 0);
		}

		void KillTimer(uint id)
		{
			::KillTimer(handle_, id);
		}

		void AdjustLayout();

		static int GetScreenWidth()
		{
			return ::GetSystemMetrics(SM_CXSCREEN);
		}

		static int GetScreenHeight()
		{
			return ::GetSystemMetrics(SM_CYSCREEN);
		}

		static void PopMessage(const CC & msg, int time = 1200);
		static LRESULT  __stdcall PopMessageProc(HWND hDlg, uint message, WPARAM wParam, LPARAM lParam);

		//////////////////////////////////事件////////////////////////////////////////
		virtual void OnInit();
		virtual HBRUSH OnDrawBackground();
		virtual void OnPaint();
		//参数为ture :按下 false 抬起
		virtual void OnMouseLeft(bool){}
		virtual void OnMouseRight(bool){}
		virtual void OnKeyUp(int){}

		virtual void OnTimer(uint){}

		virtual void OnResize()
		{

		}

		virtual void OnLayout()
		{

		}

		virtual void OnMove()
		{

		}

		//消息循环
		static void MessageLoop();


		//*******************************************
		// Summary : 获取边框大小
		//*******************************************
		void UpdateBorderSize()
		{
			POINT pon;
			pon.x = posX_;
			pon.y = posY_;
			::ScreenToClient(handle_, &pon);
			titleHeight_ = (int16_t)std::abs(pon.y);
			borderSize_ = (int16_t)std::abs(pon.x);
		}

	private:
		void InitWinData();

		void Release();


		//模态化窗口消息处理
		static intptr_t __stdcall ModalProc(HWND hDlg, uint message, WPARAM wParam, LPARAM lParam);
		//模态化与非模态话共用
		static intptr_t __stdcall WndProc(HWND hDlg, uint message, WPARAM wParam, LPARAM lParam);

		DISABLE_COPY_ASSIGN(Window);
	};


}


