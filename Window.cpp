#include "StdAfx.h"

#include "Control.h"


DF_THREAD_LOCAL_VAR HWND sdf::Control::currentHandle_ = NULL;
DF_THREAD_LOCAL_VAR sdf::Window* sdf::Control::parentWindow_ = NULL;

HINSTANCE sdf::Control::progInstance_ = nullptr;

sdf::Window* sdf::Control::currentWindow_ = nullptr;


sdf::Control* sdf::Control::mouseHandle_ = 0;

sdf::Bitmap sdf::Control::buttonBmp_;
char* sdf::Control::buttonBmpBuf_ = nullptr;

Gdiplus::Graphics* sdf::Control::graph_ = 0;

int  sdf::Window::mouseY_ = 0;

int  sdf::Window::mouseX_ = 0;

ULONG_PTR sdf::Gdip::gdiplusToken_ = 0;

Gdiplus::GdiplusStartupInput sdf::Gdip::gdiplusStartupInput_ = 0;


sdf::Font::Font(long size, df::CC name) {
	rawSize = size;
	size = (long)((float)size * Window::getScale());
	memset(&logFont_, 0, sizeof(LOGFONT));
	if (name.length_ > 31)
		name.length_ = 31;
	memcpy(logFont_.lfFaceName, name.char_, name.length_ * sizeof(TCHAR));
	logFont_.lfHeight = size;
	font_ = CreateFontIndirect(&logFont_);
}

HFONT sdf::Font::SetFont(df::CC name, long size) {
	rawSize = size;
	size = (long)((float)size * Window::getScale());
	memset(&logFont_, 0, sizeof(LOGFONT));

	if (name.length_ > 31)
		name.length_ = 31;
	memcpy(logFont_.lfFaceName, name.char_, name.length_ * sizeof(TCHAR));

	logFont_.lfHeight = size;
	DeleteObject(font_);
	font_ = CreateFontIndirect(&logFont_);
	return font_;
}


void sdf::Gdi::Init(Control& cont) {
	hdc_ = ::GetDC(cont.GetHandle());
}

void sdf::Gdi::Init(Control* cont) {
	DF_ASSERT(cont != nullptr);
	hdc_ = ::GetDC(cont->GetHandle());
}


//////////////////////////////////////////////////////////////////////////

void sdf::Control::addText(const df::CC& str) {
	int len = getTextLength();
	setSelectText(len, len);
	replaceSelectText(str.GetBuffer());
}

void sdf::Control::onMeasure() {
	if (parent_ == nullptr) {
		return;
	}
	measureX_ = 0;
	measureY_ = 0;



	if (pos.absolute) {

	}
	else {
		pos.x = parent_->measureX_ + pos.marginLeft + parent_->pos.paddingLeft;
		pos.y = parent_->measureY_ + pos.marginTop + parent_->pos.paddingTop;
	}


	int32_t w = 0, h = 0;
	getContentWH(w, h);

	if (pos.wrapX || pos.wrapY) {
		//wrap样式,提前计算所有子成员
		for (auto& sub : memberList_) {
			sub->onMeasure();
		}
	}
	measureX_ = 0;
	measureY_ = 0;

	int32_t flexXsum = 0, flexYsum = 0;
	int32_t flexWsum = 0, flexHsum = 0;
	bool isRight = false;
	//计算所有邻居flex
	if (pos.flexX > 0 || pos.flexY > 0) {
		int32_t oldMx = parent_->measureX_, oldMy = parent_->measureY_;
		for (auto& sub : parent_->memberList_) {
			if (sub->pos.absolute)
				continue;


			if (isRight) {//提前计算右侧邻节点
				sub->onMeasure();
			}

			if (sub.get() == this) {
				isRight = true;
			}

			if (sub->pos.flexX > 0)
				flexXsum += sub->pos.flexX;
			else if (sub->pos.w > 0) {
				flexWsum += sub->pos.w + sub->pos.marginRight + sub->pos.marginLeft;
			}

			if (sub->pos.flexY > 0)
				flexYsum += sub->pos.flexY;
			else if (sub->pos.h > 0) {
				flexHsum += sub->pos.h + sub->pos.marginRight + sub->pos.marginLeft;
			}
		}
		parent_->measureX_ = oldMx, parent_->measureY_ = oldMy;
	}


	if (pos.flexX > 0) {
		if (parent_->pos.w > 0) {
			if (parent_->pos.vector)
				pos.w = parent_->pos.w - parent_->pos.paddingLeft - parent_->pos.paddingRight - pos.marginLeft -
				pos.marginRight;
			else
				pos.w = (parent_->pos.w - parent_->pos.paddingLeft - parent_->pos.paddingRight - flexWsum) * pos.flexX /
				flexXsum - pos.marginLeft - pos.marginRight;
		}
	}
	else if (pos.wrapX) {
		int32_t subW = 0;

		for (auto& sub : memberList_) {
			//查找所有成员总宽,或最大宽
			if (sub->pos.w > 0) {
				int32_t sum = sub->pos.w + sub->pos.marginRight + sub->pos.marginLeft;
				if (pos.vector) {
					if (sum > subW)
						subW = sum;
				}
				else {
					if (sub->pos.absolute) {
						if (sub->pos.w > subW)
							subW = sub->pos.w;
					}
					else {
						subW += sum;
					}

				}
			}
		}
		if (subW < w)
			subW = w;

		pos.w = subW + pos.paddingLeft + pos.paddingRight;

	}
	else {

	}

	if (pos.flexY > 0) {
		//查找所有邻居
		if (parent_->pos.h > 0) {
			if (parent_->pos.vector)
				pos.h = (parent_->pos.h - flexHsum - parent_->pos.paddingTop - parent_->pos.paddingBottom) * pos.flexY /
				flexYsum - pos.marginTop - pos.marginBottom;
			else
				pos.w = parent_->pos.h - pos.marginTop - pos.marginBottom - parent_->pos.paddingTop -
				parent_->pos.paddingBottom;
		}
	}
	else if (pos.wrapY) {
		int32_t subH = 0;

		for (auto& sub : memberList_) {
			//查找所有成员总高,或最大高
			if (sub->pos.h > 0) {
				int32_t sum = sub->pos.h + sub->pos.marginTop + sub->pos.marginBottom;
				if (pos.vector) {
					if (sub->pos.absolute) {
						if (sub->pos.h > subH)
							subH = sub->pos.h;
					}
					else {
						subH += sum;
					}
				}
				else {
					if (sum > subH)
						subH = sum;
				}
			}
		}
		if (subH < h)
			subH = h;
		pos.h = subH + pos.paddingTop + pos.paddingBottom;
	}
	else {

	}

	if ((pos._centerInParentX || parent_->pos._centerX) && parent_->pos.w > 0 && pos.w > 0) {
		pos.x = (parent_->pos.w - pos.w) / 2;
	}

	if ((pos._centerInParentY || parent_->pos._centerY) && parent_->pos.h > 0 && pos.h > 0) {
		pos.y = (parent_->pos.h - pos.h) / 2;
	}

	if ((pos.x != showX_ || pos.y != showY_ || pos.w != showW_ || pos.h != showH_))
		setPosAndHW(pos.x, pos.y, pos.w, pos.h);


	if (pos.absolute) {
	}
	else {

		if (parent_->pos.vector) {
			parent_->measureY_ += pos.h + pos.marginBottom + pos.marginTop;
		}
		else {
			parent_->measureX_ += pos.w + pos.marginRight + pos.marginLeft;
		}
	}



}

