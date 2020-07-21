#ifndef Control_h__2013_8_1__9_24
#define Control_h__2013_8_1__9_24

#include "ControlData.h"
#include "Font.h"

namespace sdf {


	class Control : public std::enable_shared_from_this<Control> {
		DF_DISABLE_COPY_ASSIGN(Control);
	protected:
		HWND handle_ = 0;

		Control* parent_ = nullptr;

		friend class Tray;

		friend class View;

		friend class Button;

		friend struct WinHandle;

		friend class ImageView;

		friend class ScrollView;

		friend class Window;
		friend class LoadAnim;

		//最近一次创建的窗口,用于OnInit
		static HWND currentHandle_;
		static Window* parentWindow_;
		static Window* currentWindow_;

		static std::vector<Control*> controlOpenList_;


		static void removeOpenControl(Control* cont);

		template<class T>
		static void findControl(std::function<void(T&)>&& func) {
			for (auto s : controlOpenList_) {
				T* con = dynamic_cast<T*>(s);
				if (con != nullptr) {
					try {
						func(*con);
					}DF_CATCH_ALL;
				}
			}
		}

		void InitUserData() const {
			::SetWindowLongPtr(handle_, GWLP_USERDATA, (LONG_PTR)this);
		}


		//按钮焦点对象
		static Control* mouseHandle_;

		/// <summary>
		/// 绘图缓冲,每个window和scrollView有单独的缓冲
		/// 其他所有子控件使用父容器的绘图缓冲
		/// </summary>
		DrawBuffer* drawBuff_ = nullptr;

		//相对于父容器的坐标
		int32_t showX_ = 0;
		int32_t showY_ = 0;

		//相对于背景画布的坐标
		int32_t drawX_ = 0;
		int32_t drawY_ = 0;

		//最后一次绘制的样式
		ControlStyle* lastDrawStyle = 0;


		//是否顶层节点
		bool isTop = false;
		bool isEnable = true;
	public:
		//是否需要绘制到屏幕
		bool needDraw = true;
		std::function<void()> onCreate_;
		std::function<void()> onBind_;
		friend Window;
		bool isHover = false;
		bool isFocused = false;
		bool isDisable = false;
		bool isPress = false;


		//用于计算内容长
		int32_t contentW = 0;
		int32_t contentH = 0;

		//显示宽度,包括右侧滚动条宽
		int32_t showW_ = 0;
		int32_t showH_ = 0;


		ControlPos pos;
		ControlStyle style;
		ControlStyle stylePress;
		ControlStyle styleHover;
		ControlStyle styleDisable;


		String text;
		//所有成员控件列表
		std::vector<std::shared_ptr<Control>> memberList_;


		static HINSTANCE progInstance_;

		static const uint32_t animInterval_ = 15;

		Control() {
		}

		void fontSize(uint32_t size) {
			style.font.size = size;
			stylePress.font.size = size;
			styleHover.font.size = size;
			styleDisable.font.size = size;
		}

		void fontBold() {
			if (style.font.size == 0) {
				fontSize(Font::initSize);
			}
			style.font.bold = true;
			stylePress.font.bold = true;
			styleHover.font.bold = true;
			styleDisable.font.bold = true;
		}

		template<class T>
		inline std::shared_ptr<T> castMember(size_t index) {
			DF_ASSERT(index < memberList_.size());
			return std::static_pointer_cast<T>(memberList_[index]);
		}

		virtual ~Control() {
			ReleaseUserData();
		}

		//回取顶层绘图缓冲
		DrawBuffer* getDraw() {
			DrawBuffer* draw = drawBuff_;
			auto par = parent_;
			while (draw == nullptr) {
				if (par == nullptr) {
					return nullptr;
				}
				draw = par->drawBuff_;
				par = par->parent_;
			}
			return draw;
		}

		template<typename Derived>
		inline std::shared_ptr<Derived> sharedBase() {
			//return std::shared_ptr<Derived>(static_cast<Derived*>(this));
			return std::static_pointer_cast<Derived>(shared_from_this());
		}

		/// <summary>
		/// 绘制style
		/// </summary>
		/// <param name="cont"></param>
		/// <param name="style"></param>
		/// <param name="text"></param>
		/// <param name="parentBack">当背景为空时使用父级背景</param>
		/// <returns></returns>
		bool drawStyle(DrawBuffer* draw, ControlStyle& style, bool parentBack = false);

		Font& getFont() {
			if (style.font.hasFont()) {
				return Font::getFont(style.font);
			}
			return GlobalFont();
		}

		void setBackColor(int32_t color) {
			style.backColor = color;
			stylePress.backColor = color;
			styleHover.backColor = color;
			styleDisable.backColor = color;
		}

		Control* getParent() {
			return parent_;
		}

		HWND getParentHandle() {
			if (parent_)
				return parent_->handle_;
			return 0;
		}

		virtual int32_t getHoriPos() {
			return 0;
		}

		virtual int32_t getVertPos() {
			return 0;
		}

		int32_t getDrawX() {
			return parent_->drawX_ - parent_->getHoriPos() + pos.x;
		}

