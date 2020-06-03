#ifndef Control_h__2013_8_1__9_24
#define Control_h__2013_8_1__9_24

#include "../df/df.h"
#include <commctrl.h>

#ifdef _MSC_VER
#pragma comment(lib, "comctl32")
#pragma comment(lib, "Msimg32")
#pragma comment(lib, "gdiplus")

// Embed visual style XML manifest
#pragma comment(linker,                           \
    "\"/manifestdependency:type='Win32'"          \
    "   name='Microsoft.Windows.Common-Controls'" \
    "   version='6.0.0.0'"                        \
    "   processorArchitecture='*'"                \
    "   publicKeyToken='6595b64144ccf1df'"        \
    "   language='*'\""                           \
)

// Link common controls library
#pragma comment(lib, "ComCtl32.lib")

#endif // _MSC_VER

#include "Font.h"

#include "Gdi.h"
#include "gdiP.h"
#include "Bitmap.h"

namespace sdf {




	class ControlStyle {
	public:
		uint32_t color = 0;
		uint32_t backColor = 0;
		int shadowSize = 0;

		uint16_t borderLeft = 0;
		uint16_t borderTop = 0;
		uint16_t borderRight = 0;
		uint16_t borderBottom = 0;
		uint32_t borderColor = 0;
		std::shared_ptr<Bitmap> backImage;
		BitmapScaleType scaleType;

		void border(uint16_t size) {
			borderLeft = size;
			borderTop = size;
			borderRight = size;
			borderBottom = size;
		}
	};

	class ControlPos {
	public:
		int32_t x = 0;
		int32_t y = 0;


		//内容宽度,不包括右侧滚动条宽
		int32_t w = -1;
		int32_t h = -1;

		int32_t paddingLeft = 0;
		int32_t paddingTop = 0;
		int32_t paddingRight = 0;
		int32_t paddingBottom = 0;

		int32_t marginLeft = 0;
		int32_t marginTop = 0;
		int32_t marginRight = 0;
		int32_t marginBottom = 0;
		int16_t flexX = 0;
		int16_t flexY = 0;

		bool wrapX = false;
		bool wrapY = false;

		//是否垂直显示
		bool vector = false;

		//使用绝对坐标
		bool absolute = false;

		void flex(int32_t flex) {
			flexX = flex;
			flexY = flex;
		}

		void paddingX(int32_t val) {
			paddingLeft = val;
			paddingRight = val;
		}

		void paddingY(int32_t val) {
			paddingTop = val;
			paddingBottom = val;
		}

		void padding(int32_t val) {
			paddingLeft = val;
			paddingRight = val;
			paddingTop = val;
			paddingBottom = val;
		}


		void marginX(int32_t val) {
			marginLeft = val;
			marginRight = val;
		}

		void marginY(int32_t val) {
			marginTop = val;
			marginBottom = val;
		}

		void margin(int32_t val) {
			marginLeft = val;
			marginRight = val;
			marginTop = val;
			marginBottom = val;
		}


		//指定所有子成员居中
		void center(bool val = true) {
			_centerX = val;
			_centerY = val;
		}

		void centerX(bool val = true) {
			_centerX = val;
		}

		void centerY(bool val = true) {
			_centerY = val;
		}

		//在父容器中居中
		void centerInParent(bool val = true) {
			_centerInParentX = val;
			_centerInParentY = val;
		}

		void centerInParentX(bool val = true) {
			_centerInParentX = val;
		}

		void centerInParentY(bool val = true) {
			_centerInParentY = val;
		}

		bool _centerX = false;
		bool _centerY = false;
		bool _centerInParentX = false;
		bool _centerInParentY = false;
	};

	class Control : std::enable_shared_from_this<Control> {
	protected:
		HWND handle_ = 0;

		Control* parent_ = nullptr;

		int32_t showX_ = 0;
		int32_t showY_ = 0;

		friend class Tray;
		friend class View;
		friend class Button;
		friend struct WinHandle;
		//最近一次创建的窗口,用于OnInit
		static DF_THREAD_LOCAL_VAR HWND currentHandle_;
		static DF_THREAD_LOCAL_VAR Window* parentWindow_;
		static Window* currentWindow_;


		void InitUserData() const {
			::SetWindowLongPtr(handle_, GWLP_USERDATA, (LONG_PTR)this);
		}