void sdf::Control::UpdateWinRect() {
	RECT rect;

	GetClientRect(handle_, &rect);

	pos.h = (int16_t)(rect.bottom);
	pos.w = (int16_t)(rect.right);


	::GetWindowRect(handle_, &rect);
	pos.x = (int16_t)rect.left;
	pos.y = (int16_t)rect.top;

	if (parentWindow_) {
		pos.x -= parentWindow_->pos.x + parentWindow_->borderSize_;
		pos.y -= parentWindow_->pos.y + parentWindow_->titleHeight_;
	}
}

void sdf::Control::Init() {
	InitUserData();
}


////////////////////////////////Brush//////////////////////////////////////////

void sdf::Brush::SetFromBitmap(Bitmap& bmp) {
	ReleaseBrush();
	brush_ = CreatePatternBrush(bmp.GetBitmap());
}

////////////////////////////////Window//////////////////////////////////////////

sdf::Window::Window(void)
	: v(*this) {
	pos.w = 1024;
	pos.h = 700;
	onClose_ = [&] {
		close();
	};
}


sdf::Window::~Window(void) {
}

intptr_t __stdcall sdf::Window::ModalProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {


	if (message == WM_INITDIALOG) {
		Window* winP = (Window*)(lParam);
		if (winP == nullptr)
			return FALSE;

		winP->handle_ = hDlg;
		currentHandle_ = winP->handle_;
		currentWindow_ = winP;

		winP->InitUserData();
		winP->InitWinData();


		return TRUE;
	}

	return WndProc(hDlg, message, wParam, lParam);
}

void sdf::Control::drawMember(Gdi& gdi)
{
	needDraw = false;
	ON_SCOPE_EXIT({
					  needDraw = true;
		});
	for (auto& sub : memberList_) {
		sub->onDraw();
	}
	if (parent_->needDraw)
		gdi.DrawFrom(Control::buttonBmp_, 0, 0, pos.w, pos.h, drawX, drawY);
}

LRESULT  __stdcall sdf::Control::ButtonProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	Control* cont = GetUserData(hDlg);
	switch (message) {
	case WM_COMMAND: {
		auto l = LOWORD(wParam);
		//auto h = HIWORD(wParam);
		if (lParam) {
			Control* controlP = GetUserData((HWND)lParam);
			if (controlP) {
				controlP->ControlProc(hDlg, message, wParam, lParam);
				break;
			}
		}
		break;
	}
	case WM_LBUTTONDBLCLK:
		PostMessage(hDlg, WM_LBUTTONDOWN, wParam, lParam);
		break;
	}
	if (cont)
		cont->ControlProc(hDlg, message, wParam, lParam);

	controlComProc(hDlg, message, wParam, lParam);

	if (cont && cont->prevMsgProc_)
		return CallWindowProc(cont->prevMsgProc_, hDlg, message, wParam, lParam);
	return DefWindowProc(hDlg, message, wParam, lParam);
}

//父子公用消息处理
intptr_t sdf::Control::controlComProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	//COUT((int*)message);
	sdf::Control* cont = sdf::Window::GetUserData(hDlg);
	switch (message) {

	case WM_DRAWITEM: {
		//COUT(tt_("WM_DRAWITEM"));
		//声明一个指向DRAWITEMSTRUCT结构体的指针并将其指向存储着按钮构造信息的lParam
		LPDRAWITEMSTRUCT lpDIS = (LPDRAWITEMSTRUCT)lParam;
		sdf::Control* controlP = sdf::Window::GetUserData(lpDIS->hwndItem);
		if (controlP) {
			if (lpDIS->itemState & ODS_SELECTED) {
				controlP->isPress = true;
			}
			else {
				controlP->isPress = false;
				//在按钮内抬起
				if (sdf::Control::mouseHandle_ == controlP) {
					controlP->isHover = true;
				}
				//在按钮外抬起
				else {
					controlP->isHover = false;
				}
			}
			controlP->onDraw();
		}
		break;
	}

					//移出窗口
	case WM_NCMOUSEMOVE: {
		//case WM_NCMOUSELEAVE: {
		if (sdf::Control::mouseHandle_) {

			if (sdf::Control::mouseHandle_->isHover) {
				sdf::Control::mouseHandle_->isHover = false;
				sdf::Control::mouseHandle_->onLeave();
			}
			sdf::Control::mouseHandle_ = 0;
		}
		break;
	}
	case WM_MOUSEMOVE: {
		sdf::Window::GetMousePos(lParam);

		if (cont == nullptr)
			break;

		//鼠标移出控件
		if (cont != sdf::Control::mouseHandle_) {
			if (!cont->isHover) {
				cont->isHover = true;
				cont->onHover();
			}

			if (sdf::Control::mouseHandle_ && sdf::Control::mouseHandle_->isHover) {
				sdf::Control::mouseHandle_->isHover = false;
				sdf::Control::mouseHandle_->onLeave();
			}
			sdf::Control::mouseHandle_ = cont;
		}
		break;
	}
	}
	/*if (cont && cont->prevMsgProc_)
		return CallWindowProc(cont->prevMsgProc_, hDlg, message, wParam, lParam);
	return DefWindowProc(hDlg, message, wParam, lParam);*/
	return 0;
}

intptr_t __stdcall sdf::Window::WndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	try {
		///获取窗口对象指针

		Window* winP = (Window*)GetUserData(hDlg);
		if (winP == nullptr) {
			//currentHandle_ = 0;
			currentWindow_ = nullptr;
			return DefWindowProc(hDlg, message, wParam, lParam);
		}

		if (message == WM_DESTROY) {
			winP->Release();
			return DefWindowProc(hDlg, message, wParam, lParam);
		}

		//currentHandle_ = hDlg;
		currentWindow_ = winP;

		switch (message) {
		case WM_CLOSE: {
			if (winP->onClose_)
				winP->onClose_();
			break;
		}

		case WM_COMMAND: {
			auto l = LOWORD(wParam);
			//auto h = HIWORD(wParam);

			if (l == IDCANCEL) {
				if (winP->onClose_)
					winP->onClose_();
				break;
			}

			if (lParam) {
				Control* controlP = GetUserData((HWND)lParam);
				if (controlP) {
					controlP->ControlProc(hDlg, message, wParam, lParam);
					break;
				}
			}

			auto it = winP->commandMap_.find(l);
			if (it != winP->commandMap_.end()) {
				it->second();
			}

			break;
		}

		case WM_NOTIFY: {
			LPNMHDR lpn = (LPNMHDR)lParam;
			Control* controlP = GetUserData(lpn->hwndFrom);
			if (controlP)
				controlP->ControlNotify(lpn);
			break;
		}

		case WM_SIZE: {
			winP->pos.w = LOWORD(lParam);
			winP->pos.h = HIWORD(lParam);
			winP->onResize();
			winP->AdjustLayout();
			break;
		}
		case WM_MOVE: {
			winP->pos.x = LOWORD(lParam);
			winP->pos.y = HIWORD(lParam);
			winP->onMove();
			break;
		}
		case WM_RBUTTONDOWN:
			winP->onMouseRight(true);
			break;
		case WM_RBUTTONUP:
			winP->onMouseRight(false);
			break;
		case WM_LBUTTONDOWN:
			winP->onMouseLeft(true);
			break;
		case WM_LBUTTONUP:
			winP->onMouseLeft(false);
			break;
		case WM_TIMER:
			winP->onTimer((UINT)wParam);
			break;
		case WM_PAINT: {
			//PAINTSTRUCT ps={0} ;
			//HDC dd=BeginPaint (hDlg, &ps) ;
			//COUT(t_t("dc:")<<winP->gdi_.GetDc()<<t_t(" - ")<<dd);
			winP->onPaint();
			//EndPaint (hDlg, &ps) ;
			break;
		}
					 //case WM_ERASEBKGND:

		case Tray::TRAY_MESSAGE:
			//cout<<"hwnd:"<<(int)hDlg<<endl;
			if (LOWORD(lParam) == WM_RBUTTONUP) //右键菜单
			{
				Tray::OnRightClick()();
			}
			else if (LOWORD(lParam) == WM_LBUTTONUP) //左键显示窗口
			{
				Tray::OnLeftClick()();
			}
			break;
		case WM_CTLCOLORBTN:
			::SetBkMode((HDC)wParam, TRANSPARENT);
			return (intptr_t)Brush::GetNullBrush();
		case WM_CTLCOLORSTATIC:
			::SetBkMode((HDC)wParam, TRANSPARENT);
			//return FALSE;
		case WM_CTLCOLORDLG:
			return (intptr_t)winP->OnDrawBackground();
			//限制大小
			//case   WM_GETMINMAXINFO:{
			//	MINMAXINFO*   pMinMax = (MINMAXINFO*)lParam;
			//	pMinMax->ptMinTrackSize.x = 640;
			//	pMinMax->ptMinTrackSize.y = 480;
			//	pMinMax->ptMaxTrackSize.x = 1024;
			//	pMinMax->ptMaxTrackSize.y = 768;
			//	return TRUE;
			//						}
		}

		controlComProc(hDlg, message, wParam, lParam);

	} DF_CATCH_ALL;
	return DefWindowProc(hDlg, message, wParam, lParam);
}

