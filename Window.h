#pragma once

namespace sdf {
	class Window;

	typedef std::shared_ptr<Window> WinPtr;

	class Window : public Control {
		DF_DISABLE_COPY_ASSIGN(Window);
	protected:

		Brush backBrush_;


		//自我引用, onClose时销毁
		std::shared_ptr<Window> ptr_;

		//父窗口引用
		std::shared_ptr<Window> ptrParent_;
	public:
		static int mouseX_, mouseY_;
		static const int32_t taskMessage_ = WM_USER + 8274;

		df::CC winClassName;

		//构造
		Window(void) : v(*this) {
			v.style.backColor = Color::white;
			isTop = true;
			drawBuff_ = new DrawBuffer(&gdi_);
		}

		virtual ~Window(void) {
			COUT(tt_("window gone:") << text);
			delete drawBuff_;
		}

		int32_t minHeight = 0;
		int32_t minWeight = 0;
		int32_t maxHeight = 0;
		int32_t maxWeight = 0;
		//是否有最大化按钮
		bool maxBox = true;
		bool minBox = true;
		//是否允许调整大小
		bool resizeAble = true;
		//初始为最大化
		bool initMaxSize = false;
		//初始为最小化
		bool initMixSize = false;

		bool noBorder = false;

		bool topMost = false;

		//是否为顶层窗口,关闭时同时结束消息循环
		bool isMain = false;

		Window& v;

		Gdi gdi_;
		//标题边框大小
		int32_t titleHeight_ = 0;

		//左边框大小
		int32_t borderSize_ = 0;


		int32_t GetTitleHeight() const { return titleHeight_; }

		int32_t GetBorderSize() const { return borderSize_; }

		uint32_t getExStyle() {
			uint32_t styEX = 0;
			if (topMost)
				styEX |= WS_EX_TOPMOST;

			if (noBorder) {
				//不显示在任务栏
				styEX |= WS_EX_TOOLWINDOW;
			}
			return styEX;
		}

		void updateStyle() {
			SetWindowLong(handle_, GWL_EXSTYLE, getExStyle());
		}

		//获取窗口字体
		inline static HFONT GetFont(HWND handle) {
			return (HFONT) ::SendMessage(handle, WM_GETFONT, 0, 0);
		}


		static Font& SetGlobalFont(Font& f) {
			GlobalFont().SetFont(f);
			Gdi::GetScreen().setFont(f);
		}

		static float getScale();


		static void runOnUi(std::function<void()>&& func);


		inline HFONT GetFont() {
			return GetFont(handle_);
		}

		//设置窗口图标
		inline static void setIcon(HWND h, int id) {
			::SendMessage(h, WM_SETICON, TRUE, (LPARAM)LoadIcon(Control::progInstance_, MAKEINTRESOURCE(id)));
		}

		inline void setIcon(int id) {
			::SendMessage(handle_, WM_SETICON, TRUE, (LPARAM)LoadIcon(Control::progInstance_, MAKEINTRESOURCE(id)));
		}


		/// <summary>
		/// 打开并显示窗口
		/// </summary>
		/// <param name="parent"></param>
		/// <param name="show"></param>
		void open(Window& parent, bool show = true) {
			open(&parent, show);
		}


		//打开窗口并移动到点击位置
		void openToPress(Window* parent = nullptr, bool show = true) {
			setPosToMouse();
			open(parent, show);
		}

		/// <summary>
		///  打开并显示窗口
		/// </summary>
		/// <param name="parent">不为空则禁用父窗口</param>
		/// <param name="show"></param>
		void open(Window* parent = nullptr, bool show = true) {
			if (parent)
				ptrParent_ = parent->sharedBase<Window>();
			openRaw(parent ? parent->GetHandle() : nullptr, show);
		}


		inline void toTop() {
			::BringWindowToTop(handle_);
			::SetForegroundWindow(handle_);
			::SetFocus(handle_);
		}

		void setPosToMouse() {
			POINT p;
			GetCursorPos(&p);
			pos.x = p.x;
			pos.y = p.y;
		}

		//关闭窗口
		void close(int code = 0) {
			PostMessage(handle_, WM_CLOSE, code, 0);
		}


		//判断窗口是否已经关闭
		bool isClosed() {
			return handle_ == 0;
		}

		//显示窗口,并开始消息循环
		void run(bool show = true);


		void AdjustLayout();

		virtual void onDraw() override;

		//获取屏幕高宽实际像素
		static int GetScreenWidth() {
			return ::GetSystemMetrics(SM_CXSCREEN);
		}

		static int GetScreenHeight() {
			return ::GetSystemMetrics(SM_CYSCREEN);
		}

		//获取dpi缩放后的屏幕高宽
		static float getScreenW() {
			return GetScreenWidth() / getScale();
		}

		static float getScreenH() {
			return GetScreenHeight() / getScale();
		}

		static void PopMessage(const df::CC& msg, int time = 1500);

		static LRESULT  __stdcall PopMessageProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

		//////////////////////////////////事件////////////////////////////////////////



		virtual void onInit();

		virtual HBRUSH OnDrawBackground();

		virtual void onPaint();


		virtual void onCreate() {}

		virtual void onResize() {

		}

		virtual void onExitResize() {

		}

		virtual void onActive() {

		}

		virtual void onInActive() {

		}

		virtual bool onClose(int code) {
			return true;
		}

		//布局更改事件
		virtual void onLayout() {

		}

		//移动事件
		virtual void onMove(int32_t x, int32_t y) {

		}

		//消息循环
		static void MessageLoop();


		//*******************************************
		// Summary : 获取边框大小
		//*******************************************
		void UpdateBorderSize() {
			POINT pon;
			pon.x = pos.x;
			pon.y = pos.y;
			::ScreenToClient(handle_, &pon);
			titleHeight_ = (int32_t)std::abs(pon.y);
			borderSize_ = (int32_t)std::abs(pon.x);
		}

		int32_t getBorderW() {
			return borderSize_ * 2;
		}

		int32_t getBorderH() {
			return titleHeight_ + borderSize_;
		}

	protected:
		//关闭窗口
		void closeRelease(int code);

		void setPosXY();

		///非阻塞,显示非模态对话框,parent:父窗口句柄,
		void openRaw(HWND parent = NULL, bool show = true);


		void InitWinData();

		void Release();

		//模态化与非模态话共用
		static intptr_t __stdcall WndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

	};


	inline int32_t dp2px(int32_t dv) {
		return (int32_t)(dv * Window::getScale());
	}

}