		//按钮焦点对象
		static Control* mouseHandle_;
		//全局绘图缓冲
		static Bitmap buttonBmp_;
		static char* buttonBmpBuf_;
		static Gdiplus::Graphics* graph_;

		int32_t drawX = 0;
		//是否需要绘制到屏幕
		bool needDraw = true;
		int32_t drawY = 0;
	public:
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
		ControlStyle styleFocused;
		String text;
		//所有成员控件列表
		std::vector<std::unique_ptr<Control>> memberList_;


		static HINSTANCE progInstance_;

		Control() {
		}


		virtual ~Control() {
			ReleaseUserData();
		}

		static bool drawStyle(Control* cont, ControlStyle& style, const String& text);

		void setBackColor(int32_t color) {
			style.backColor = color;
			stylePress.backColor = color;
			styleHover.backColor = color;
			styleDisable.backColor = color;
			styleFocused.backColor = color;
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
			return parent_->drawX - parent_->getHoriPos() + pos.x;
		}

		int32_t getDrawY() {
			return parent_->drawY - parent_->getVertPos() + pos.y;
		}

		static Font& GlobalFont();

		static void init(HINSTANCE inst) {
			::SetProcessDPIAware();
			progInstance_ = inst;
		}

		//在鼠标位置弹出菜单
		static bool PopMenu(int menuId, WinHandle hWnd);

		inline void ReleaseUserData() {
			if (handle_) {
				::SetWindowLongPtr(handle_, GWLP_USERDATA, 0);
				handle_ = 0;
			}

		}

		inline static Control* GetUserData(HWND wnd) {
			return (Control*) ::GetWindowLongPtr(wnd, GWLP_USERDATA);
		}

		inline HWND GetHandle() const {
			return handle_;
		}

		///用资源标识符初始化
		virtual void Init();

		virtual void onHover() {

		};

		virtual void onLeave() {

		}

		virtual void onDraw() {

		}


		virtual void onDrawText(RECT& rect) {
			buttonBmp_.Txt(rect, text);
		}

		virtual void getContentWH(int32_t& w, int32_t& h) {
			w = 0;
			h = 0;
		}