void sdf::Window::open(HWND parent/*=0*/, bool show) {
	isModal_ = false;
	//DF_ASSERT(resourceID_ != 0);

	TCHAR className[18] = tt_("sdf");
	for (int i = 0; i < 14; i++) {
		className[i + 3] = std::rand() % 26 + 'a';
	}

	WNDCLASS wndclass = { 0 };
	wndclass.style = CS_VREDRAW | CS_HREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = Control::progInstance_;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	if (style.backColor) {
		backBrush_.SetBrush(style.backColor);
		wndclass.hbrBackground = backBrush_.GetBrush();//窗口背影画刷为空
	}
	else {
		wndclass.hbrBackground = Brush::GetWhiteBrush();//窗口背影画刷为空
	}

	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = className;
	if (!RegisterClass(&wndclass)) {
		DF_ERR(tcc_("RegisterClass failed"));
		return;
	}

	Window::scalePos(pos);

	if (pos._centerInParentX) {
		pos.x = (Window::GetScreenWidth() - pos.w) / 2;
	}
	if (pos._centerInParentY) {
		pos.y = (Window::GetScreenHeight() - pos.h) / 2;
	}

	DWORD sty = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE;
	if (maxBox)
		sty |= WS_MAXIMIZEBOX;

	if (minBox)
		sty |= WS_MINIMIZEBOX;

	if (resizeAble)
		sty |= WS_THICKFRAME;


	if (initMaxSize)
		sty |= WS_MAXIMIZE;

	if (initMixSize)
		sty |= WS_MINIMIZE;

	handle_ = CreateWindowEx(NULL,
		className,
		text.c_str(),
		sty,
		pos.x,
		pos.y,
		pos.w,
		pos.h,
		NULL,
		NULL,
		Control::progInstance_,
		NULL);

	//handle_ = ::CreateDialogParam(Control::progInstance_, tt_("class"), parent, WndProc, 0);
	if (handle_ == NULL) {
		uint32_t dw = df::GetError();
		DF_ERR(tcc_("CreateWindowEx failed"));
		Release();
		return;
	}
	InitUserData();
	//SetScrollRange(handle_, SB_VERT, 0, 100, FALSE);
	HWND oldHandle = currentHandle_;
	//Window * oldWindow = currentWindow_;
	currentHandle_ = handle_;
	currentWindow_ = this;

	::ShowWindow(handle_, show);
	InitWinData();
	::SendMessage(handle_, WM_SETFONT, (WPARAM)Window::GlobalFont().GetFont(), TRUE);
	::UpdateWindow(handle_);
	currentHandle_ = oldHandle;
	//currentWindow_=oldWindow;


}

void sdf::Window::onInit() {

}

void sdf::Window::Release() {
	currentHandle_ = 0;
	currentWindow_ = nullptr;
	ReleaseUserData();
}

void sdf::Window::MessageLoop() {
	MSG msg;
	while (::GetMessage(&msg, NULL, 0, 0)) {
		//if(IsDialogMessage (msg.hwnd , &msg))continue;

		//if (WinVar::accelTable_ && TranslateAccelerator (msg.hwnd, WinVar::accelTable_, &msg))
		//continue;

		/*
		Control * form = GetUserData(msg.hwnd);
		if (form && form->GetHandle()==msg.hwnd)
		{
		if (!form->ControlProc(msg.hwnd, msg.message, msg.wParam, msg.lParam))
		continue;
		}*/
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
}

void sdf::Window::close(bool exit/*=false*/) {
	(isModal_) ? ::EndDialog(handle_, 0) : ::DestroyWindow(handle_);
	if (exit)
		::PostQuitMessage(0);
	mouseHandle_ = 0;
}

void sdf::Window::run(bool show/*=true*/) {
	onClose_ = [&] {
		close(true);
	};

	open(nullptr, show);
	MessageLoop();
}

void sdf::Window::openModal(HWND parent) {
	isModal_ = true;
	::DialogBoxParam(Control::progInstance_, tt_("class"), parent, ModalProc, (LPARAM)this);
}

float sdf::Window::getScale() {
	if (currentWindow_) {
		auto res = (float)GetDpiForWindow(currentWindow_->GetHandle()) / (float)96.0;
		if (res < 1)
			res = 1;
		return res;
	}
	else {
		auto res = (float)GetDeviceCaps(Gdi::GetScreen().GetDc(), LOGPIXELSX) / (float)96.0;
		if (res < 1)
			res = 1;
		return res;
	}

}


HBRUSH sdf::Window::OnDrawBackground() {
	if (backBrush_.GetBrush())
		return backBrush_.GetBrush();
	else
		return NULL;
}

void sdf::Window::onPaint() {
	//COUT(L"重绘");
}


void sdf::Window::AdjustLayout() {
	//COUT(tcc_("AdjustLayout") << memberList_.size());
	onLayout();
	Control::adjustRecur(this);
}

struct PopMsgStruct {
	df::String msg_;
	int transparent_;
	int time_;
};

void sdf::Window::PopMessage(const df::CC& msg, int time) {
	const TCHAR* szAppName = tt_("_PopMessage");
	static bool isReg = true;
	if (isReg) {
		isReg = false;
		WNDCLASS wndclass = { 0 };
		wndclass.style = CS_VREDRAW | CS_HREDRAW;
		wndclass.lpfnWndProc = PopMessageProc;
		wndclass.cbClsExtra = 0;
		wndclass.cbWndExtra = 0;
		wndclass.hInstance = Control::progInstance_;
		wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);;
		wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
		wndclass.hbrBackground = Brush::GetBlackBrush();//窗口背影画刷为空
		wndclass.lpszMenuName = NULL;
		wndclass.lpszClassName = szAppName;

		if (!RegisterClass(&wndclass)) {
			MessageBox(NULL, tt_("Program Need Windows NT!"), tt_("Error"), MB_ICONERROR);
			return;
		}
	}

	HWND hwnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
		szAppName,
		msg.char_,
		WS_POPUP,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		100,
		100,
		NULL,
		NULL,
		Control::progInstance_,
		NULL);
	if (hwnd == NULL) {
		DF_ERR(tcc_("CreateWindowEx failed"));
		return;
	}

	::SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) new PopMsgStruct{ msg.ToString(), 0, time });
	//::ShowWindow(hwnd, 1);
	::SetWindowPos(hwnd, 0, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOACTIVATE);

	//::UpdateWindow(hwnd);

}

