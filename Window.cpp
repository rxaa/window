#include "StdAfx.h"

#include "Control.h"


HWND sdf::Control::currentHandle_ = NULL;
sdf::Window* sdf::Control::parentWindow_ = NULL;

HINSTANCE sdf::Control::progInstance_ = nullptr;

sdf::Window* sdf::Control::currentWindow_ = nullptr;


sdf::Control* sdf::Control::mouseHandle_ = 0;


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

void sdf::Control::measureWrapX(int32_t minW) {
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
	if (subW < minW)
		subW = minW;

	pos.w = subW + pos.paddingLeft + pos.paddingRight;
}

void sdf::Control::measureWrapY(int32_t minH) {
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
	if (subH < minH)
		subH = minH;
	pos.h = subH + pos.paddingTop + pos.paddingBottom;
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
		//wrap,提前计算所有子成员
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
				pos.w = (int32_t)std::round((parent_->pos.w - parent_->pos.paddingLeft - parent_->pos.paddingRight - flexWsum) * pos.flexX /
					(float)flexXsum) - pos.marginLeft - pos.marginRight;
		}
	}
	else if (pos.wrapX) {
		measureWrapX(w);
	}
	else {

	}

	if (pos.flexY > 0) {

		if (parent_->pos.h > 0) {
			if (parent_->pos.vector)
				pos.h = (int32_t)std::round((parent_->pos.h - flexHsum - parent_->pos.paddingTop - parent_->pos.paddingBottom) * pos.flexY /
					(float)flexYsum) - pos.marginTop - pos.marginBottom;
			else
				pos.h = parent_->pos.h - pos.marginTop - pos.marginBottom - parent_->pos.paddingTop -
				parent_->pos.paddingBottom;
		}
	}
	else if (pos.wrapY) {
		measureWrapY(h);
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

	if (pos.h >= 0)
		pos.h = (int16_t)(rect.bottom);

	if (pos.w >= 0)
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

void sdf::Control::doCreate() {

	if (onCreate_) {
		onCreate_();
		onCreate_ = nullptr;
		if (pos.w < 0 && pos.flexX < 1)
			pos.wrapX = true;

		if (pos.h < 0 && pos.flexY < 1)
			pos.wrapY = true;
	}
	for (auto& con : memberList_) {
		con->doCreate();
	}
	Window::scalePos(pos);


}


////////////////////////////////Brush//////////////////////////////////////////

void sdf::Brush::SetFromBitmap(Bitmap& bmp) {
	ReleaseBrush();
	brush_ = CreatePatternBrush(bmp.GetBitmap());
}

////////////////////////////////Window//////////////////////////////////////////



void sdf::Control::drawMember(Gdi& gdi, DrawBuffer* draw) {
	needDraw = false;
	ON_SCOPE_EXIT({
					  needDraw = true;
		});
	for (auto& sub : memberList_) {

		if (sub->showOverflow()) {
			continue;
		}
		/*int32_t hp = getHoriPos();
		int32_t vp = getVertPos();

		if (sub->pos.y - vp > GetHeight() && sub->showY_ > GetHeight()) {
			continue;
		}
		if (sub->pos.y - vp + sub->pos.h < 0 && sub->showY_ + sub->pos.h < 0) {
			continue;
		}*/
		sub->onDraw();
	}
	if (parent_->needDraw && draw) {
		gdi.DrawFrom(draw->buttonBmp_, 0, 0, pos.w, pos.h, drawX_, drawY_);
	}

}

LRESULT  __stdcall sdf::Control::ButtonProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {

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
	Control* cont = GetUserData(hDlg);
	if (cont) {

		auto contRes = cont->ControlProc(hDlg, message, wParam, lParam);
		if (!contRes)
			return  0;
	}


	auto res = controlComProc(hDlg, message, wParam, lParam);
	if (res) {
		return res;
	}

	if (cont && cont->prevMsgProc_)
		return CallWindowProc(cont->prevMsgProc_, hDlg, message, wParam, lParam);
	return DefWindowProc(hDlg, message, wParam, lParam);
}

//父子公用消息处理
intptr_t sdf::Control::controlComProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	//COUT((int*)message);
	sdf::Control* cont = sdf::Window::GetUserData(hDlg);
	switch (message) {
	case WM_CTLCOLOREDIT: {
		//文本样式
		HDC hdc = (HDC)wParam;
		HWND hwnd = (HWND)lParam;
		if (cont->style.color) {
			SetTextColor(hdc, Color::toRGB(Color::red));
			//SetBkMode(hdc, TRANSPARENT);
			return (intptr_t)Brush::GetWhiteBrush();
		}
		return 0;
	}
	case WM_CTLCOLORBTN: {
		//绘制控件背景,(自绘控件仍触发)
		//Control* controlP = GetUserData((HWND)lParam);
		//::SetBkMode((HDC)wParam, TRANSPARENT);
		//返回空背景
		return (intptr_t)Brush::GetNullBrush();
	}
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
			/*if (controlP->showOverflow()) {
				break;
			}*/
			controlP->msgDraw = true;
			controlP->onDraw();
			controlP->msgDraw = false;
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
		case WM_CREATE: {

			break;
		}
		case WM_CLOSE: {
			winP->closeRelease();
			break;
		}

		case WM_COMMAND: {
			auto l = LOWORD(wParam);
			//auto h = HIWORD(wParam);

			if (l == IDCANCEL) {
				winP->closeRelease();
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
		case WM_ACTIVATE: {
			if (wParam == WA_INACTIVE) {
				winP->onInActive();
			}
			else {
				winP->onActive();
			}
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

		case Tray::TRAY_MESSAGE: {
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
		}
		case WM_CTLCOLORSTATIC: {
			//::SetBkMode((HDC)wParam, TRANSPARENT);
			break;
		}
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

		auto res = controlComProc(hDlg, message, wParam, lParam);
		if (res)
			return res;

	} DF_CATCH_ALL;
	return DefWindowProc(hDlg, message, wParam, lParam);
}

void sdf::Window::setPosXY() {
	if (pos._centerInParentX) {
		if (parent_) {
			pos.x = parent_->pos.x + (parent_->pos.w - pos.w) / 2;
		}
		else {
			pos.x = (Window::GetScreenWidth() - pos.w) / 2;
		}

	}
	if (pos._centerInParentY) {
		if (parent_) {
			pos.y = parent_->pos.y + (parent_->pos.h - pos.h) / 2;
		}
		else {
			pos.y = (Window::GetScreenHeight() - pos.h) / 2;
		}
	}
}

void sdf::Window::openRaw(HWND parent/*=0*/, bool show) {


	onCreate();

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

	Window::scalePos(pos, false);

	setPosXY();
	DWORD sty = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE;

	//WS_EX_TRANSPARENT;
	DWORD styEX = 0;

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

	if (noBorder) {
		sty = WS_POPUP | WS_VISIBLE | WS_SYSMENU;

		//不显示在任务栏
		styEX |= WS_EX_TOOLWINDOW;
	}

	handle_ = CreateWindowEx(styEX,
		className,
		text.c_str(),
		sty,
		pos.x,
		pos.y,
		pos.w,
		pos.h,
		parent,
		NULL,
		Control::progInstance_,
		NULL);

	if (parent_) {
		parent_->enable(false);
	}
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

	ptr_ = sharedBase<Window>();

	if (pos.w > 0 && pos.h > 0)
		drawBuff_->newBmp(pos.w, pos.h);

	InitWinData();
	::SendMessage(handle_, WM_SETFONT, (WPARAM)Window::GlobalFont().GetFont(), TRUE);

	if (pos.w < 0 || pos.h < 0) {
		if (pos.w < 0)
			measureWrapX(0);
		if (pos.h < 0)
			measureWrapY(0);
		setPosXY();
		if (pos.x + pos.w + borderSize_ > Window::GetScreenWidth()) {
			pos.x = Window::GetScreenWidth() - pos.w - borderSize_;
		}
		if (pos.y + pos.h + borderSize_ > Window::GetScreenHeight()) {
			pos.y = Window::GetScreenHeight() - pos.h - borderSize_;
		}

		::SetWindowPos(handle_, 0, pos.x, pos.y, pos.w + borderSize_, pos.h + titleHeight_ + borderSize_,
			SWP_NOZORDER | SWP_NOCOPYBITS);
	}



	::ShowWindow(handle_, show);
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

void sdf::Window::closeRelease() {
	ON_SCOPE_EXIT({
					  ptr_.reset();
		});
	if (parent_) {
		parent_->enable(true);
	}
	mouseHandle_ = 0;
	::DestroyWindow(handle_);
	if (isMain)
		::PostQuitMessage(0);
	onClose();
}

void sdf::Window::run(bool show/*=true*/) {
	isMain = true;
	open(nullptr, show);
	MessageLoop();
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
	update();
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
		con->doCreate();
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


	if (isPress) {
	}
	else if (isDisable) {
	}
	else if (isHover) {
	}
	else {

		if (msgDraw && !parent_->isTop) {
			//作为子成员无需单独绘制
			return;
		}
		auto draw = getDraw();

		updateDrawXY();

		//COUT(tt_("重绘view"));
		if (drawStyle(draw, style)) {

			drawMember(gdi_, draw);
		}


	}

}


void sdf::ScrollView::onMeasure() {

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
			if (sub->showW_ > contentW)
				contentW = sub->showW_;
		}
		else {
			contentW += sub->showW_;
			if (sub->showH_ > contentH)
				contentH = sub->showH_;
		}


	}
	int w = pos.w, h = pos.h;

	if (contentW > pos.w) {
		pos.h = h - getScrollWidth();
	}
	if (contentH > pos.h) {
		pos.w = w - getScrollWidth();
	}

	if (contentW > pos.w) {
		pos.h = h - getScrollWidth();
		//horiPos = 0;
		horiPage = pos.w / fontSize;
		if (pos.w % fontSize == 0)
			horiPage -= 1;

		horiMax = contentW / fontSize;
		horiRemain = contentW % fontSize;
		if (horiRemain == 0)
			horiMax -= 1;

		addHoriPos(0);

	}
	else {
		horiPos = 0;
		horiPage = 0;
		horiMax = 0;
	}

	if (contentH > pos.h) {
		pos.w = w - getScrollWidth();
		//vertPos = 0;
		vertPage = pos.h / fontSize;
		if (pos.h % fontSize == 0)
			vertPage -= 1;
		vertMax = contentH / fontSize;
		vertRemain = contentH % fontSize;
		if (vertRemain == 0)
			vertMax -= 1;


		addVertPos(0);

	}
	else {
		vertPos = 0;
		vertPage = 0;
		vertMax = 0;
	}


	setVertScrollInfo(vertMax, vertPage);
	setHoriScrollInfo(horiMax, horiPage);

}

void sdf::ScrollView::onDraw() {
	if (!parent_)
		return;


	if (msgDraw && !parent_->isTop) {
		//作为子成员无需单独绘制
		return;
	}

	updateDrawXY();

	COUT(tt_("scroll onDraw"));

	DrawBuffer* draw = getDraw();

	bool drawSub = true;

	if (isPress) {
		drawSub = drawStyle(draw, stylePress, true);
		return;
	}
	if (isDisable) {
		drawSub = drawStyle(draw, styleDisable, true);
	}
	else if (isHover) {
		drawSub = drawStyle(draw, styleHover, true);
	}
	else {
		drawSub = drawStyle(draw, style, true);
	}

	if (drawSub) {
		needDraw = false;
		//df::TickClock([&] {
		for (auto& sub : memberList_) {

			if (sub->showOverflow())
				continue;


			sub->onDraw();
		}
		//	}, 1);

		needDraw = true;
		if (draw) {
			DrawBuffer* drawParent = parent_->getDraw();
			if (drawParent)
				drawParent->buttonBmp_.DrawFrom(draw->buttonBmp_, drawX_, drawY_, pos.w, pos.h, drawX_, drawY_);
			if (parent_->needDraw) {
				gdi_.DrawFrom(draw->buttonBmp_, 0, 0, pos.w, pos.h, drawX_, drawY_);
			}
		}

	}





}

void sdf::ScrollView::Init() {
	onMeasure();
	handle_ = CreateWindow(
		tt_("BUTTON"),  // Predefined class; Unicode assumed
		text.c_str(),      // Button text
		WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,  // Styles  |BS_OWNERDRAW
		pos.x,         // x position
		pos.y,         // y position
		showW_,        // Button width
		showH_,        // Button height
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
	setHoriScrollInfo(horiMax, horiPage);
}

bool sdf::ScrollView::ControlProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_MOUSEWHEEL: {
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
	case WM_HSCROLL: {
		SCROLLINFO si;
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;
		GetScrollInfo(hDlg, SB_HORZ, &si);
		//当前滑块的位置
		//COUT("WM_HSCROLL:" << si.nPos);
		switch (wParam & 0xffff) {
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
		horiPos = si.nPos;
		SetScrollInfo(hDlg, SB_HORZ, &si, false);
		onDraw();
		break;
	}
	case WM_VSCROLL: {

		SCROLLINFO si;
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;
		GetScrollInfo(hDlg, SB_VERT, &si);
		//当前滑块的位置
		//COUT("WM_VSCROLL:" << si.nPos);
		switch (wParam & 0xffff) {
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

	return true;
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

////////////////////////////////ImageView//////////////////////////////////////////

void sdf::ImageView::onDraw() {
	if (!parent_)
		return;

	if (isPress) {
	}
	else if (isDisable) {
	}
	else if (isHover) {
	}
	else {
		if (msgDraw && !parent_->isTop) {
			//作为子成员无需单独绘制
			return;
		}
		updateDrawXY();

		DrawBuffer* draw = getDraw();

		if (Control::drawStyle(draw, style, true) && draw) {

			int w = GetWidth();
			int h = GetHeight();

			Gdiplus::Rect dest(drawX_, drawY_, w, h);
			if (showI >= 0 && showI < imageList_.size()) {
				draw->graph_->DrawImage(imageList_[showI]->getImg(), dest);
			}
			//COUT(tt_("image ondraw"));

			drawMember(gdi_, draw);
		}


	}
}

void sdf::ImageView::Init() {


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
	if (imageList_.size() > 1) {
		timerId = (uint32_t)(std::rand() % (INT_MAX - 1)) + 1;
		setTimer(timerId, interval);
	}
}

bool sdf::ImageView::ControlProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_TIMER: {
		showI += 1;
		if (showI >= imageList_.size())
			showI = 0;
		onDraw();
		break;
	}
	}
	return true;
}

////////////////////////////////Button//////////////////////////////////////////

void sdf::Button::Init() {
	if (style.backColor == 0 && !style.backImage && style.borderColor == 0) {
		setColorDark(Color::blueColor_);
	}

	onMeasure();

	DWORD sty = WS_TABSTOP | BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD;

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
	case WM_CONTEXTMENU: {
		COUT(tt_("button right click"));
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

void sdf::DrawBuffer::newBmp(int32_t w, int32_t h) {
	auto oldBmp = std::move(buttonBmp_);
	buttonBmpBuf_ = buttonBmp_.CreateDib(max(w, oldBmp.GetWidth()), max(h, oldBmp.GetHeight()));
	buttonBmp_.SetTextBackColor();
	buttonBmp_.setFont(Control::GlobalFont());

	if (oldBmp.GetDc())
		oldBmp.DrawTo(buttonBmp_);

	Gdip::Init();

	if (graph_)
		delete graph_;
	graph_ = Gdiplus::Graphics::FromHDC(buttonBmp_.GetDc());
	graph_->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
	//双线性插值
	graph_->SetInterpolationMode(Gdiplus::InterpolationModeBilinear);
}


bool sdf::Control::drawStyle(DrawBuffer* draw, ControlStyle& style, bool parentBack) {
	if (!draw)
		return false;


	lastDrawStyle = &style;
	int32_t drawX = getDrawX();
	int32_t drawY = getDrawY();

	RECT rect;
	rect.left = drawX;
	rect.top = drawY;
	rect.right = drawX + GetWidth();
	rect.bottom = drawY + GetHeight();

	int32_t w = GetWidth();
	int32_t h = GetHeight();

	if (drawX + w < parent_->drawX_ || drawX > parent_->drawX_ + parent_->GetWidth() ||
		drawY + h < parent_->drawY_ || drawY > parent_->drawY_ + parent_->GetHeight()

		) {
		//COUT(tt_("skip draw :") << cont->text);
		return false;
	}

	if (!draw->buttonBmpBuf_ || draw->buttonBmp_.GetWidth() < drawX + w || draw->buttonBmp_.GetHeight() < drawY + h) {
		draw->newBmp(drawX + w, drawY + h);
	}
	int32_t* buf = (int32_t*)draw->buttonBmpBuf_;
	if (buf == nullptr)
		return false;

	int bufW = draw->buttonBmp_.GetWidth();


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

	if (isFocused) {
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
	else if (!style.backImage && parentBack) {
		uint32_t color = 0;
		Control* par = parent_;
		while (par != nullptr && color == 0) {
			if (par->lastDrawStyle)
				color = par->lastDrawStyle->backColor;
			par = par->parent_;
		}
		for (int i = shadowLen + drawY; i < rect.bottom - style.borderBottom; i++) {
			for (int y = shadowLen + drawX; y < rect.right - style.borderRight; y++) {
				if (i * bufW + y >= 0)
					buf[i * bufW + y] = color;
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
			Gdiplus::Rect dest(sX, sY, nW, nH);


			draw->graph_->DrawImage(style.backImage->getImg(), dest);

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
			//graph_->ScaleTransform(nW/ style.backImage->GetWidth(), nH/ style.backImage->GetHeight());

			Gdiplus::Rect dest(drawX, drawY, w, h);
			draw->graph_->DrawImage(style.backImage->getImg(), dest, sX / 2, sY / 2, style.backImage->GetWidth() - sX,
				style.backImage->GetHeight() - sY, Gdiplus::UnitPixel);
		}
		else {
			Gdiplus::Rect dest(sX, sY, nW, nH);
			draw->graph_->DrawImage(style.backImage->getImg(), dest);
		}


	}

	if (text.length() > 0) {
		draw->buttonBmp_.SetTextColor(style.color);
		onDrawText(rect, draw);
	}

	return true;
}

void sdf::Control::removeAllMember() {
	for (auto& c : memberList_) {
		c->_removeFromParent(false);
	}
	memberList_.clear();
}

void sdf::Control::removeFromParent() {
	_removeFromParent(true);

}

void sdf::Control::_removeFromParent(bool remove) {
	auto par = parent_;
	if (par) {
		if (handle_) {
			::SetParent(handle_, 0);
			setHW(0, 0);
		}
		//hide();
		parent_ = 0;
		if (remove) {
			for (auto it = par->memberList_.begin(); it != par->memberList_.end(); it++) {
				if (this == it->get()) {
					par->memberList_.erase(it);
					break;
				}
			}
		}

	}
}

void sdf::Control::addMember(const std::shared_ptr<Control>& con) {
	DF_ASSERT(con->parent_ == nullptr);
	con->parent_ = this;
	memberList_.push_back(con);

	if (handle_) {
		if (!con->handle_) {
			con->doCreate();

			con->Init();
		}
		else {
			::SetParent(con->handle_, handle_);
		}

	}
}


void sdf::Button::onDraw() {
	if (oldStyle) {
		return;
	}
	if (!parent_)
		return;

	updateDrawXY();

	//if (parent_->needDraw && !parent_->isTop) {
	//	//作为子成员无需单独绘制
	//	return;
	//}

	DrawBuffer* draw = getDraw();
	bool drawSub = true;

	//df::TickClock([&] {
	if (isPress) {
		drawSub = drawStyle(draw, stylePress, true);
	}
	else if (isDisable) {
		drawSub = drawStyle(draw, styleDisable, true);
	}
	else if (isHover) {
		drawSub = drawStyle(draw, styleHover, true);
	}
	else if (isCheck) {
		drawSub = drawStyle(draw, styleCheck, true);
	}
	else {
		drawSub = drawStyle(draw, style, true);
	}
	//}, 10);


	if (drawSub) {

		drawMember(buttonGdi_, draw);
	}

	//if (drawSub) {
	//	COUT(tt_("draw button"));
	//}

}


////////////////////////////////////TextBox//////////////////////////////////////

void sdf::TextBox::Init() {
	onMeasure();

	DWORD sty = WS_TABSTOP | WS_CHILD | WS_VISIBLE |
		ES_LEFT | WS_BORDER;

	if (mutiLine) {
		sty |= ES_MULTILINE | WS_VSCROLL | ES_AUTOVSCROLL;
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


bool sdf::TextBox::ControlProc(HWND, UINT msg, WPARAM wParam, LPARAM) {

	if (msg == WM_PAINT) {

		updateDrawXY();
		//return false;
	}
	else if (msg == WM_COMMAND) {
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

	setFont(Window::GlobalFont());

	prevMsgProc_ = (WNDPROC)SetWindowLongPtr(handle_, GWLP_WNDPROC, (LONG_PTR)Control::ButtonProc);
	initAllSub();
}

bool sdf::ListBox::ControlProc(HWND, UINT msg, WPARAM wParam, LPARAM) {
	if (msg == WM_PAINT) {
		updateDrawXY();
		//return false;
	}
	else if (msg == WM_COMMAND) {
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
	if (msg == WM_PAINT) {
		updateDrawXY();
		//return false;
	}

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





void sdf::CheckBox::onDrawText(RECT& rect, DrawBuffer* draw) {

	if (!draw)
		return;

	//COUT(tt_("draw checkbox"));
	int32_t drawX = getDrawX() + pos.paddingLeft;
	int32_t drawY = getDrawY() + pos.paddingTop;

	int32_t size = (int32_t)(1 * Window::getScale());
	int32_t w = pos.h - pos.paddingTop - pos.paddingBottom;
	int bufW = draw->buttonBmp_.GetWidth();
	uint32_t* buf = (uint32_t*)draw->buttonBmpBuf_;

	uint32_t borderColor = style.borderColor;
	if (isCheck || isHover || isPress || isFocused) {
		borderColor = styleHover.borderColor;
	}

	if (isDisable) {
		if (dot) {
			Gdiplus::Pen bluePen(Gdiplus::Color(styleDisable.borderColor), (float)size * 2);
			// Create a Rect object that bounds the ellipse.
			Gdiplus::RectF ellipseRect((float)drawX, (float)drawY, (float)w, (float)w);
			draw->graph_->DrawEllipse(&bluePen, ellipseRect);
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
			draw->graph_->DrawEllipse(&bluePen, ellipseRect);

			float dotSize = (float)w / 2;
			Gdiplus::SolidBrush brush(Gdiplus::Color((Gdiplus::ARGB)borderColor));
			Gdiplus::RectF ellipseRect2(drawX + (w - dotSize) / 2, drawY + (w - dotSize) / 2, dotSize, dotSize);
			draw->graph_->FillEllipse(&brush, ellipseRect2);
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
			draw->graph_->DrawLine(&pen, drawX + w * 0.20f, drawY + w * 0.42f, drawX + w * 0.40f, drawY + w * 0.67f);
			draw->graph_->DrawLine(&pen, drawX + w * 0.38f, drawY + w * 0.66f, drawX + w * 0.8f, drawY + w * 0.28f);
		}
	}
	else {
		if (dot) {

			Gdiplus::Pen bluePen(Gdiplus::Color(borderColor), (float)size * 2);
			// Create a Rect object that bounds the ellipse.
			Gdiplus::RectF ellipseRect((float)drawX, (float)drawY, (float)w, (float)w);
			draw->graph_->DrawEllipse(&bluePen, ellipseRect);
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
	draw->buttonBmp_.Txt(rect, text);
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

	releaseImg();
	imgp_ = new Gdiplus::Image(wName);

	if (imgp_ == nullptr || imgp_->GetLastStatus() != Gdiplus::Ok) {
		width_ = 1;
		height_ = 1;
		DF_ERR(name << tcc_(" Gdiplus Load Image failed!"));
		return false;
	}
	auto alpha = imgp_->GetPixelFormat() & PixelFormatAlpha;
	hasAlpha = !!alpha;
	width_ = imgp_->GetWidth();
	height_ = imgp_->GetHeight();

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
	releaseImg();
	imgp_ = Gdiplus::Image::FromStream(pstm);

	ON_SCOPE_EXIT({
					  ::GlobalUnlock(m_hMem);
					  pstm->Release();
					  ::GlobalFree(m_hMem);
		});

	if (imgp_ == nullptr || imgp_->GetLastStatus() != Gdiplus::Ok) {
		width_ = 1;
		height_ = 1;
		DF_ERR(id << tcc_(" Gdiplus Load Image failed! type:") << resType);
		return false;
	}
	width_ = imgp_->GetWidth();
	height_ = imgp_->GetHeight();


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


//////////////////////////

//SetWindowLongPtr(handle_, GWL_EXSTYLE, GetWindowLongPtr(handle_, GWL_EXSTYLE) | WS_EX_LAYERED);
//// 设置透明度 0 - completely transparent   255 - opaque
//::SetLayeredWindowAttributes(handle_, 0, 100, LWA_ALPHA);

void sdf::FormMenu::onCreate() {
	//v.pos.centerInParent();
	v.noBorder = true;
	ui_view{
				v.style.backColor = Color::blue;
				v.pos.padding(1);
				v.pos.vector = true;

				for (auto& item : itemList_) {
					ui_button {
								  v.pos.centerY();
								  sdf::Button::setMenuStyle(&v);

								  ui_onclick {
									  close();
									  if (item.onClick)
										  item.onClick();
								  };
								  if (item.img) {
									  ui_image {
												   v.pos.w = 25;
												   v.add(item.img);
											   };
								  }
								  ui_view {
											  v.pos.marginLeft = 5;
											  v.pos.marginRight = 5;
											  v.text = item.text;

										  };


							  };
				}

	};
}