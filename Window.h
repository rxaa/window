#pragma once

namespace sdf {
	class Window;

	typedef std::shared_ptr<Window> WinPtr;

	class Window : public Control {
		DF_DISABLE_COPY_ASSIGN(Window);
	protected:


		Brush backBrush_;

		std::map<int, std::function<void()>> commandMap_;
		//自我引用, onClose时销毁
		std::shared_ptr<Window> ptr_;

		//父窗口引用
		std::shared_ptr<Window> ptrParent_;
	public:
		static int mouseX_, mouseY_;
		static const int32_t taskMessage_ = WM_USER + 8274;



		//构造
		Window(void) : v(*this) {
			v.style.backColor = Color::white;
			isTop = true;
			drawBuff_ = new DrawBuffer();
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
		int16_t titleHeight_ = 0;

		//左边框大小
		int16_t borderSize_ = 0;


		int16_t GetTitleHeight() const { return titleHeight_; }

		int16_t GetBorderSize() const { return borderSize_; }

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

		static void scalePos(ControlPos& pos, bool xy = true) {
			if (pos.scaleMeasured)
				return;

			pos.scaleMeasured = true;
			auto sca = getScale();
			if (xy) {
				pos.x = (int32_t)((float)pos.x * sca);
				pos.y = (int32_t)((float)pos.y * sca);
			}

			if (pos.w > 0)
				pos.w = (int32_t)((float)pos.w * sca);
			if (pos.h > 0)
				pos.h = (int32_t)((float)pos.h * sca);

			if (pos.maxH > 0)
				pos.maxH = (int32_t)((float)pos.maxH * sca);

			if (pos.maxW > 0)
				pos.maxW = (int32_t)((float)pos.maxW * sca);

			pos.paddingLeft = (int32_t)((float)pos.paddingLeft * sca);
			pos.paddingTop = (int32_t)((float)pos.paddingTop * sca);
			pos.paddingRight = (int32_t)((float)pos.paddingRight * sca);
			pos.paddingBottom = (int32_t)((float)pos.paddingBottom * sca);

			pos.marginLeft = (int32_t)((float)pos.marginLeft * sca);
			pos.marginTop = (int32_t)((float)pos.marginTop * sca);
			pos.marginRight = (int32_t)((float)pos.marginRight * sca);
			pos.marginBottom = (int32_t)((float)pos.marginBottom * sca);
		}

		static void scaleStyle(ControlStyle& sty) {
			auto sca = getScale();
			sty.shadowSize = (int32_t)((float)sty.shadowSize * sca);
			sty.borderTop = (int16_t)((float)sty.borderTop * sca);
			sty.borderRight = (int16_t)((float)sty.borderRight * sca);
			sty.borderLeft = (int16_t)((float)sty.borderLeft * sca);
			sty.borderBottom = (int16_t)((float)sty.borderBottom * sca);
		}

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

		inline static void GetMousePos(LPARAM lParam) {
			mouseX_ = LOWORD(lParam);
			mouseY_ = HIWORD(lParam);
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
			parent_ = parent;
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

		template<class T>
		void AddEvent(int id, T cb) {
			commandMap_[id] = cb;
		}


		void AdjustLayout();

		virtual void onDraw() override {

		}

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

		//窗口移动事件
		virtual void onMove() {

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
			titleHeight_ = (int16_t)std::abs(pon.y);
			borderSize_ = (int16_t)std::abs(pon.x);
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