LRESULT  __stdcall sdf::Window::PopMessageProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_CREATE:
		// 设置分层属性
		if (::SetTimer(hDlg, 1, 20, 0) == 0) {
			PopMsgStruct* mp = (PopMsgStruct*) ::GetWindowLongPtr(hDlg, GWLP_USERDATA);
			delete mp;
			return 0;
		}
		SetWindowLongPtr(hDlg, GWL_EXSTYLE, GetWindowLongPtr(hDlg, GWL_EXSTYLE) | WS_EX_LAYERED);
		// 设置透明度 0 - completely transparent   255 - opaque
		::SetLayeredWindowAttributes(hDlg, 0, 0, LWA_ALPHA);
		return 0;

	case WM_TIMER: {
		//wParam
		PopMsgStruct* mp = (PopMsgStruct*) ::GetWindowLongPtr(hDlg, GWLP_USERDATA);
		if (!mp)
			return 0;
		if (wParam == 1) {
			mp->transparent_ += 20;
			::SetLayeredWindowAttributes(hDlg, 0, (BYTE)mp->transparent_, LWA_ALPHA);
			if (mp->transparent_ > 160) {
				::KillTimer(hDlg, 1);
				::SetTimer(hDlg, 2, mp->time_, 0);
			}
		}
		else if (wParam == 2) {
			::KillTimer(hDlg, 2);
			::SetTimer(hDlg, 3, 20, 0);
		}
		else if (wParam == 3) {
			mp->transparent_ -= 40;
			if (mp->transparent_ < 0) {
				::KillTimer(hDlg, 3);
				::SetWindowLongPtr(hDlg, GWLP_USERDATA, 0);
				::DestroyWindow(hDlg);
				delete mp;
				return 0;
			}
			::SetLayeredWindowAttributes(hDlg, 0, (BYTE)mp->transparent_, LWA_ALPHA);
		}

		return 0;
	}
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = ::BeginPaint(hDlg, &ps);
		PopMsgStruct* mp = (PopMsgStruct*) ::GetWindowLongPtr(hDlg, GWLP_USERDATA);
		if (!mp)
			return 0;

		::SetTextColor(hdc, Color::toRGB(Color::white));
		::SetBkMode(hdc, TRANSPARENT);
		::SelectObject(hdc, Window::GlobalFont().GetFont());
		SIZE wid;
		GetTextExtentPoint32(hdc, &(mp->msg_[0]), (int)mp->msg_.size(), &wid);

		int h = Window::GlobalFont().GetFontSize() * 2;
		RECT rect{ 0, 0, wid.cx + h, h };

		::SetWindowPos(hDlg, 0, (GetScreenWidth() - rect.right) / 2, (GetScreenHeight() - h) / 2, rect.right, h,
			SWP_NOZORDER | SWP_NOCOPYBITS | SWP_NOACTIVATE);

		::DrawText(hdc, &(mp->msg_[0]), (int)mp->msg_.size(), &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

		::EndPaint(hDlg, &ps);

		return 0;
	}
	}
	return DefWindowProc(hDlg, message, wParam, lParam);
}

void sdf::Window::InitWinData() {
	gdi_.Init(handle_);
	//使用父窗口字体
	gdi_.setFont(Control::GlobalFont());
	//文字背景透明
	gdi_.SetTextBackColor();

	UpdateWinRect();
	UpdateBorderSize();
	auto oldWin = parentWindow_;
	parentWindow_ = this;
	try {
		onInit();
	} DF_CATCH_ALL;

	parentWindow_ = oldWin;

	for (auto& con : memberList_) {
		con->initCreate();
		Window::scalePos(con->pos);
	}

	initAllSub();
	//AdjustLayout();
}


////////////////////////////////Control//////////////////////////////////////////

sdf::Font& sdf::Control::GlobalFont() {
	static Font* f = 0;
	if (f == nullptr) {
		f = new Font();
		Gdi::GetScreen().setFont(*f);
	}
	return *f;
}

bool sdf::Control::PopMenu(int menuId, WinHandle hWnd) {
	HMENU hMenu = ::LoadMenu(Control::progInstance_, MAKEINTRESOURCE(menuId));
	if (!hMenu) {
		DF_ERR(tcc_("LoadMenu faild:MenuId=") << menuId);
		return false;
	}

	HMENU hSubMenu = ::GetSubMenu(hMenu, 0);
	if (!hSubMenu) {
		DF_ERR2;
		::DestroyMenu(hMenu);
		return false;
	}

	::SetMenuDefaultItem(hSubMenu, 0, TRUE);

	//CustomizeMenu(hSubMenu);

	POINT pos;
	::GetCursorPos(&pos);

	::SetForegroundWindow(hWnd.handle_);
	::TrackPopupMenu(hSubMenu, 0, pos.x, pos.y, 0, hWnd.handle_, NULL);

	//::PostMessage(hWnd, WM_NULL, 0, 0);

	::DestroyMenu(hMenu);
	return true;
}


void sdf::View::onDraw() {
	if (!parent_)
		return;

	updateDrawXY();

	if (isPress) {
	}
	else if (isDisable) {
	}
	else if (isHover) {
	}
	else {
		//COUT(tt_("重绘view"));

		Control::drawStyle(this, style, text);

		drawMember(gdi_);
	}

}


void sdf::ScrollView::onMeasure()
{

	Control::onMeasure();
	auto fontSize = Control::GlobalFont().GetFontSize();

	measureX_ = 0;
	measureY_ = 0;
	contentW = 0;
	contentH = 0;
	for (auto& sub : memberList_) {
		sub->onMeasure();
		if (pos.vector) {
			contentH += sub->showH_;
		}
		else {
			contentW += sub->showW_;
		}


	}

	if (contentW > pos.w) {
		pos.h -= getScrollWidth();
		//horiPos = 0;
		horiPage = pos.w / fontSize;
		horiMax = contentW / fontSize;
		horiRemain = contentW % fontSize;
		if (horiRemain == 0) {
			horiMax -= 1;
			horiPage -= 1;
		}
		addHoriPos(0);


	}
	else {
		horiPos = 0;
		horiPage = 0;
		horiMax = 0;
	}

	if (contentH > pos.h) {
		pos.w -= getScrollWidth();
		//vertPos = 0;
		vertPage = pos.h / fontSize;
		vertMax = contentH / fontSize;
		vertRemain = contentH % fontSize;
		if (vertRemain == 0) {
			vertMax -= 1;
			vertPage -= 1;
		}
		addVertPos(0);


	}
	else {
		vertPos = 0;
		vertPage = 0;
		vertMax = 0;
	}

	setVertScrollInfo(vertMax, vertPage);
	setHoriScrollInfo(horiPage, horiPage);

}

void sdf::ScrollView::onDraw()
{
	if (!parent_)
		return;

	COUT(tt_("scroll onDraw"));
	updateDrawXY();

	//df::TickClock([&] {
	if (isPress) {
		drawStyle(this, stylePress, text);
	}
	else if (isDisable) {
		drawStyle(this, styleDisable, text);
	}
	else if (isHover) {
		drawStyle(this, styleHover, text);
	}
	else {
		drawStyle(this, style, text);
	}
	//}, 10);
	drawMember(gdi_);
}