		//是否完全溢出父容器
		bool showOverflow();

		inline bool hited(int32_t x, int32_t y) {
			return x > pos.x && y > pos.y && x < pos.x + pos.w && y < pos.y + pos.h;
		}

		int32_t getDrawY() {
			return parent_->drawY_ - parent_->getVertPos() + pos.y;
		}

		static Font& GlobalFont();

		static void initInst(HINSTANCE inst) {
			::SetProcessDPIAware();
			progInstance_ = inst;
		}

		//在鼠标位置弹出菜单
		static bool PopMenu(int menuId, WinHandle hWnd);

		void ReleaseUserData();

		inline static Control* GetUserData(HWND wnd) {
			return (Control*) ::GetWindowLongPtr(wnd, GWLP_USERDATA);
		}

		inline HWND GetHandle() const {
			return handle_;
		}


		virtual void onHover() {

		};

		virtual void onLeave() {

		}

		virtual void onPress(bool down) {

		}

		virtual void onLeftClick() {

		}
		virtual void onRightClick() {

		}

		virtual void onDraw() {
			updateDrawXY();
		}

		virtual void onKillFocus() {

		}
		virtual void onFocus() {

		}

		virtual void bindUpdate(bool draw = true);


		virtual void onDrawText(RECT& rect, ControlStyle& style, DrawBuffer* draw);

		virtual void getContentWH(int32_t& w, int32_t& h) {
			w = 0;
			h = 0;
		}

		virtual void onMouseMove(int32_t x,int32_t y){

		}

		virtual void reGetContentWH(int32_t& w, int32_t& h) {

		}

		virtual void onMeasure();

		virtual void onTimer(uint32_t) {}

		//*******************************************
		// Summary : 获取在父窗口中的位置与大小
		//*******************************************
		void UpdateWinRect();


		inline void hide() const {
			DF_ASSERT(handle_ != NULL);
			::ShowWindow(handle_, 0);
		}

		inline void show() const {
			DF_ASSERT(handle_ != NULL);
			::ShowWindow(handle_, 1);
		}


		///是否启用控件
		inline void enable(bool bo) {
			isEnable = bo;
			if (handle_)
				::EnableWindow(handle_, bo);
		}

		inline int32_t GetWidth() const {
			return pos.w;
		}

		inline int32_t GetHeight() const {
			return pos.h;
		}

		inline int32_t GetPosX() const {
			return pos.x;
		}

		inline int32_t GetPosY() const {
			return pos.y;
		}

		//设置显示位置
		BOOL setPos(int32_t x, int32_t y);

		BOOL setPosAndHW(int32_t x, int32_t y, int32_t w, int32_t h);

		BOOL setHW(int32_t w, int32_t h, bool save = true) {
			if (save) {
				showW_ = w;
				showH_ = h;
			}
			return ::SetWindowPos(handle_, 0, 0, 0, w, h, SWP_NOMOVE | SWP_NOREDRAW | SWP_NOZORDER | SWP_NOCOPYBITS);
		}

		//static inline BOOL SetPos(WinHandle handle, int x, int y, int w, int h);

		inline void setLimitText(int maxLen) {
			DF_ASSERT(handle_ != NULL);
			::SendMessage(handle_, EM_SETLIMITTEXT, maxLen, 0);
		}

		void setReadOnly(BOOL bReadOnly /* = TRUE */) {
			::SendMessage(handle_, EM_SETREADONLY, bReadOnly, 0L);
		}

		//设置滚动条
		inline void setProgressPos(int val) {
			DF_ASSERT(handle_ != NULL);
			DF_ASSERT(val >= 0);
			::PostMessage(handle_, PBM_SETPOS, static_cast<WPARAM>(val), 0);
		}

		void addText(const df::CC& str);

		int getLineCount() const {
			return (int) ::SendMessage(handle_, EM_GETLINECOUNT, 0, 0);
		}

		int getLineIndex(int nLine) const {
			return (int) ::SendMessage(handle_, EM_LINEINDEX, nLine, 0);
		}


		//*******************************************
		// Summary : 获取字符位置nStartChar所在行的长度
		// Parameter - int nStartChar : 字符位置(-1获取光标所在行)
		// Returns - int : 所在行长度
		//*******************************************
		int getLineLength(int nStartChar) const {
			return (int) ::SendMessage(handle_, EM_LINELENGTH, nStartChar, 0);
		}

		void setSelectText(int nStartChar, int nEndChar, BOOL bNoScroll = FALSE) {
			::SendMessage(handle_, EM_SETSEL, nStartChar, nEndChar);
			if (!bNoScroll)
				::SendMessage(handle_, EM_SCROLLCARET, 0, 0);
		}

		void setScroll(int nLines, int nChars = 0) {
			::SendMessage(handle_, EM_LINESCROLL, nChars, nLines);
		}

		void replaceSelectText(const df::CC& lpszNewText, BOOL bCanUndo = FALSE) {
			::SendMessage(handle_, EM_REPLACESEL, (WPARAM)bCanUndo, (LPARAM)lpszNewText.GetBuffer());
		}

		void setActive() {
			::SetActiveWindow(handle_);
		}