		virtual void onMeasure();

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
		inline void enable(BOOL bo) const {
			DF_ASSERT(handle_ != NULL);
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
		inline BOOL setPos(int32_t x, int32_t y) {
			DF_ASSERT(handle_ != NULL);
			showX_ = x;
			showY_ = y;
			if (handle_)
				return ::SetWindowPos(handle_, 0, x, y, 0, 0, SWP_NOSIZE | SWP_NOREDRAW | SWP_NOZORDER | SWP_NOCOPYBITS);
			return false;
		}

		inline BOOL setPosAndHW(int32_t x, int32_t y, int32_t w, int32_t h) {
			showX_ = x;
			showY_ = y;
			showW_ = w;
			showH_ = h;
			if (handle_)
				return ::SetWindowPos(handle_, 0, x, y, w, h, SWP_NOZORDER | SWP_NOCOPYBITS | SWP_NOREDRAW);
			return false;
		}

		inline BOOL setHW(int32_t w, int32_t h) {
			showW_ = w;
			showH_ = h;
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
			return ::GetWindowTextLength(handle_);
		}

		df::String& getText() {
			getText(text);
			return text;
		}

		df::String& getText(df::String& text) const {
			DF_ASSERT(handle_ != NULL);
			int res = ::GetWindowTextLength(handle_);
			text.resize(res);
			GetWindowText(handle_, &text[0], (int)text.capacity());
			return text;
		}

		void setText(const df::CC& str) {
			DF_ASSERT(handle_ != NULL);
			text = str.c_str();
			::SetWindowText(handle_, str.char_);
		}


		void setLimitText(UINT nMax) {
			::SendMessage(handle_, EM_SETLIMITTEXT, nMax, 0);
		}

		void setBitmap(HBITMAP hBitmap) {
			::SendMessage(handle_, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);
		}

		HBITMAP getBitmap() const {
			return (HBITMAP) ::SendMessage(handle_, STM_GETIMAGE, IMAGE_BITMAP, 0L);
		}

		void setFont(const Font& font) {
			::SendMessage(handle_, WM_SETFONT, (WPARAM)font.GetFont(), TRUE);
		}

		void Message(const df::CC& con, const df::CC& tit = tcc_("消息")) {
			MessageBox(handle_, con.char_, tit.char_, MB_ICONINFORMATION);
		}

		bool MessageOK(const df::CC& con, const df::CC& tit = tcc_("消息")) {
			return MessageBox(handle_, con.char_, tit.char_, MB_OKCANCEL | MB_ICONQUESTION) == IDOK;
		}

		void MessageERR(const df::CC& con, const df::CC& tit = tcc_("错误")) {
			MessageBox(handle_, con.char_, tit.char_, MB_ICONERROR);
		}

		void measureUpdate() {
			Control::adjustRecur(this);
			update();
		}

		void update() {
			RECT re;
			re.left = 0;
			re.top = 0;
			re.right = pos.w;
			re.bottom = pos.h;
			::InvalidateRect(handle_, &re, true);

			//::UpdateWindow(handle_);
		}

		//*******************************************
		// Summary : 返回false取消此消息
		// Returns - bool :
		//*******************************************
		virtual bool ControlProc(HWND, UINT, WPARAM, LPARAM) {
			return true;
		}

		virtual bool ControlNotify(LPNMHDR) {
			return true;
		}

	protected:


		WNDPROC prevMsgProc_ = 0;
		int32_t measureX_ = 0;
		int32_t measureY_ = 0;

		static void adjustRecur(sdf::Control* cont) {
			cont->measureX_ = 0;
			cont->measureY_ = 0;
			for (auto& control : cont->memberList_) {
				control->onMeasure();
				adjustRecur(control.get());
			}
		}

		void updateDrawXY() {
			int32_t hp = parent_->getHoriPos();
			int32_t vp = parent_->getVertPos();
			drawX = parent_->drawX - hp + pos.x;
			drawY = parent_->drawY - vp + pos.y;
			if (pos.x - hp != showX_ || pos.y - vp != showY_)
				setPos(pos.x - hp, pos.y - vp);
		}

		void drawMember(Gdi& gdi);

		static LRESULT  __stdcall ButtonProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

		static intptr_t controlComProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

		void addSub(Control* control) {
			memberList_.push_back(std::unique_ptr<Control>(control));
		}

		void setParent(Control* parent) {
			parent_ = parent;
			parent->addSub(this);
		}

		virtual void initCreate() {

		}

		template<class T>
		void doCreate(T inst) {
			if (inst->onCreate_) {
				inst->onCreate_(*inst);
				if (pos.w < 0 && pos.flexX < 1)
					pos.wrapX = true;

				if (pos.h < 0 && pos.flexY < 1)
					pos.wrapY = true;

				/*            if (style.backImage) {
								if (pos.w > 0 && pos.h < 0) {
									pos.h = pos.w * style.backImage->GetHeight() / style.backImage->GetWidth();
								} else if (pos.h > 0 && pos.w < 0) {
									pos.w = pos.h * style.backImage->GetWidth() / style.backImage->GetHeight();
								} else if (pos.h < 0 && pos.w < 0) {
									pos.h = style.backImage->GetHeight();
									pos.w = style.backImage->GetWidth();
								}
							}*/

				for (auto& con : memberList_) {
					con->initCreate();
					Window::scalePos(con->pos);
				}

				inst->onCreate_ = 0;
			}
		}

		void initAllSub() {
			measureX_ = 0;
			measureY_ = 0;
			for (auto& con : memberList_) {
				con->Init();
			}
		}

		DF_DISABLE_COPY_ASSIGN(Control);
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


}

#define ui_control(Name_) Name_ * DF_MIX_LINENAME(UIBUTTON, __LINE__)=new Name_(&v);DF_MIX_LINENAME(UIBUTTON, __LINE__)->onCreate_=[&](Name_ &v)
#define ui_control2(Name_,Paras_) Name_ * DF_MIX_LINENAME(UIBUTTON, __LINE__)=new Name_(&v,Paras_);DF_MIX_LINENAME(UIBUTTON, __LINE__)->onCreate_=[&](Name_ &v)


#include "View.h"
#include "Button.h"
#include "ScrollView.h"
#include "TextBox.h"
#include "ListBox.h"
#include "ComBox.h"
#include "CheckBox.h"
#include "Tray.h"
#include "Window.h"


#endif // Control_h__2013_8_1__9_24