void sdf::ScrollView::Init() {
	onMeasure();
	handle_ = CreateWindow(
		tt_("BUTTON"),  // Predefined class; Unicode assumed
		text.c_str(),      // Button text
		WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,  // Styles  |BS_OWNERDRAW
		pos.x,         // x position
		pos.y,         // y position
		pos.w + getScrollWidth(),        // Button width
		pos.h,        // Button height
		getParentHandle(),     // Parent window
		NULL,       // No menu.
		Control::progInstance_,
		NULL);      // Pointer not needed.

	if (!handle_) {
		DF_ERR(tt_("CreateWindow Button failed!"));
		return;
	}

	Control::Init();

	gdi_.Init(handle_);
	//buttonGdi_.SetPen(Pen::GetWhitePen());
	//buttonGdi_.SetBrush(BlueBrush_);
	//buttonGdi_.SetTextColor(Color::white);
	//文字背景透明
	gdi_.SetTextBackColor();
	//背景透明
	//gdi_.SetBrush(Brush::GetNullBrush());
	//使用全局字体
	gdi_.setFont(Window::GlobalFont());

	prevMsgProc_ = (WNDPROC)SetWindowLongPtr(handle_, GWLP_WNDPROC, (LONG_PTR)Control::ButtonProc);
	initAllSub();

	setVertScrollInfo(vertMax, vertPage);
	setHoriScrollInfo(horiPage, horiPage);
}

bool sdf::ScrollView::ControlProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_MOUSEWHEEL:
	{
		int zDelta = GET_WHEEL_DELTA_WPARAM(wParam) / 40 * -1;
		//COUT("WM_MOUSEWHEEL\n" << zDelta);

		if (vertMax > 0) {
			addVertPos(zDelta);
			SetScrollPos(hDlg, SB_VERT, vertPos, true);
			onDraw();
		}
		else if (horiMax > 0) {
			addHoriPos(zDelta);
			SetScrollPos(hDlg, SB_HORZ, horiPos, true);
			onDraw();
		}

		break;
	}
	case WM_VSCROLL:
	{

		SCROLLINFO si;
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;
		GetScrollInfo(hDlg, SB_VERT, &si);
		//当前滑块的位置
		COUT("WM_VSCROLL:" << si.nPos);
		switch (wParam & 0xffff)
		{
		case SB_TOP:
			si.nPos = si.nMin;
			break;
		case SB_BOTTOM:
			si.nPos = si.nMax;
			break;
		case SB_LINEUP:
			si.nPos -= 1;
			break;
		case SB_LINEDOWN:
			si.nPos += 1;
			break;
		case SB_PAGEUP:
			si.nPos -= si.nPage;

			break;
		case SB_PAGEDOWN:
			si.nPos += si.nPage;
			break;
		case SB_THUMBTRACK:
			si.nPos = si.nTrackPos;
			break;
		default:
			break;
		}
		if (si.nPos < 0) {
			si.nPos = 0;
		}
		if (si.nPos > si.nMax - (int)si.nPage + 1) {
			si.nPos = si.nMax - (int)si.nPage + 1;
		}

		si.fMask = SIF_POS;
		vertPos = si.nPos;
		SetScrollInfo(hDlg, SB_VERT, &si, false);
		onDraw();
		//InvalidateRect(hDlg, NULL, TRUE);
	}
	break;
	}

	return false;
}

void sdf::View::Init() {
	onMeasure();
	handle_ = CreateWindow(
		tt_("STATIC"),  // Predefined class; Unicode assumed
		text.c_str(),      // Button text
		WS_VISIBLE | WS_CHILD | SS_OWNERDRAW,  // Styles  |BS_OWNERDRAW
		pos.x,         // x position
		pos.y,         // y position
		pos.w,        // Button width
		pos.h,        // Button height
		getParentHandle(),     // Parent window
		NULL,       // No menu.
		Control::progInstance_,
		NULL);      // Pointer not needed.

	if (!handle_) {
		DF_ERR(tt_("CreateWindow Button failed!"));
		return;
	}

	Control::Init();

	gdi_.Init(handle_);
	//buttonGdi_.SetPen(Pen::GetWhitePen());
	//buttonGdi_.SetBrush(BlueBrush_);
	//buttonGdi_.SetTextColor(Color::white);
	//文字背景透明
	gdi_.SetTextBackColor();
	//背景透明
	//gdi_.SetBrush(Brush::GetNullBrush());
	//使用全局字体
	gdi_.setFont(Window::GlobalFont());

	prevMsgProc_ = (WNDPROC)SetWindowLongPtr(handle_, GWLP_WNDPROC, (LONG_PTR)Control::ButtonProc);
	initAllSub();
}

////////////////////////////////Button//////////////////////////////////////////

void sdf::Button::Init() {
	onMeasure();

	DWORD sty = WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON;

	if (!oldStyle) {
		sty |= BS_OWNERDRAW;
	}
	handle_ = CreateWindow(
		tt_("BUTTON"),  // Predefined class; Unicode assumed
		text.c_str(),      // Button text
		sty,  // Styles  |BS_OWNERDRAW
		pos.x,         // x position
		pos.y,         // y position
		pos.w,        // Button width
		pos.h,        // Button height
		getParentHandle(),     // Parent window
		NULL,       // No menu.
		Control::progInstance_,
		NULL);      // Pointer not needed.

	if (!handle_) {
		DF_ERR(tt_("CreateWindow Button failed!"));
		return;
	}

	Control::Init();

	buttonGdi_.Init(handle_);
	//buttonGdi_.SetPen(Pen::GetWhitePen());
	//buttonGdi_.SetBrush(BlueBrush_);
	//buttonGdi_.SetTextColor(Color::white);
	//文字背景透明
	buttonGdi_.SetTextBackColor();
	//背景透明
	//buttonGdi_.SetBrush(Brush::GetNullBrush());
	//使用全局字体
	buttonGdi_.setFont(Window::GlobalFont());

	prevMsgProc_ = (WNDPROC)SetWindowLongPtr(handle_, GWLP_WNDPROC, (LONG_PTR)Control::ButtonProc);
	initAllSub();
}

bool sdf::Button::ControlProc(HWND, UINT msg, WPARAM, LPARAM lParam) {
	switch (msg) {
	case WM_VSCROLL:
	{
		printf("BUtton WM_VSCROLL\n");
		break;
	}
	case WM_COMMAND: {

		if (onClick_)
			onClick_();
		break;
	}
	}
	return true;
}