		void setFocus() {
			//::SetActiveWindow(handle_);
			::SetFocus(handle_);
		}


		int getTextLength() const {
			if (handle_)
				return ::GetWindowTextLength(handle_);
			else
				return (int)text.size();
		}

		df::String& getText() {
			getText(text);
			return text;
		}

		df::String& getText(df::String& text) const {
			if (!handle_) {
				text = this->text;
				return text;
			}

			int res = ::GetWindowTextLength(handle_);
			text.resize(res);
			GetWindowText(handle_, &text[0], (int)text.capacity());
			return text;
		}

		void setText(const df::CC& str) {
			text = str.c_str();
			if (handle_)
				::SetWindowText(handle_, str.char_);
		}


		void setLimitText(UINT nMax) {
			::SendMessage(handle_, EM_SETLIMITTEXT, nMax, 0);
		}


		void setFont(const Font& font) {
			::SendMessage(handle_, WM_SETFONT, (WPARAM)font.GetFont(), TRUE);
		}

		virtual void measureUpdate() {
			if (parent_) {
				parent_->measureX_ = pos.x;
				parent_->measureY_ = pos.y;
				onMeasure();
				measureX_ = 0;
				measureY_ = 0;
				for (auto& sub : memberList_) {
					sub->onMeasure();
				}
			}
			
			//Control::adjustRecur(this);
			onDraw();
		}

		void update() {
			::InvalidateRect(handle_, NULL, true);
			//::UpdateWindow(handle_);
		}


		void _addSub(const std::shared_ptr<Control>& control) {
			control->parent_ = this;
			memberList_.push_back(control);
		}

		void addMember(const std::shared_ptr<Control>& con);

		void removeAllMember();

		void removeFromParent();

		//*******************************************
		// Summary : 返回false取消此消息
		// Returns - bool :
		//*******************************************
		virtual bool ControlProc(HWND, UINT, WPARAM, LPARAM, LRESULT&) {
			return true;
		}

		virtual bool ControlNotify(LPNMHDR) {
			return true;
		}



		virtual void doCreate();

		uint32_t generateId() {
			return (uint32_t)(std::rand() % (INT_MAX - 1)) + 1;
		}

		static void drawRect(uint32_t* buf, int32_t bufW, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color);
		void _removeFromParent(bool remove);
	protected:



		WNDPROC prevMsgProc_ = 0;
		int32_t measureX_ = 0;
		int32_t measureY_ = 0;

		void measureWrapX(int32_t minW);

		void measureWrapY(int32_t minH);

		void drawParentBack(DrawBuffer* draw);

		static void adjustRecur(sdf::Control* cont);

		//更新在父容器中的位置
		void updateDrawXY();


		void drawMember(Gdi& gdi, DrawBuffer* draw);

		static LRESULT  __stdcall ButtonProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

		static intptr_t controlComProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


		/**
		* 设置定时器
		* 每隔time毫秒触发onTimer
		* @param id
		* @param time 毫秒时间
		*/
		void setTimer(uint32_t id, uint32_t time) {
			::SetTimer(handle_, id, time, 0);
		}

		void killTimer(uint32_t id) {
			::KillTimer(handle_, id);
		}

		///执行measure, 创建窗口句柄, 初始化所有子view
		virtual void Init();


		void initAllSub();


	};

	struct WinHandle {
		HWND handle_;

		WinHandle(HWND han)
			: handle_(han) {
		}

		WinHandle(Control* hp) {
			DF_ASSERT(hp != nullptr);
			handle_ = hp->handle_;
		}

		WinHandle(Control& hp)
			: handle_(hp.handle_) {
		}

	};

	typedef std::shared_ptr<sdf::Control> PtrControl;
}


#define ui_control(Name_) std::shared_ptr<Name_> DF_MIX_LINENAME(UIBUTTON, __LINE__)=std::make_shared<Name_>();v._addSub(DF_MIX_LINENAME(UIBUTTON, __LINE__));DF_MIX_LINENAME(UIBUTTON, __LINE__)->onCreate_=[&,&v=*DF_MIX_LINENAME(UIBUTTON, __LINE__)]()
#define ui_control2(Name_, Paras_) std::shared_ptr<Name_> DF_MIX_LINENAME(UIBUTTON, __LINE__)=std::make_shared<Name_>(Paras_);v._addSub(DF_MIX_LINENAME(UIBUTTON, __LINE__));DF_MIX_LINENAME(UIBUTTON, __LINE__)->onCreate_=[&,&v=*DF_MIX_LINENAME(UIBUTTON, __LINE__)]()

#define ui_bind v.onBind_=[&]()

# include"View.h"
#include "Button.h"
#include "ImageView.h"
#include "LoadAnim.h"
#include "ScrollView.h"
#include "TextBox.h"
#include "ListBox.h"
#include "ComBox.h"
#include "CheckBox.h"
#include "ListView.h"
#include "Tray.h"
#include "Window.h"
#include "FormMenu.h"
#include "FormOk.h"
#include "ViewOk.h"
#include "ImageMore.h"

#endif // Control_h__2013_8_1__9_24