bool sdf::Control::drawStyle(Control* cont, ControlStyle& style, const String& text) {

	int32_t drawX = cont->getDrawX();
	int32_t drawY = cont->getDrawY();


	RECT rect;
	rect.left = drawX;
	rect.top = drawY;
	rect.right = drawX + cont->GetWidth();
	rect.bottom = drawY + cont->GetHeight();

	int w = cont->GetWidth();
	int h = cont->GetHeight();

	if (buttonBmp_.GetWidth() < drawX + w || buttonBmp_.GetHeight() < drawY + h) {
		auto oldBmp = std::move(buttonBmp_);
		buttonBmpBuf_ = buttonBmp_.CreateDib(max(drawX + w, oldBmp.GetWidth()), max(drawY + h, oldBmp.GetHeight()));
		buttonBmp_.SetTextBackColor();
		oldBmp.DrawTo(buttonBmp_);
		Gdip::Init();
		if (graph_)
			delete graph_;
		graph_ = Gdiplus::Graphics::FromHDC(buttonBmp_.GetDc());
		graph_->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

	}
	int32_t* buf = (int32_t*)buttonBmpBuf_;
	if (buf == nullptr)
		return false;
	int bufW = buttonBmp_.GetWidth();



	//画阴影
	int shadowLen = std::abs(style.shadowSize);

	if (shadowLen > 0 && shadowLen < 1023) {
		DWORD col[1024] = { 0 };//渐变

		for (int i = 1; i <= shadowLen; i++) {
			col[i - 1] = Color::mixColor(style.backColor, 7 * i);
		}

		//横向阴影
		for (int y = drawY; y < drawY + shadowLen; y++) {
			for (int i = y - drawY + drawX; i < rect.right; i++) {
				//buf[y * bufW + i] = Color::black;
				if (y * bufW + i > 0)
					buf[y * bufW + i] = col[shadowLen - 1 - (y - drawY)];
			}
		}
		//纵向
		for (int y = drawX; y < drawX + shadowLen; y++) {
			for (int i = y - drawX + drawY; i < rect.bottom; i++) {
				//buf[y + i * bufW] = Color::black;
				if (y + i * bufW > 0)
					buf[y + i * bufW] = col[shadowLen - 1 - (y - drawX)];
			}
		}
		rect.top += 2;
		rect.left += 2;
	}

	if (cont->isFocused) {
		//but.buttonGdi_.SetPen(WhiteDotPen_);
		//but.buttonGdi_.SetBrush(Brush::GetNullBrush());
		//but.buttonGdi_.Rect(but.buttonRect_.left + 3, but.buttonRect_.top + 3, but.buttonRect_.right - 3, but.buttonRect_.bottom - 3);
	}


	if (style.backColor != 0) {
		//画矩形
		for (int i = shadowLen + drawY; i < rect.bottom - style.borderBottom; i++) {
			for (int y = shadowLen + drawX; y < rect.right - style.borderRight; y++) {
				if (i * bufW + y >= 0)
					buf[i * bufW + y] = style.backColor;
			}
		}
	}

	if (style.borderTop > 0) {
		for (int y = drawY; y < drawY + style.borderTop; y++) {
			for (int i = drawX; i < drawX + w; i++) {
				if (y * bufW + i >= 0)
					buf[y * bufW + i] = style.borderColor;
			}
		}
	}
	if (style.borderBottom > 0 && h > 0) {
		for (int y = drawY + h - style.borderBottom; y < drawY + h; y++) {
			for (int i = drawX; i < drawX + w; i++) {
				if (y * bufW + i >= 0)
					buf[y * bufW + i] = style.borderColor;
			}
		}
	}
	if (style.borderLeft > 0) {
		for (int y = drawX; y < drawX + style.borderLeft; y++) {
			for (int i = drawY; i < drawY + h; i++) {
				if (y + i * bufW >= 0)
					buf[y + i * bufW] = style.borderColor;
			}
		}
	}
	if (style.borderRight > 0 && w > 0) {
		for (int y = drawX + w - style.borderRight; y < drawX + w; y++) {
			for (int i = drawY; i < drawY + h; i++) {
				if (y + i * bufW >= 0)
					buf[y + i * bufW] = style.borderColor;
			}
		}
	}




	if (style.backImage) {
		int32_t nW = w, nH = h;
		int32_t sX = drawX, sY = drawY;
		float ratio = (float)w / h;
		float ratioImg = (float)style.backImage->GetWidth() / style.backImage->GetHeight();

		if (style.scaleType == BitmapScaleType::center) {
			if (ratio > ratioImg) {
				nW = (int32_t)(ratioImg * nH);
				sX = drawX + (w - nW) / 2;
			}
			else {
				nH = (int32_t)(nW / ratioImg);
				sY = drawY + (h - nH) / 2;
			}
			if (style.backImage->alpha()) {
				style.backImage->DrawAlphaTo(buttonBmp_, sX, sY, nW, nH);
			}
			else {
				style.backImage->DrawStretchTo(buttonBmp_, sX, sY, nW, nH);

			}
		}
		else if (style.scaleType == BitmapScaleType::centerClip) {
			if (ratio > ratioImg) {
				nH = (int32_t)(nW / ratioImg);
				sY = ((nH - h)) * style.backImage->GetHeight() / nH;
			}
			else {
				nW = (int32_t)(ratioImg * nH);
				sX = ((nW - w)) * style.backImage->GetWidth() / nW;

			}
			if (style.backImage->alpha()) {
				style.backImage->DrawAlphaTo(buttonBmp_, drawX, drawY, w, h, style.backImage->GetWidth() - sX,
					style.backImage->GetHeight() - sY, sX / 2, sY / 2);
			}
			else {
				style.backImage->DrawStretchTo(buttonBmp_, drawX, drawY, w, h, style.backImage->GetWidth() - sX,
					style.backImage->GetHeight() - sY, sX / 2, sY / 2);

			}
		}
		else {
			if (style.backImage->alpha()) {
				style.backImage->DrawAlphaTo(buttonBmp_, sX, sY, nW, nH);
			}
			else {
				style.backImage->DrawStretchTo(buttonBmp_, sX, sY, nW, nH);
			}
		}


	}

	if (text.length() > 0) {
		buttonBmp_.setFont(Control::GlobalFont());
		buttonBmp_.SetTextColor(style.color);
		cont->onDrawText(rect);
	}

	return false;
}

void sdf::Button::onDraw() {
	if (oldStyle) {
		return;
	}
	if (!parent_)
		return;

	updateDrawXY();

	//df::TickClock([&] {
	if (isPress) {
		drawStyle(this, stylePress, text);
	}
	else if (isDisable) {
		drawStyle(this, styleDisable, text);
	}
	else if (isHover) {
		drawStyle(this, styleHover, text);
	}
	else if (isCheck) {
		drawStyle(this, styleCheck, text);
	}
	else {
		drawStyle(this, style, text);
	}
	//}, 10);

	drawMember(buttonGdi_);


}


////////////////////////////////////TextBox//////////////////////////////////////

void sdf::TextBox::Init() {
	onMeasure();

	DWORD sty = WS_TABSTOP | WS_CHILD | WS_VISIBLE |
		ES_LEFT | WS_BORDER;

	if (mutiLine) {
		sty |= ES_MULTILINE | WS_BORDER | WS_VSCROLL | ES_AUTOVSCROLL;
	}
	else {
		sty |= ES_AUTOHSCROLL;
	}
	if (onlyNumber) {
		sty |= ES_NUMBER;
	}
	if (readOnly) {
		sty |= ES_READONLY;
	}
	if (password) {
		sty |= ES_PASSWORD;
	}

	handle_ = CreateWindow(
		tt_("EDIT"),  // Predefined class; Unicode assumed
		text.c_str(),      // Button text
		sty,   // Styles  |BS_OWNERDRAW
		pos.x,         // x position
		pos.y,         // y position
		pos.w,        // Button width
		pos.h,        // Button height
		getParentHandle(),     // Parent window
		NULL,       // No menu.
		Control::progInstance_,
		NULL);      // Pointer not needed.

	if (!handle_) {
		DF_ERR(tt_("CreateWindow Button failed!"));
		return;
	}

	Control::Init();

	//gdi_.Init(handle_);
	//gdi_.SetPen(Pen::GetWhitePen());
	//gdi_.SetBrush(BlueBrush_);
	//gdi_.SetTextColor(Color::white);
	//文字背景透明
	//gdi_.SetTextBackColor();
	//背景透明
	//gdi_.SetBrush(Brush::GetNullBrush());
	setFont(Window::GlobalFont());

	prevMsgProc_ = (WNDPROC)SetWindowLongPtr(handle_, GWLP_WNDPROC, (LONG_PTR)Control::ButtonProc);
	initAllSub();
}

void sdf::TextBox::initCreate() {
	pos.w = Control::GlobalFont().getRawSize() * 5;
	pos.h = Control::GlobalFont().getRawSize() + 5;
	doCreate(this);
}


bool sdf::TextBox::ControlProc(HWND, UINT msg, WPARAM wParam, LPARAM) {
	if (msg == WM_COMMAND) {
		switch (HIWORD(wParam)) {
		case EN_CHANGE:
			if (onChange_)
				onChange_();
			break;
		case EN_SETFOCUS:
			if (onFocus_)
				onFocus_();
			break;
		case EN_KILLFOCUS:
			if (onLeave_)
				onLeave_();
			break;
		}
	}
	return true;
}


/////////////////////////////////ListBox/////////////////////////////////////////

void sdf::ListBox::initCreate() {
	doCreate(this);
}

void sdf::ListBox::Init() {
	onMeasure();

	DWORD sty = WS_TABSTOP | WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | WS_HSCROLL;

	if (mutiColumn) {
		sty |= LBS_MULTICOLUMN;
	}

	if (mutiSelect) {
		sty |= LBS_MULTIPLESEL;
	}

	handle_ = CreateWindow(
		tt_("LISTBOX"),  // Predefined class; Unicode assumed
		text.c_str(),      // Button text
		sty,   // Styles  |BS_OWNERDRAW
		pos.x,         // x position
		pos.y,         // y position
		pos.w,        // Button width
		pos.h,        // Button height
		getParentHandle(),     // Parent window
		NULL,       // No menu.
		Control::progInstance_,
		NULL);      // Pointer not needed.

	if (!handle_) {
		DF_ERR(tt_("CreateWindow Button failed!"));
		return;
	}

	Control::Init();

	if (initList) {
		for (auto& s : *initList) {
			add(s);
		}
		initList.reset();
	}
	//gdi_.Init(handle_);
	//gdi_.SetPen(Pen::GetWhitePen());
	//gdi_.SetBrush(BlueBrush_);
	//gdi_.SetTextColor(Color::white);
	//文字背景透明
	//gdi_.SetTextBackColor();
	//背景透明
	//gdi_.SetBrush(Brush::GetNullBrush());
	setFont(Window::GlobalFont());

	prevMsgProc_ = (WNDPROC)SetWindowLongPtr(handle_, GWLP_WNDPROC, (LONG_PTR)Control::ButtonProc);
	initAllSub();
}

bool sdf::ListBox::ControlProc(HWND, UINT msg, WPARAM wParam, LPARAM) {
	if (msg == WM_COMMAND) {
		switch (HIWORD(wParam)) {
		case LBN_SELCHANGE:
			if (onClick_)
				onClick_();
			break;
		}
	}
	return true;
}


void sdf::ListBox::SetHorizontal(const df::CC& str) {
	SIZE wid;
	GetTextExtentPoint32(currentWindow_->gdi_.GetDc(), str.char_, (int)str.length_, &wid);
	COUT(wid.cx);
	SendMessage(handle_, LB_SETHORIZONTALEXTENT, (WPARAM)(wid.cx + 5), 0);
	textLength_ = (int)str.length_;
}



///////////////////////////////////ComBox///////////////////////////////////////

void sdf::ComBox::initCreate() {
	pos.w = Control::GlobalFont().getRawSize() * 5;
	pos.h = Control::GlobalFont().getRawSize() + 7;
	//pos.marginBottom = 3;
	doCreate(this);
}

void sdf::ComBox::Init() {
	onMeasure();

	DWORD sty = WS_TABSTOP | WS_CHILD | WS_VISIBLE | CBS_AUTOHSCROLL;


	if (editAble)
		sty |= CBS_DROPDOWN;
	else
		sty |= CBS_DROPDOWNLIST;

	handle_ = CreateWindow(
		tt_("COMBOBOX"),  // Predefined class; Unicode assumed
		text.c_str(),      // Button text
		sty,   // Styles  |BS_OWNERDRAW
		pos.x,         // x position
		pos.y,         // y position
		pos.w,        // Button width
		pos.h,        // Button height
		getParentHandle(),     // Parent window
		NULL,       // No menu.
		Control::progInstance_,
		NULL);      // Pointer not needed.

	if (!handle_) {
		DF_ERR(tt_("CreateWindow COMBOBOX failed!"));
		return;
	}

	Control::Init();

	if (initList) {
		for (auto& s : *initList) {
			add(s);
		}
		initList.reset();
	}
	//gdi_.Init(handle_);
	//gdi_.SetPen(Pen::GetWhitePen());
	//gdi_.SetBrush(BlueBrush_);
	//gdi_.SetTextColor(Color::white);
	//文字背景透明
	//gdi_.SetTextBackColor();
	//背景透明
	//gdi_.SetBrush(Brush::GetNullBrush());
	setFont(Window::GlobalFont());

	prevMsgProc_ = (WNDPROC)SetWindowLongPtr(handle_, GWLP_WNDPROC, (LONG_PTR)Control::ButtonProc);
	initAllSub();

}

bool sdf::ComBox::ControlProc(HWND, UINT msg, WPARAM wParam, LPARAM) {
	if (msg == WM_COMMAND) {
		switch (HIWORD(wParam)) {
		case CBN_SELCHANGE:
			if (onSelectChange_)
				onSelectChange_();
			break;
		case CBN_EDITCHANGE:
			if (onChange_)
				onChange_();
			break;
		}
	}
	return true;
}

/////////////////////////////////CheckBox/////////////////////////////////////////





void sdf::CheckBox::onDrawText(RECT& rect)
{
	int32_t drawX = getDrawX() + pos.paddingLeft;
	int32_t drawY = getDrawY() + pos.paddingTop;

	int32_t size = (int32_t)(1 * Window::getScale());
	int32_t w = pos.h - pos.paddingTop - pos.paddingBottom;
	int bufW = buttonBmp_.GetWidth();
	int32_t* buf = (int32_t*)buttonBmpBuf_;

	uint32_t borderColor = style.borderColor;
	if (isCheck || isHover || isPress || isFocused) {
		borderColor = styleHover.borderColor;
	}

	if (isDisable) {
		if (dot) {
			Gdiplus::Pen bluePen(Gdiplus::Color(styleDisable.borderColor), (float)size * 2);
			// Create a Rect object that bounds the ellipse.
			Gdiplus::RectF ellipseRect((float)drawX, (float)drawY, (float)w, (float)w);
			graph_->DrawEllipse(&bluePen, ellipseRect);
		}
		else {
			//横
			for (int y = drawY; y < drawY + size; y++) {
				for (int i = drawX + size; i < drawX + w - size; i++) {
					if (y * bufW + i >= 0)
						buf[y * bufW + i] = styleDisable.borderColor;
					if ((y + w - size) * bufW + i >= 0)
						buf[(y + w - size) * bufW + i] = styleDisable.borderColor;
				}
			}
			//竖
			for (int x = drawX; x < drawX + size; x++) {
				for (int y = drawY + size; y < drawY + w - size; y++) {
					if (x + y * bufW >= 0)
						buf[x + y * bufW] = styleDisable.borderColor;
					if (x + w - size + y * bufW >= 0)
						buf[x + w - size + y * bufW] = styleDisable.borderColor;
				}
			}
		}

	}
	else if (isCheck) {
		if (dot) {
			Gdiplus::Pen bluePen(Gdiplus::Color(borderColor), (float)size * 2);
			// Create a Rect object that bounds the ellipse.
			Gdiplus::RectF ellipseRect((float)drawX, (float)drawY, (float)w, (float)w);
			graph_->DrawEllipse(&bluePen, ellipseRect);

			float dotSize = (float)w / 2;
			Gdiplus::SolidBrush brush(Gdiplus::Color((Gdiplus::ARGB)borderColor));
			Gdiplus::RectF ellipseRect2(drawX + (w - dotSize) / 2, drawY + (w - dotSize) / 2, dotSize, dotSize);
			graph_->FillEllipse(&brush, ellipseRect2);
		}
		else {

			//横
			for (int y = drawY; y < drawY + size; y++) {
				for (int i = drawX + size; i < drawX + w - size; i++) {
					if (y * bufW + i >= 0)
						buf[y * bufW + i] = borderColor;
					if ((y + w - size) * bufW + i >= 0)
						buf[(y + w - size) * bufW + i] = borderColor;
				}
			}

			//中间
			for (int y = drawY + size; y < drawY + w - size; y++) {
				for (int i = drawX; i < drawX + w; i++) {
					if (y * bufW + i >= 0)
						buf[y * bufW + i] = borderColor;
				}
			}

			Gdiplus::Pen pen(Gdiplus::Color(Color::white), 2 * Window::getScale());
			graph_->DrawLine(&pen, drawX + w * 0.20f, drawY + w * 0.42f, drawX + w * 0.40f, drawY + w * 0.67f);
			graph_->DrawLine(&pen, drawX + w * 0.38f, drawY + w * 0.66f, drawX + w * 0.8f, drawY + w * 0.28f);
		}
	}
	else {
		if (dot) {

			Gdiplus::Pen bluePen(Gdiplus::Color(borderColor), (float)size * 2);
			// Create a Rect object that bounds the ellipse.
			Gdiplus::RectF ellipseRect((float)drawX, (float)drawY, (float)w, (float)w);
			graph_->DrawEllipse(&bluePen, ellipseRect);
			//buttonBmp_.Round(drawX, drawY, drawX + w, drawY + w);
		}
		else {
			//横
			for (int y = drawY; y < drawY + size; y++) {
				for (int i = drawX + size; i < drawX + w - size; i++) {
					if (y * bufW + i >= 0)
						buf[y * bufW + i] = borderColor;
					if ((y + w - size) * bufW + i >= 0)
						buf[(y + w - size) * bufW + i] = borderColor;
				}
			}
			//竖
			for (int x = drawX; x < drawX + size; x++) {
				for (int y = drawY + size; y < drawY + w - size; y++) {
					if (x + y * bufW >= 0)
						buf[x + y * bufW] = borderColor;
					if (x + w - size + y * bufW >= 0)
						buf[x + w - size + y * bufW] = borderColor;
				}
			}
		}


	}
	rect.left = drawX + w;
	rect.top = getDrawY();
	buttonBmp_.Txt(rect, text);
}

bool sdf::CheckBox::ControlProc(HWND, UINT msg, WPARAM wParam, LPARAM) {
	if (msg == WM_COMMAND) {
		switch (HIWORD(wParam)) {
		case BN_CLICKED:
			CheckGroup* check = dynamic_cast <CheckGroup*>(parent_);
			if (check) {
				check->setCheck(this);
			}
			else {
				isCheck = !isCheck;
			}

			if (onClick_)
				onClick_();
			break;
		}
	}
	return true;
}


/////////////////////////////////Tray/////////////////////////////////////////

std::function<void()>& sdf::Tray::OnLeftClick() {
	static std::function<void()> func = []() {};
	return func;
}

std::function<void()>& sdf::Tray::OnRightClick() {
	static std::function<void()> func = []() {};
	return func;
}

//////////////////////////////////////////////////////////////////////////

bool sdf::Bitmap::Load(const df::CC& name) {
	Gdip::Init();
	const WCHAR* wName = (const WCHAR*)name.char_;
#ifndef UNICODE
	auto wStr = df::code::AnsiToWide(name);
	wName = wStr.c_str();
#endif // !UNICODE

	Gdiplus::Image* img = new Gdiplus::Image(wName);

	ON_SCOPE_EXIT({
					  delete img;
		});

	if (img == nullptr || img->GetLastStatus() != Gdiplus::Ok) {
		DF_ERR(name << tcc_(" Gdiplus Load Image failed!"));
		return false;
	}
	auto alpha = img->GetPixelFormat() & PixelFormatAlpha;
	hasAlpha = !!alpha;

	if (CreateDib(img->GetWidth(), img->GetHeight()) == nullptr)
		return false;

	Gdiplus::Graphics g(hdc_);
	g.DrawImage(img, 0, 0, width_, height_);
	return true;

}

bool sdf::Bitmap::LoadBMP(int id) {
	Init();
	img_ = ::LoadBitmap(Control::progInstance_, MAKEINTRESOURCE(id));
	if (img_ == 0) {
		DF_ERR(tcc_("LoadBitmap failed,id:") << id);
		return false;
	}
	GetHW();
	return ::SelectObject(hdc_, img_) != NULL;
}

bool sdf::Bitmap::Load(int id, const df::CC& resType /*= tcc_("png")*/) {
	Gdip::Init();
	HRSRC hRsrc = ::FindResource(Control::progInstance_, MAKEINTRESOURCE(id), resType.char_);
	if (!hRsrc) {
		DF_ERR(tcc_("FindResource failed"))
			return false;
	}
	// load resource into memory
	DWORD len = ::SizeofResource(Control::progInstance_, hRsrc);
	HGLOBAL lpRsrc = ::LoadResource(Control::progInstance_, hRsrc);
	ON_SCOPE_EXIT({
					  ::FreeResource(lpRsrc);
		});

	///重新申请一块内存
	HGLOBAL m_hMem = GlobalAlloc(GMEM_FIXED, len);
	BYTE* pmem = (BYTE*)GlobalLock(m_hMem);
	memcpy(pmem, lpRsrc, len);

	IStream* pstm;
	CreateStreamOnHGlobal(m_hMem, FALSE, &pstm);
	// load from stream
	Gdiplus::Image* img = Gdiplus::Image::FromStream(pstm);

	ON_SCOPE_EXIT({
					  delete img;
					  ::GlobalUnlock(m_hMem);
					  pstm->Release();
					  ::GlobalFree(m_hMem);
		});

	if (img == nullptr || img->GetLastStatus() != Gdiplus::Ok) {
		DF_ERR(id << tcc_(" Gdiplus Load Image failed! type:") << resType);
		return false;
	}

	if (CreateDib(img->GetWidth(), img->GetHeight()) == nullptr)
		return false;

	Gdiplus::Graphics g(hdc_);
	g.DrawImage(img, 0, 0, width_, height_);

	return true;
}

char* sdf::Bitmap::CreateDib(int w, int h) {
	Init();
	////////////////////
	BITMAPINFO info = { {0} };
	info.bmiHeader.biSize = sizeof(info.bmiHeader);
	info.bmiHeader.biWidth = w;
	//info.bmiHeader.biHeight        = h;
	info.bmiHeader.biHeight = -h;
	info.bmiHeader.biPlanes = 1;
	info.bmiHeader.biBitCount = 32;
	info.bmiHeader.biCompression = BI_RGB;
	info.bmiHeader.biSizeImage = w * h * (32 / 8);

	// 创建一块内存纹理并获取其数据指针
	void* pBits = NULL;
	img_ = ::CreateDIBSection(hdc_, &info, DIB_RGB_COLORS, &pBits, NULL, 0);
	if (img_ == 0) {
		DF_ERR(tcc_("CreateDIBSection failed"));
		return nullptr;
	}
	::SelectObject(hdc_, img_);
	//DIBSECTION GDIBSection;
	//::GetObject(img_ , sizeof(DIBSECTION), &GDIBSection);

	width_ = w;
	height_ = h;
	//这里指向图像的内存区域
	//return (char *)GDIBSection.dsBm.bmBits;
	imgBuf_ = (char*)pBits;
	return imgBuf_;
}
