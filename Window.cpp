#include "StdAfx.h"
#include "Control.h"
#include "Font.h"
#include "../df/df_impl.hpp"
#include "FormOk.h"
#include "ScrollView.h"
#include "WebBrowser.hpp"



std::unordered_map<uint32_t, sdf::TimerItem> sdf::Timer::timerMap;

moodycamel::ConcurrentQueue<std::function<void()>> taskQueue_;

std::unordered_map<int32_t, std::shared_ptr<sdf::Bitmap>>* sdf::Bitmap::bmpBuffer = new std::unordered_map<int32_t, std::shared_ptr<sdf::Bitmap>>();

std::unordered_map<FontType, sdf::Font> sdf::Font::cache_;
HWND sdf::Control::currentHandle_ = NULL;
sdf::Window* sdf::Control::parentWindow_ = NULL;

HINSTANCE sdf::Control::progInstance_ = nullptr;

sdf::Window* sdf::Control::currentWindow_ = nullptr;

std::vector<sdf::Control*> sdf::Control::controlOpenList_;

std::array<uint32_t, 256> sdf::Control::keyboardState = { 0 };

//sdf::Control* sdf::Control::mouseHandle_ = 0;

int  sdf::Window::mouseY_ = 0;

int  sdf::Window::mouseX_ = 0;

ULONG_PTR gdiplusToken_ = 0;

Gdiplus::GdiplusStartupInput gdiplusStartupInput_ = 0;

float sdf::Control::scale_ = 1;


void sdf::Gdip::Init()
{
	if (gdiplusToken_ == 0) {
		Gdiplus::GdiplusStartup(&gdiplusToken_, &gdiplusStartupInput_, NULL);
		atexit(Shutdown);
	}
}

void sdf::Gdip::Shutdown()
{
	Gdiplus::GdiplusShutdown(gdiplusToken_);
}

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

sdf::Font::Font(const FontType& type) {
	rawSize = type.size;
	auto size = (long)((float)type.size * Window::getScale());
	memset(&logFont_, 0, sizeof(LOGFONT));
	auto fonts = FontType::getFontName();
	auto fontI = type.font >= fonts.size() ? 0 : type.font;
	auto name = fonts[fontI];
	std::memcpy(logFont_.lfFaceName, name.char_, name.length_ * sizeof(TCHAR));
	logFont_.lfHeight = size;
	if (type.bold) {
		logFont_.lfWeight = FW_SEMIBOLD;
	}
	logFont_.lfItalic = type.italic;
	logFont_.lfUnderline = type.underLine;
	logFont_.lfStrikeOut = type.strikeOut;
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
			if (pos.vertical) {
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
	if (pos.maxW >= 0 && pos.w > pos.maxW) {
		pos.w = pos.maxW;
	}
}

void sdf::Control::measureWrapY(int32_t minH) {
	int32_t subH = 0;

	for (auto& sub : memberList_) {
		//查找所有成员总高,或最大高
		if (sub->pos.h > 0) {
			int32_t sum = sub->pos.h + sub->pos.marginTop + sub->pos.marginBottom;
			if (pos.vertical) {
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

	if (pos.maxH >= 0 && pos.h > pos.maxH) {
		pos.h = pos.maxH;
	}
}

void sdf::Control::drawParentBack(DrawBuffer* draw) {
	int32_t* buf = (int32_t*)draw->buttonBmpBuf_;
	int bufW = draw->buttonBmp_.GetWidth();

	uint32_t color = 0;
	Control* par = parent_;
	while (par != nullptr && color == 0) {
		if (par->lastDrawStyle)
			color = par->lastDrawStyle->backColor;
		par = par->parent_;
	}
	drawRect((uint32_t*)buf, bufW, drawX_, drawY_, GetWidth(), GetHeight(), color);
}

void sdf::Control::bindUpdate(bool draw) {
	if (onBind_) {
		onBind_();
		for (auto& sub : memberList_) {
			sub->bindUpdate(false);
		}

		if (draw)
			onDraw();
	}
	else {
		for (auto& sub : memberList_) {
			sub->bindUpdate(draw);
		}
	}
}

void sdf::Control::onDrawText(RECT& rect, ControlStyle& style, DrawBuffer* draw) {
	if (draw) {

		if (pos.textMutiline) {
			rect.left += pos.paddingLeft;
			rect.top += pos.paddingTop;
			rect.right -= pos.paddingRight;
			rect.bottom -= pos.paddingBottom;
		}
		else {
			if (pos.textAlignX != AlignType::center) {
				rect.left += pos.paddingLeft;
				rect.right -= pos.paddingRight;
			}
			if (pos.textAlignY != AlignType::center) {
				rect.top += pos.paddingTop;
				rect.bottom -= pos.paddingBottom;
			}
		}



		/*if (pos.textAlignY != AlignType::center || pos.textMutiline) {
			rect.right -= pos.paddingRight;
			rect.bottom -= pos.paddingBottom;
		}*/


		if (pos.textMutiline)
			draw->buttonBmp_.TxtMutiLine(rect, text);
		else
			draw->buttonBmp_.Txt(rect, text, pos.textAlignX, pos.textAlignY);
	}
}

void sdf::Control::doLeave(bool doEvent) {
	/*auto ho = hoverGlobal_;
	while (ho) {
		ho->hoverView = nullptr;
		ho->isHover = false;
		ho->onLeave();
		ho = ho->parent_;
	}
	hoverGlobal_ = nullptr;*/
	auto old = hoverView;
	auto ho = hoverView;
	hoverView = nullptr;

	while (ho) {
		ho->isHover = false;
		if (doEvent)
			ho->onLeave();
		auto oldHo = ho;
		ho = ho->hoverView;
		oldHo->hoverView = nullptr;
	}
}

void sdf::Control::doParentMove() {
	auto par = parent_;
	auto cur = this;
	while (par) {
		if (cur != par->hoverView) {
			if (par->hoverView) {
				par->doLeave();
			}
			par->hoverView = cur;
		}
		cur = par;
		par = par->parent_;

	}
	if (isHover == false) {
		isHover = true;
		onHover();
	}
}

void sdf::Control::onMouseMove(int32_t x, int32_t y) {
	int32_t hori = getHoriPos();
	int32_t vert = getVertPos();
	int32_t vX = x + hori;
	int32_t vy = y + vert;
	//
		//倒序

	for (intptr_t memI = memberList_.size() - 1; memI >= 0; memI--) {
		auto& v = memberList_[memI];

		if ((*v).hited(vX, vy)) {
			hoverViewIndex = memI;
			if (hoverView != v.get()) {
				if (hoverView) {
					doLeave();
				}
				hoverView = v.get();
				hoverView->isHover = true;
				hoverView->onHover();
			}
			(*v).onMouseMove(x - (*v).pos.x + hori, y - (*v).pos.y + vert);
			return;
		}
	}
	hoverViewIndex = -1;
	doLeave();
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


	int32_t flexXsum = 0, flexYsum = 0;
	int32_t flexWsum = 0, flexHsum = 0;
	bool isRight = false;
	//计算所有邻居flex
	if ((pos.flexX > 0 && !parent_->pos.vertical) || (pos.flexY > 0 && parent_->pos.vertical)) {
		int32_t oldMx = parent_->measureX_, oldMy = parent_->measureY_;
		for (auto& sub : parent_->memberList_) {
			if (sub->pos.absolute)
				continue;

			if (isRight && sub->pos.w < 0) {//提前计算右侧邻节点
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
			if (parent_->pos.vertical)
				pos.w = parent_->pos.w - parent_->pos.paddingLeft - parent_->pos.paddingRight - pos.marginLeft -
				pos.marginRight;
			else
				pos.w = (int32_t)std::round(
					(parent_->pos.w - parent_->pos.paddingLeft - parent_->pos.paddingRight - flexWsum) * pos.flexX /
					(float)flexXsum) - pos.marginLeft - pos.marginRight;
		}
	}


	if (pos.flexY > 0) {

		if (parent_->pos.h > 0) {
			if (parent_->pos.vertical)
				pos.h = (int32_t)std::round(
					(parent_->pos.h - flexHsum - parent_->pos.paddingTop - parent_->pos.paddingBottom) * pos.flexY /
					(float)flexYsum) - pos.marginTop - pos.marginBottom;
			else
				pos.h = parent_->pos.h - pos.marginTop - pos.marginBottom - parent_->pos.paddingTop -
				parent_->pos.paddingBottom;
		}
	}

	if (pos.wrapX || pos.wrapY) {
		//wrap,提前计算所有子成员
		for (auto& sub : memberList_) {
			sub->onMeasure();
		}
	}
	measureX_ = 0;
	measureY_ = 0;

	if (pos.wrapX) {
		measureWrapX(w);
	}

	if (pos.wrapY) {
		reGetContentWH(w, h);
		measureWrapY(h);
	}

	if (parent_->pos.w > 0 && pos.w > 0) {
		if (pos.alignInParentX == AlignType::center || parent_->pos.alignX == AlignType::center) {
			pos.x = (parent_->pos.w - pos.w) / 2;
		}
		else if (pos.alignInParentX == AlignType::end || parent_->pos.alignX == AlignType::end) {
			pos.x = parent_->pos.w - pos.w - parent_->pos.paddingRight - pos.marginRight;
		}
	}


	if (parent_->pos.h > 0 && pos.h > 0) {
		if (pos.alignInParentY == AlignType::center || parent_->pos.alignY == AlignType::center) {
			pos.y = (parent_->pos.h - pos.h) / 2;
		}
		else if (pos.alignInParentY == AlignType::end || parent_->pos.alignY == AlignType::end) {
			pos.y = parent_->pos.h - pos.h - parent_->pos.paddingBottom - pos.marginBottom;
		}
	}

	//int32_t hp = pos.x - parent_->getHoriPos();
	//int32_t vp = pos.y - parent_->getVertPos();
	updateDrawXY();
	/*if (drawX_ != showX_ || drawY_ != showY_ || pos.w != showW_ || pos.h != showH_)
		setPosAndHW(drawX_, drawY_, pos.w, pos.h);*/

	if (pos.absolute) {
	}
	else {

		if (parent_->pos.vertical) {
			parent_->measureY_ += pos.h + pos.marginBottom + pos.marginTop;
		}
		else {
			parent_->measureX_ += pos.w + pos.marginRight + pos.marginLeft;
		}
	}

	pos.controlW = pos.w;
	pos.controlH = pos.h;

}

void sdf::Control::UpdateWinRect() {
	RECT rect;

	GetClientRect(handle_, &rect);

	if (pos.h >= 0)
		pos.h = (int32_t)(rect.bottom);

	if (pos.w >= 0)
		pos.w = (int32_t)(rect.right);


	::GetWindowRect(handle_, &rect);
	pos.x = (int32_t)rect.left;
	pos.y = (int32_t)rect.top;

	if (parentWindow_) {
		pos.x -= parentWindow_->pos.x + parentWindow_->borderSize_;
		pos.y -= parentWindow_->pos.y + parentWindow_->titleHeight_;
	}
}

inline BOOL sdf::Control::setPos(int32_t x, int32_t y, bool draw) {
	showX_ = x;
	showY_ = y;
	//SetWindowPos很慢
	if (handle_) {
		uint32_t sty = SWP_NOSIZE | SWP_NOZORDER | SWP_DEFERERASE | SWP_NOOWNERZORDER | SWP_NOACTIVATE |
			SWP_NOCOPYBITS;
		if (!draw) {
			sty |= SWP_NOREDRAW;
		}
		return ::SetWindowPos(handle_, 0, x, y, 0, 0, sty);
	}
	return false;
}

BOOL sdf::Control::setPosAndHW(int32_t x, int32_t y, int32_t w, int32_t h) {
	showX_ = x;
	showY_ = y;
	showW_ = w;
	showH_ = h;
	if (handle_)
		return ::SetWindowPos(handle_, 0, x, y, w, h, SWP_NOZORDER | SWP_NOCOPYBITS | SWP_NOREDRAW);
	return false;
}

sdf::Timer sdf::Control::setTimer(uint32_t time, std::function<void()>&& func) {
	return Timer::set(this, time, std::move(func));
}

void sdf::Control::Init() {
	InitUserData();
	inited = true;
}

void sdf::Control::initAllSub() {
	measureX_ = 0;
	measureY_ = 0;
	for (auto& con : memberList_) {
		con->Init();
	}
}

void sdf::Control::doCreate() {

	if (onCreate_) {
		onCreate_();
		if (onBind_)
			onBind_();
		onCreate_ = nullptr;
	}
	else {
		if (onBind_)
			onBind_();
	}
	if (pos.w < 0 && pos.flexX < 1)
		pos.wrapX = true;

	if (pos.h < 0 && pos.flexY < 1)
		pos.wrapY = true;
	for (auto& con : memberList_) {
		con->doCreate();
	}
	scalePos(pos);
	scaleStyle(style);
	scaleStyle(stylePress);
	scaleStyle(styleHover);
	scaleStyle(styleDisable);
}


////////////////////////////////Brush//////////////////////////////////////////

void sdf::Brush::SetFromBitmap(Bitmap& bmp) {
	ReleaseBrush();
	brush_ = CreatePatternBrush(bmp.GetBitmap());
}

////////////////////////////////Window//////////////////////////////////////////



void sdf::Control::adjustRecur(sdf::Control* cont) {
	cont->measureX_ = 0;
	cont->measureY_ = 0;
	for (auto& control : cont->memberList_) {
		control->onMeasure();
		if (!control->pos.wrapX && !control->pos.wrapY)
			adjustRecur(control.get());
	}
}

void sdf::Control::updateDrawXY() {
	int32_t hp = parent_->getHoriPos();
	int32_t vp = parent_->getVertPos();
	drawX_ = parent_->drawX_ - hp + pos.x;
	drawY_ = parent_->drawY_ - vp + pos.y;
}

void sdf::Control::updateHandleXy(Gdi& gdi, DrawBuffer* draw) {
	int32_t x = getShowX();
	int32_t y = getShowY();

	if (x != showX_ || y != showY_) {
		setPos(x, y, true);
	}

	if (!draw)
		return;

	if (!draw->buttonBmpBuf_)
		return;

	gdi.DrawTo(draw->buttonBmp_, x, y, getWidthNoBorder(), getHeighNoBorder());

	if (parent_->needDraw) {
		draw->draw(drawX_, drawY_, pos.w, pos.h);
	}
}

void sdf::Control::drawMember(DrawBuffer* draw) {
	needDraw = false;
	DF_SCOPE_GUARD
	{
		needDraw = true;
	};

	for (auto& sub : memberList_) {

		if (sub->showOverflow()) {
			continue;
		}
		sub->onDraw();
	}


	layerToDraw(draw);

	if (parent_ && parent_->needDraw && draw) {
		drawParentLayer();

		draw->draw(drawX_, drawY_, pos.w, pos.h);
	}
	//draw->gdi_->DrawFrom(draw->buttonBmp_, drawX_, drawY_, pos.w, pos.h, drawX_, drawY_);

}


void sdf::Control::drawMember(Gdi& gdi, DrawBuffer* draw) {
	needDraw = false;
	DF_SCOPE_GUARD
	{
		needDraw = true;
	};

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
	if (parent_ && parent_->needDraw && draw) {
		if (gdi.GetDc())
			gdi.DrawFrom(draw->buttonBmp_, 0, 0, pos.w, pos.h, drawX_, drawY_);
	}

}

LRESULT  __stdcall sdf::Control::ButtonProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {

	try {
		switch (message) {

		case WM_COMMAND: {
			auto l = LOWORD(wParam);
			//auto h = HIWORD(wParam);
			if (lParam) {
				Control* controlP = GetUserData((HWND)lParam);
				if (controlP) {
					LRESULT ret;
					controlP->ControlProc(hDlg, message, wParam, lParam, ret);
					break;
				}
			}
			break;
		}
		case WM_LBUTTONDBLCLK:
			PostMessage(hDlg, WM_LBUTTONDOWN, wParam, lParam);
			break;

		case WM_ERASEBKGND: {
			//COUT(tt_("button WM_ERASEBKGND"));
			//取消系统背景
			return 1;
		}
		}

		Control* cont = GetUserData(hDlg);
		if (cont) {
			LRESULT ret;
			auto contRes = cont->ControlProc(hDlg, message, wParam, lParam, ret);
			if (!contRes)
				return ret;
		}

		auto res = controlComProc(hDlg, message, wParam, lParam);
		if (res) {
			return res;
		}

		if (cont && cont->prevMsgProc_)
			return CallWindowProc(cont->prevMsgProc_, hDlg, message, wParam, lParam);

	}
	catch (df::Exception& ex) {
		df::Exception::ExceptionLog(ex);
		MessageBox(::GetActiveWindow(), ex.message_.c_str(), df::lang().exception.c_str(), MB_OK | MB_ICONERROR);
		DF_BREAK_POINT;
	}
	catch (std::exception& ex) {
		df::Exception::ExceptionLog(ex);
		MessageBox(::GetActiveWindow(), (String() << ex.what()).c_str(), df::lang().exception.c_str(),
			MB_OK | MB_ICONERROR);
		DF_BREAK_POINT;
	}

	return DefWindowProc(hDlg, message, wParam, lParam);
}

//父子公用消息处理
intptr_t sdf::Control::controlComProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	//COUT((int*)message);
	sdf::Control* cont = sdf::Window::GetUserData(hDlg);
	switch (message) {
	case Window::taskMessage_: {
		std::function<void()> ff;
		while (taskQueue_.try_dequeue(ff)) {
			try {
				ff();
			} DF_CATCH_ALL;
		}
		break;
	}
	case WM_SYSKEYDOWN:
	case WM_KEYDOWN: {
		if (wParam > 0 && wParam < Control::keyboardState.size()) {
			uint32_t extended = (uint32_t)lParam & (1 << 24);
			Control::keyboardState[wParam] = 1 | extended;

			//COUT(tt_("WM_KEYDOWN ") << wParam << tt_(" extended ") << extended);

		}

		break;
	}
	case WM_SYSKEYUP:
	case WM_KEYUP: {
		if (wParam > 0 && wParam < Control::keyboardState.size()) {
			Control::keyboardState[wParam] = 0;
		}
		//COUT(tt_("WM_KEYUP ") << wParam);
		break;
	}
	case WM_KILLFOCUS: {
		if (cont)
			cont->onKillFocus();
		break;
	}
	case WM_SETFOCUS: {
		if (cont)
			cont->onFocus();
		break;
	}
	case WM_LBUTTONDOWN: {
		if (cont) {
			cont->doEvent(&Control::onCaptureLeftDown, &Control::onLeftDown);
		}
		break;
	}
	case WM_LBUTTONUP: {
		if (cont) {
			cont->doEvent(&Control::onCaptureLeftUp, &Control::onLeftUp);
		}
		break;
	}
	case WM_RBUTTONDOWN: {
		if (cont) {
			cont->doEvent(&Control::onCaptureRightDown, &Control::onRightDown);
		}
		break;
	}
	case WM_RBUTTONUP: {
		if (cont) {
			cont->doEvent(&Control::onCaptureRightUp, &Control::onRightUp);
		}
		break;
	}
	case WM_CTLCOLOREDIT: {
		//文本样式
		HDC hdc = (HDC)wParam;
		HWND hwnd = (HWND)lParam;
		if (cont && cont->style.color) {
			SetTextColor(hdc, Color::toRGB(Color::red));
			//SetBkMode(hdc, TRANSPARENT);
			return (intptr_t)Brush::GetWhiteBrush();
		}
		return 0;
	}
	case WM_TIMER: {
		uint32_t id = (uint32_t)wParam;
		Timer::onTimer(id);
		break;
	}

	case WM_CTLCOLORBTN: {
		//绘制控件背景,(自绘控件仍触发)
		//Control* controlP = GetUserData((HWND)lParam);
		//::SetBkMode((HDC)wParam, TRANSPARENT);
		//返回空背景
		return (intptr_t)Brush::GetNullBrush();
	}
	case WM_SETCURSOR: {
		if (cont && cont->hasCursor)
			return cont->hasCursor;
		//COUT(tt_("WM_SETCURSOR"));
		return false;
	}
					 //移出窗口
	case WM_NCMOUSEMOVE: {

		//case WM_NCMOUSELEAVE: {
		if (cont) {
			cont->doLeave();
		}
		break;
	}

	case WM_MOUSEMOVE: {

		int32_t x = (int16_t)LOWORD(lParam);
		int32_t y = (int16_t)HIWORD(lParam);

		sdf::Window::mouseX_ = x;
		sdf::Window::mouseY_ = y;

		if (cont == nullptr)
			break;

		cont->onMouseMove(x, y);
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

		currentHandle_ = hDlg;
		currentWindow_ = winP;

		switch (message) {
		case WM_CREATE: {

			break;
		}
		case WM_CLOSE: {
			winP->closeRelease((int)wParam);
			return TRUE;
		}

		case WM_COMMAND: {
			auto l = LOWORD(wParam);
			auto code = HIWORD(wParam);

			if (code == BN_CLICKED) {
			}
			else if (code == BN_DBLCLK) {

			}
			else if (code == IDCANCEL) {
			}
			//auto h = HIWORD(wParam);

			/*	if (l == IDCANCEL) {
					winP->closeRelease();
					break;
				}*/

			if (lParam) {
				Control* controlP = GetUserData((HWND)lParam);
				if (controlP) {
					LRESULT ret;
					controlP->ControlProc(hDlg, message, wParam, lParam, ret);
					break;
				}
			}

			break;
		}

					   /*	case WM_NOTIFY: {
							   LPNMHDR lpn = (LPNMHDR)lParam;
							   Control* controlP = GetUserData(lpn->hwndFrom);
							   if (controlP)
								   controlP->ControlNotify(lpn);
							   break;
						   }*/

		case WM_ACTIVATE: {

			if (wParam == WA_INACTIVE) {
				//COUT(tt_("WA_INACTIVE"));
				winP->onInActive();
				std::memset(keyboardState.data(), 0, keyboardState.size());
			}
			else {
				//COUT(tt_("WM_ACTIVATE"));
				winP->onActive();
			}
			break;
		}
		case WM_EXITSIZEMOVE: {
			//COUT(tt_("WM_EXITSIZEMOVE"));
			winP->onExitResize();
			break;
		}
		case WM_SIZE: {
			int32_t w = LOWORD(lParam);
			int32_t h = HIWORD(lParam);
			{
				//COUT(tt_("WM_SIZE"));
				winP->pos.w = w;
				winP->pos.h = h;
				winP->onResize();
				winP->AdjustLayout();
				::SendMessage(hDlg, WM_PAINT, 0, 0);
			}
			break;
		}
		case WM_MOVE: {
			winP->pos.x = LOWORD(lParam);
			winP->pos.y = HIWORD(lParam);
			winP->onMove(winP->pos.x, winP->pos.y);
			break;
		}

		case WM_PAINT: {
			//COUT(tt_("WM_PAINT"));
			winP->onPaint();

			//if (df::Time::getNowMsec() - winP->drawTime > 10) {
			winP->onDraw();
			if (winP->drawBuff_->buttonBmpBuf_) {
				PAINTSTRUCT ps = { 0 };
				HDC dd = BeginPaint(winP->handle_, &ps);
				//COUT(tt_("dc:") << (int*)winP->gdi_.GetDc() << tt_(" - ") << (int*)dd);
				winP->drawBuff_->buttonBmp_.DrawTo(winP->gdi_.GetDc());
				EndPaint(winP->handle_, &ps);
			}
			winP->drawTime = df::Time::getNowMsec();
			//}


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
		case WM_ENDSESSION: {
			winP->onEndSession();
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

	}
	catch (df::Exception& ex) {
		df::Exception::ExceptionLog(ex);
		MessageBox(::GetActiveWindow(), ex.message_.c_str(), df::lang().exception.c_str(), MB_OK | MB_ICONERROR);
		DF_BREAK_POINT;
	}
	catch (std::exception& ex) {
		df::Exception::ExceptionLog(ex);
		MessageBox(::GetActiveWindow(), (String() << ex.what()).c_str(), df::lang().exception.c_str(),
			MB_OK | MB_ICONERROR);
		DF_BREAK_POINT;
	}
	return DefWindowProc(hDlg, message, wParam, lParam);
}

void sdf::Window::setPosXY() {
	if (pos.alignInParentX == AlignType::center) {
		if (parent_) {
			pos.x = parent_->pos.x + (parent_->pos.w - pos.w) / 2;
		}
		else {
			pos.x = (Window::GetScreenWidth() - pos.w) / 2;
		}

	}
	if (pos.alignInParentY == AlignType::center) {
		if (parent_) {
			pos.y = parent_->pos.y + (parent_->pos.h - pos.h) / 2;
		}
		else {
			pos.y = (Window::GetScreenHeight() - pos.h) / 2;
		}
	}
}

void sdf::Window::openRaw(HWND parent/*=0*/, bool show) {

	Gdi::gobalGdi().setFont(GlobalFont());

	onCreate();


	WNDCLASS wndclass = { 0 };
	//wndclass.style = CS_VREDRAW | CS_HREDRAW;
	wndclass.style = 0;
	wndclass.lpfnWndProc = (WNDPROC)WndProc;
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

	TCHAR className[18] = tt_("sdfWin");
	if (winClassName.length() > 0)
		wndclass.lpszClassName = winClassName.c_str();
	else {
		for (int i = 0; i < 14; i++) {
			className[i + 3] = std::rand() % 26 + 'a';
		}
		wndclass.lpszClassName = className;
	}

	if (!RegisterClass(&wndclass)) {
		DF_ERR(tcc_("RegisterClass failed"));
		return;
	}

	Window::scalePos(pos, false);

	setPosXY();
	DWORD sty = WS_VISIBLE;

	//WS_EX_TRANSPARENT;
	DWORD styEX = getExStyle();

	if (maxBox)
		sty |= WS_MAXIMIZEBOX;

	if (minBox)
		sty |= WS_MINIMIZEBOX;

	if (resizeAble) {
		sty |= WS_CAPTION | WS_OVERLAPPED | WS_SYSMENU | WS_THICKFRAME;
	}
	else {
		//sty |= WS_POPUP;
		//styEX |= WS_EX_DLGMODALFRAME;
	}


	if (initMaxSize)
		sty |= WS_MAXIMIZE;

	if (initMixSize)
		sty |= WS_MINIMIZE;

	if (noBorder) {
		sty = WS_POPUP | WS_VISIBLE;
	}


	handle_ = CreateWindowEx(styEX,
		wndclass.lpszClassName,
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

	if (ptrParent_) {
		ptrParent_->doLeave();
		ptrParent_->enable(false);
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


	gdi_.Init(handle_);
	//使用父窗口字体
	gdi_.setFont(Control::GlobalFont());
	//文字背景透明
	gdi_.SetTextBackColor();

	//if (pos.w > 0 && pos.h > 0) {
	//	drawBuff_->newBmp(pos.w, pos.h);
	//}

	Control::controlOpenList_.push_back(this);

	InitWinData();

	::SendMessage(handle_, WM_SETFONT, (WPARAM)Window::GlobalFont().GetFont(), TRUE);

	if (pos.w < 0 || pos.h < 0) {
		if (pos.w < 0)
			measureWrapX(0);
		if (pos.h < 0)
			measureWrapY(0);
		setPosXY();
		if (pos.x + pos.w + getBorderW() > Window::GetScreenWidth()) {
			pos.x = Window::GetScreenWidth() - pos.w - getBorderW();
		}
		if (pos.y + pos.h + getBorderH() > Window::GetScreenHeight()) {
			pos.y = Window::GetScreenHeight() - pos.h - getBorderH();
		}

		::SetWindowPos(handle_, 0, pos.x, pos.y,
			pos.w + getBorderW(), pos.h + getBorderH(),
			SWP_NOZORDER | SWP_NOCOPYBITS | SWP_NOSENDCHANGING);

		/*if (pos.w > 0 && pos.h > 0) {
			drawBuff_->newBmp(pos.w, pos.h);
		}*/
	}

	if (memberList_.size() == 1) {
		if (dynamic_cast<WebView*>(memberList_[0].get())) {
			if (memberList_[0]->pos.flexX == 1 && memberList_[0]->pos.flexY == 1) {
				drawBack = false;
			}
		}
	}
	::ShowWindow(handle_, show);
	::UpdateWindow(handle_);
	currentHandle_ = oldHandle;
	//currentWindow_=oldWindow;
	if (show) {
		::PostMessage(handle_, WM_PAINT, 0, 0);
	}

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

void sdf::Window::closeRelease(int code) {


	if (!onClose(code)) {
		return;
	}

	doLeave();
	Control::removeOpenControl(this);
	DF_SCOPE_GUARD
	{
		ptr_.reset();
	};


	if (ptrParent_) {
		ptrParent_->enable(true);
		ptrParent_->setFocus();
	}

	::DestroyWindow(handle_);
	if (isMain)
		::PostQuitMessage(0);

}

void sdf::Window::run(bool show/*=true*/) {
	isMain = true;
	open(nullptr, show);
	MessageLoop();
}


bool sdf::Window::setClip(df::CC text)
{
	if (OpenClipboard(NULL))
	{
		EmptyClipboard();
		size_t size = (text.size() + 1) * sizeof(TCHAR);
		HGLOBAL clipbuffer = GlobalAlloc(GMEM_DDESHARE, size);
		if (clipbuffer == NULL) {
			return false;
		}
		TCHAR* buffer = (TCHAR*)GlobalLock(clipbuffer);
		if (buffer == NULL) {
			return false;
		}
		memcpy(buffer, text.c_str(), size);
		GlobalUnlock(clipbuffer);
		SetClipboardData(sizeof(TCHAR) == 2 ? CF_UNICODETEXT : CF_TEXT, clipbuffer);

		CloseClipboard();
		return true;
	}
	return false;
}


float sdf::Window::getScale() {
	auto res = (float)GetDeviceCaps(Gdi::GetScreen().GetDc(), LOGPIXELSX) / (float)96.0;
	if (res < 1)
		res = 1;
	Control::scale_ = res;
	return res;
}

void sdf::Window::runOnUi(std::function<void()>&& func) {
	taskQueue_.enqueue(std::move(func));
	::PostMessage(currentHandle_, taskMessage_, 0, 0);
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

	//drawMember(gdi_, getDraw());
}

void sdf::Window::onDraw() {
	if (!drawBack) {
		return;
	}

	auto draw = getDraw();

	bool drawSub = drawStyle(draw, style, false, false);

	if (drawSub) {
		drawMember(draw);
	}
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

	::SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) new PopMsgStruct{ msg.toString(), 0, time });
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


	UpdateWinRect();
	UpdateBorderSize();
	auto oldWin = parentWindow_;
	parentWindow_ = this;

	parentWindow_ = oldWin;

	for (auto& con : memberList_) {
		con->doCreate();
	}

	initAllSub();
	//AdjustLayout();

	try {
		onInit();
	} DF_CATCH_ALL;
}


////////////////////////////////Control//////////////////////////////////////////

bool sdf::Control::showOverflow() {
	int32_t vert = parent_->getVertPos();
	int32_t hori = parent_->getHoriPos();


	int32_t w = GetWidth() + parent_->pos.paddingLeft + parent_->pos.paddingRight + pos.marginLeft + pos.marginRight;
	int32_t h = GetHeight() + parent_->pos.paddingTop + parent_->pos.paddingBottom
		+ pos.marginTop + pos.marginBottom;

	if (
		pos.x - hori + w * 2 < 0 ||
		pos.x - hori - 2 > parent_->GetWidth() + parent_->pos.paddingLeft + parent_->pos.paddingRight
		||
		pos.y - vert + h * 2 < 0 ||
		pos.y - vert - 2 > parent_->GetHeight() + parent_->pos.paddingTop + parent_->pos.paddingBottom) {
		return true;
	}

	return false;
}


sdf::Font& sdf::Control::GlobalFont() {
	static Font* f = 0;
	if (f == nullptr) {
		f = new Font();
		Gdi::GetScreen().setFont(*f);
	}
	return *f;
}

void sdf::Control::initInst(HINSTANCE inst) {
	::SetProcessDPIAware();
	progInstance_ = inst;
	Window::getScale();
}


void sdf::Control::ReleaseUserData() {
	if (handle_) {
		::SetWindowLongPtr(handle_, GWLP_USERDATA, 0);
		if (!isTop) {
			::DestroyWindow(handle_);
		}
		handle_ = 0;
	}

}


void sdf::View::onDraw() {
	if (!parent_)
		return;

	updateDrawXY();

	auto draw = getDraw();

	bool drawSub = drawStyle(draw, style, parent_->needDraw);



	if (drawSub) {
		drawMember(draw);
	}

}


void sdf::ScrollView::setVertScrollInfo(int max, int page) {
	vertMax = max;
	vertPage = page;

	if (!handle_)
		return;

	SCROLLINFO si = { 0 };
	si.cbSize = sizeof(si);
	si.nMin = 0;
	si.nMax = max;
	si.nPage = page;
	si.fMask = SIF_RANGE | SIF_PAGE;

	SetScrollInfo(handle_, SB_VERT, &si, TRUE);
}

void sdf::ScrollView::setHoriScrollInfo(int max, int page) {
	horiMax = max;
	horiPage = page;

	if (!handle_)
		return;


	SCROLLINFO si = { 0 };
	si.cbSize = sizeof(si);
	si.nMin = 0;
	si.nMax = max;
	si.nPage = page;
	si.fMask = SIF_RANGE | SIF_PAGE;


	SetScrollInfo(handle_, SB_HORZ, &si, TRUE);
}

void sdf::ScrollView::onMeasure() {

	Control::onMeasure();

	if (pos.controlW != showW_ || pos.controlH != showH_) {
		setHW(pos.controlW, pos.controlH);
	}

	auto fontSize = Control::GlobalFont().GetFontSize();

	measureX_ = 0;
	measureY_ = 0;


	contentW = pos.paddingLeft + pos.paddingRight;
	contentH = pos.paddingTop + pos.paddingBottom;
	bool flexX = false, flexY = false;
	for (auto& sub : memberList_) {
		sub->onMeasure();
		if (pos.vertical) {

			contentH += sub->pos.controlH + sub->pos.marginTop + sub->pos.marginBottom;
			if (sub->pos.controlW > contentW) {
				contentW = sub->pos.controlW;
				flexX = sub->pos.flexX > 0;
			}
		}
		else {
			contentW += sub->pos.controlW + sub->pos.marginLeft + sub->pos.marginRight;
			if (sub->pos.controlH > contentH) {
				contentH = sub->pos.controlH;
				flexY = sub->pos.flexY > 0;
			}
		}


	}
	int w = pos.w, h = pos.h;

	if (!flexX && contentW > pos.w + 1) {
		pos.h = h - getScrollWidth();
	}
	if (!flexY && contentH > pos.h + 1) {
		pos.w = w - getScrollWidth();
	}

	if (!flexX && contentW > pos.w + 1) {
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

	if (!flexY && contentH > pos.h + 1) {
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

	drawX_ = 0;
	drawY_ = 0;
	int32_t x = getDrawX();
	int32_t y = getDrawY();
	if (x != showX_ || y != showY_) {
		setPos(x, y, false);
	}

	DrawBuffer* draw = getDraw();

	bool drawSub = true;

	if (isPress) {
		drawSub = drawStyle(draw, stylePress, parent_->needDraw);
		return;
	}
	if (isDisable) {
		drawSub = drawStyle(draw, styleDisable, parent_->needDraw);
	}
	else if (isHover) {
		drawSub = drawStyle(draw, styleHover, parent_->needDraw);
	}
	else {
		drawSub = drawStyle(draw, style, parent_->needDraw);
	}



	if (drawSub) {

		if (linePos >= 0) {
			if (pos.vertical) {
				int32_t lineX = 0;
				int32_t lineY = linePos - getVertPos();
				auto drawLayer = getDrawLayer(lineX, lineY, pos.w, dragLineSize, true);
				drawRect(drawLayer_->buttonBmpBuf_, drawLayer_->buttonBmp_.GetWidth(), lineX, lineY,
					pos.w, dragLineSize, dragLineColor);
			}
			else {
				int32_t lineX = linePos - getHoriPos();
				int32_t lineY = 0;
				auto drawLayer = getDrawLayer(lineX, lineY, dragLineSize, pos.h, true);
				drawRect(drawLayer_->buttonBmpBuf_, drawLayer_->buttonBmp_.GetWidth(), lineX, lineY,
					dragLineSize, pos.h, dragLineColor);
			}

		}
		else if (drawLayer_) {
			drawLayer_->clear();
		}

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
			layerToDraw(draw);
			DrawBuffer* drawParent = parent_->getDraw();
			if (drawParent)
				drawParent->buttonBmp_.DrawFrom(draw->buttonBmp_, x, y, pos.w, pos.h, drawX_, drawY_);
			if (parent_->needDraw) {
				drawParentLayer();
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
		WS_VISIBLE | WS_CHILD | BS_OWNERDRAW | WS_CLIPCHILDREN,  // Styles  |BS_OWNERDRAW
		getDrawX(),         // x position
		getDrawY(),         // y position
		pos.controlW,        // Button width
		pos.controlH,        // Button height
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
	//文字背景透明
	gdi_.SetTextBackColor();
	if (style.font.hasFont())
		gdi_.setFont(Font::getFont(style.font));
	else
		gdi_.setFont(Window::GlobalFont());

	prevMsgProc_ = (WNDPROC)SetWindowLongPtr(handle_, GWLP_WNDPROC, (LONG_PTR)Control::ButtonProc);

	//df::TickClock([&]
	//	{
	initAllSub();
	/*	}, 1);*/
	dragLineSize = (int32_t)(dragLineSize * Control::scale_);

	setVertScrollInfo(vertMax, vertPage);
	setHoriScrollInfo(horiMax, horiPage);
}

bool sdf::ScrollView::ControlProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& ret) {
	switch (msg) {

	case WM_COMMAND: {
		auto code = HIWORD(wParam);

		if (code == BN_CLICKED) {


			//Window::PopMessage(tt_("scorll click"));
		}
		else if (code == BN_DBLCLK) {

		}
		break;
	}
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
		SetScrollInfo(hDlg, SB_HORZ, &si, true);
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
		SetScrollInfo(hDlg, SB_VERT, &si, true);
		onDraw();
		//InvalidateRect(hDlg, NULL, TRUE);
	}
				   break;
	}

	return true;
}



void sdf::ScrollView::onMouseMove(int32_t x, int32_t y) {
	//COUT(tt_("ScrollView onMouseMove"))
	mouseX = x;
	mouseY = y;
	doParentMove();
	Control::onMouseMove(x, y);
}

void sdf::ScrollView::onLeave() {
	//COUT(tt_("ScrollView onLeave"))
		//doLeave();

}


void sdf::View::Init() {
	onMeasure();


	Control::Init();

	initAllSub();
}

////////////////////////////////ImageView//////////////////////////////////////////

void sdf::ImageView::onDraw() {
	if (!parent_)
		return;

	updateDrawXY();

	DrawBuffer* draw = getDraw();

	if (Control::drawStyle(draw, style, parent_->needDraw) && draw) {

		int w = GetWidth() - pos.paddingLeft - pos.paddingRight;
		int h = GetHeight() - pos.paddingTop - pos.paddingBottom;

		Gdiplus::Rect dest(drawX_ + pos.paddingLeft, drawY_ + pos.paddingTop, w, h);
		if (img_) {
			draw->graph_->DrawImage(img_->getImg(), dest);
		}
		//COUT(tt_("image ondraw"));

		drawMember(draw);
	}
}

void sdf::ImageView::Init() {

	onMeasure();

	Control::Init();

	initAllSub();
	/* if (imageList_.size() > 1) {
		 time = setTimer(interval, [this]
		 {
			 showI += 1;
			 if (showI >= (intptr_t) imageList_.size())
				 showI = 0;
			 onDraw();
		 });
	 }*/
}



////////////////////////////////Button//////////////////////////////////////////

void sdf::LoadAnim::onDraw() {
	auto draw = getDraw();

	updateDrawXY();

	drawStyle(draw, style, parent_->needDraw);

	if (showAnim) {
		int32_t penSize = (int32_t)(GetWidth() / 15);
		if (penSize < 1)
			penSize = 1;
		int32_t drawX = drawX_ + pos.paddingLeft + penSize;
		int32_t drawY = drawY_ + pos.paddingTop + penSize;
		int32_t w = GetWidth() - penSize * 2;

		Gdiplus::Pen greenPen(Gdiplus::Color(Color::greenColor_), (float)penSize);
		draw->graph_->DrawArc(&greenPen, drawX, drawY, w - pos.paddingLeft - pos.paddingRight,
			w - pos.paddingTop - pos.paddingBottom, startAngle, arcLen);

		Gdiplus::Pen bluePen(Gdiplus::Color(Color::blue), (float)penSize);
		draw->graph_->DrawArc(&bluePen, drawX, drawY, w - pos.paddingLeft - pos.paddingRight,
			w - pos.paddingTop - pos.paddingBottom, (float)(startAngle + 180), arcLen);

	}

	if (dotSize > 0 && dotColor && showDot) {
		Gdiplus::SolidBrush brush(Gdiplus::Color((Gdiplus::ARGB)dotColor));
		Gdiplus::RectF ellipseRect2((float)drawX_ + (GetWidth() - dotSize) / 2,
			(float)drawY_ + (GetHeight() - dotSize) / 2, dotSize, dotSize);
		draw->graph_->FillEllipse(&brush, ellipseRect2);
	}
	drawMember(draw);
}

void sdf::LoadAnim::doCreate() {
	Control::doCreate();
	dotSize = dotSize * scale_;
}

void sdf::LoadAnim::Init() {

	onMeasure();

	Control::Init();

	initAllSub();

	if (showAnim)
		enableAnim(true);
}

void sdf::LoadAnim::onTimer() {

	startAngle += 3 + ((uint16_t)arcInc >> 15) * 3;

	if (startAngle > 360) {
		startAngle = 0;
	}

	arcLen += arcInc;
	if (arcLen > 180) {
		arcInc *= -1;
	}

	if (arcLen < 6) {
		arcInc *= -1;
		arcLen = 6;
	}

	onDraw();
}


////////////////////////////////Button//////////////////////////////////////////

void sdf::Button::Init() {
	if (style.backColor == 0 && styleHover.backColor == 0 && !style.backImage && style.borderColor == 0) {
		setColorDark(Color::blueColor_);
	}

	onMeasure();

	Control::Init();

	initAllSub();
}


void sdf::DrawBuffer::newBmp(int32_t w, int32_t h) {
	auto oldBmp = std::move(buttonBmp_);
	COUT(tt_("new bmp:") << max(w, oldBmp.GetWidth()) << tt_(" - ") << max(h, oldBmp.GetHeight()));
	buttonBmpBuf_ = (uint32_t*)buttonBmp_.CreateDib(max(w, oldBmp.GetWidth()), max(h, oldBmp.GetHeight()));
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
	graph_->SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBilinear);
}


void sdf::Control::removeOpenControl(Control* cont) {
	for (intptr_t i = controlOpenList_.size() - 1; i >= 0; i--) {
		auto con = controlOpenList_[i];
		if (con == cont) {
			controlOpenList_.erase(controlOpenList_.begin() + i);
			break;
		}
	}
}

bool sdf::Control::setClipboardText(df::CCa text) {
	return setClipboardText(df::code::UTF8ToWide(text));
}

bool sdf::Control::setClipboardText(df::CCw text) {
	if (OpenClipboard(currentHandle_)) {
		EmptyClipboard();
		HANDLE hData = GlobalAlloc(GMEM_MOVEABLE, text.bytes() + 2);
		LPWSTR pData = (LPWSTR)GlobalLock(hData);
		CopyMemory(pData, text.data(), text.bytes());
		pData[text.size()] = 0;
		GlobalUnlock(hData);
		SetClipboardData(CF_UNICODETEXT, hData);
		CloseClipboard();
		return true;
	}
	return false;
}

sdf::DrawBuffer* sdf::Control::getDraw()
{
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

sdf::Window* sdf::Control::getWindow() {
	Control* top = getTopParent();
	if (top) {
		return dynamic_cast<Window*>(top);
	}
	return nullptr;
}

sdf::DrawBuffer* sdf::Control::getDrawLayer(int32_t& x, int32_t& y, int32_t w, int32_t h, bool clear)
{
	if (!drawLayer_) {
		drawLayer_ = new DrawBuffer(nullptr);
		drawLayer_->newBmp(w, h);
		drawLayer_->x_ = x;
		drawLayer_->y_ = y;
		x = 0;
		y = 0;
	}
	else if (clear) {
		if (drawLayer_->buttonBmp_.GetWidth() < w || drawLayer_->buttonBmp_.GetHeight() < h) {
			drawLayer_->newBmp(w, h);
		}
		drawLayer_->clear();
		drawLayer_->x_ = x;
		drawLayer_->y_ = y;
		x = 0;
		y = 0;
	}

	return drawLayer_;
}

void sdf::Control::clearDrawLayer()
{

}

void sdf::Control::drawParentLayer()
{
	auto par = this->parent_;
	while (par) {
		if (par->drawLayer_) {
			par->layerToDraw(par->getDraw());
		}
		par = par->parent_;
	}
}

void sdf::Control::layerToDraw(DrawBuffer* draw)
{
	if (drawLayer_ && draw) {
		drawLayer_->buttonBmp_.DrawAlphaTo(draw->buttonBmp_, drawLayer_->x_, drawLayer_->y_,
			drawLayer_->buttonBmp_.GetWidth(), drawLayer_->buttonBmp_.GetHeight()
		);
	}
}

bool sdf::Control::drawStyle(DrawBuffer* draw, ControlStyle& style, bool parentBack, bool brawText) {
	if (!draw)
		return false;

	lastDrawStyle = &style;
	int32_t drawX = drawX_;
	int32_t drawY = drawY_;

	RECT rect;
	rect.left = drawX;
	rect.top = drawY;
	rect.right = drawX + GetWidth();
	rect.bottom = drawY + GetHeight();

	int32_t w = GetWidth();
	int32_t h = GetHeight();

	if (!draw->buttonBmpBuf_ || draw->buttonBmp_.GetWidth() < drawX + w || draw->buttonBmp_.GetHeight() < drawY + h) {
		draw->newBmp(drawX + w, drawY + h);
	}
	uint32_t* buf = draw->buttonBmpBuf_;
	if (buf == nullptr)
		return false;

	int bufW = draw->buttonBmp_.GetWidth();


	if (isFocused) {
		//but.buttonGdi_.SetPen(WhiteDotPen_);
		//but.buttonGdi_.SetBrush(Brush::GetNullBrush());
		//but.buttonGdi_.Rect(but.buttonRect_.left + 3, but.buttonRect_.top + 3, but.buttonRect_.right - 3, but.buttonRect_.bottom - 3);
	}

	if (style.backColor != 0) {

		if (style.radius == 360) {

			if (parent_ && parent_->needDraw)
				drawParentBack(draw);
			Gdiplus::SolidBrush brush(Gdiplus::Color((Gdiplus::ARGB)style.backColor));
			Gdiplus::Rect ellipseRect2(drawX_, drawY_, w, h);

			draw->graph_->FillEllipse(&brush, ellipseRect2);
		}
		else {
			//矩形背景
			//df::TickClock([&] {
			drawRect(buf, bufW, drawX, drawY, w, h, style.backColor);
			//}, 1);

		}

	}
	else if (!style.backImage && parentBack && parent_) {
		drawParentBack(draw);
	}

	//画阴影
	int shadowLen = std::abs(style.shadowSize);

	if (shadowLen > 0 && shadowLen < 1023) {
		uint32_t col[1024] = { 0 };//渐变

		for (int i = 1; i <= shadowLen; i++) {
			col[i - 1] = Color::mixColor(style.backColor, 7 * i);
		}

		//横向阴影
		for (int32_t y = df::positive(drawY); y < drawY + shadowLen; y++) {
			for (int32_t i = df::positive(y - drawY + drawX); i < rect.right; i++) {
				buf[y * bufW + i] = col[shadowLen - 1 - (y - drawY)];
			}
		}
		//纵向
		for (int32_t y = df::positive(drawX); y < drawX + shadowLen; y++) {
			for (int32_t i = df::positive(y - drawX + drawY); i < rect.bottom; i++) {
				buf[y + i * bufW] = col[shadowLen - 1 - (y - drawX)];
			}
		}
		rect.top += (uint32_t)(2 * scale_);
		rect.left += (uint32_t)(2 * scale_);
	}

	if (style.borderTop > 0) {
		for (int y = df::positive(drawY); y < drawY + style.borderTop; y++) {
			for (int i = df::positive(drawX); i < drawX + w; i++) {
				buf[y * bufW + i] = style.borderColor;
			}
		}
	}
	if (style.borderBottom > 0 && h > 0) {
		for (int y = df::positive(drawY + h - style.borderBottom); y < drawY + h; y++) {
			for (int i = df::positive(drawX); i < drawX + w; i++) {
				buf[y * bufW + i] = style.borderColor;
			}
		}
	}
	if (style.borderLeft > 0) {
		for (int y = df::positive(drawX); y < drawX + style.borderLeft; y++) {
			for (int i = df::positive(drawY); i < drawY + h; i++) {
				buf[y + i * bufW] = style.borderColor;
			}
		}
	}
	if (style.borderRight > 0 && w > 0) {
		for (int y = df::positive(drawX + w - style.borderRight); y < drawX + w; y++) {
			for (int i = df::positive(drawY); i < drawY + h; i++) {
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

	if (brawText && text.length() > 0) {
		draw->buttonBmp_.SetTextColor(style.color);
		if (style.font.hasFont()) {
			draw->buttonBmp_.setFont(Font::getFont(style.font));
		}
		else if (this->style.font.hasFont()) {
			draw->buttonBmp_.setFont(Font::getFont(this->style.font));
		}
		else {
			draw->buttonBmp_.setFont(Control::GlobalFont());
		}
		onDrawText(rect, style, draw);
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

void sdf::Control::drawRect(uint32_t* buf, int32_t bufW, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color) {
	for (int32_t yy = df::positive(y); yy < y + h; yy++) {
		for (int32_t xx = df::positive(x); xx < x + w; xx++) {
			buf[yy * bufW + xx] = color;
		}
	}
}

void recurRemoveHandle(sdf::Control* con) {
	if (con->GetHandle()) {
		//con->setHW(0, 0);
		con->hide();
		//::SetParent(con->GetHandle(), 0);
		return;
	}

	for (auto& c : con->memberList_) {
		recurRemoveHandle(c.get());
	}
}

void sdf::Control::_removeFromParent(bool remove) {
	auto par = parent_;

	if (parent_ && parent_->hoverView == this) {
		parent_->doLeave(false);
	}
	else
		doLeave(false);


	if (par) {
		recurRemoveHandle(this);
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

void recurSetHandle(const std::shared_ptr<sdf::Control>& con, HWND handl) {
	if (con->GetHandle()) {
		::SetParent(con->GetHandle(), handl);
		con->show();
		return;
	}
	for (auto& c : con->memberList_) {
		recurSetHandle(c, handl);
	}
}

void sdf::Control::addMember(const std::shared_ptr<Control>& con, bool update) {
	DF_ASSERT(con->parent_ == nullptr);
	con->parent_ = this;
	memberList_.push_back(con);

	if (inited) {
		if (!con->inited) {
			measureX_ = 0;
			measureY_ = 0;
			con->doCreate();
			con->Init();
		}
		else {
			auto handl = getParentHandle();
			if (handl)
				recurSetHandle(con, handl);
		}

		if (update) {
			measureUpdate();
		}
	}

}


void sdf::Button::onDraw() {
	if (!parent_)
		return;

	updateDrawXY();

	DrawBuffer* draw = getDraw();
	bool drawSub = true;

	//df::TickClock([&] {
	if (isPress) {
		drawSub = drawStyle(draw, stylePress, parent_->needDraw);
	}
	else if (isDisable) {
		drawSub = drawStyle(draw, styleDisable, parent_->needDraw);
	}
	else if (isCheck) {
		if (isHover) {
			auto oldBack = styleCheck.backColor;
			styleCheck.backColor = styleHover.backColor;
			drawSub = drawStyle(draw, styleCheck, parent_->needDraw);
			styleCheck.backColor = oldBack;
		}
		else
			drawSub = drawStyle(draw, styleCheck, parent_->needDraw);
	}
	else if (isHover) {
		drawSub = drawStyle(draw, styleHover, parent_->needDraw);
	}
	else {
		drawSub = drawStyle(draw, style, parent_->needDraw);
	}
	//}, 10);


	if (drawSub) {

		drawMember(draw);
	}

	//if (drawSub) {
	//	COUT(tt_("draw button"));
	//}

}


////////////////////////////////////TextBox//////////////////////////////////////

void sdf::Control::fixPos() {
	if (!parent_)
		return;

	Control* pare = parent_;

	int32_t x = style.borderLeft + pos.paddingLeft + pos.x;
	int32_t y = style.borderTop + pos.paddingTop + pos.y;

	do {

		ScrollView* sv = dynamic_cast<ScrollView*>(pare);
		if (sv) {
			x = x - sv->getHoriPos();
			y = y - sv->getVertPos();
			if (x != showX_ || y != showY_) {
				setPos(x, y, false);
			}
			return;
		}
		else {
			x += pare->pos.x;
			y += pare->pos.y;
		}

	} while (pare = pare->getParent());

}

void sdf::TextBox::onMeasure() {
	Control::onMeasure();
	auto wNo = getWidthNoBorder();
	auto hNo = getHeighNoBorder();
	if (wNo != showW_ || hNo != showH_)
		setHW(wNo, hNo);
}

void sdf::TextBox::onMouseMove(int32_t x, int32_t y) {
	/*int32_t xx = getShowX();
	int32_t yy = getShowY();

	if (xx != showX_ || yy != showY_) {
		setPos(xx, yy, false);
	}*/

	doParentMove();
	Control::onMouseMove(x, y);
}

void sdf::TextBox::onDraw() {
	//COUT(tt_("重绘TextBox"));
	if (pos.w > 0 && pos.h > 0) {
		updateDrawXY();

		DrawBuffer* draw = getDraw();
		if (focused)
			drawStyle(draw, styleFocus, parent_->needDraw, false);
		else
			drawStyle(draw, style, parent_->needDraw, false);

		//update();
		updateHandleXy(gdi_, draw);
	}
}

bool sdf::TextBox::onLeftDown()
{
	return false;
}


void sdf::TextBox::Init() {
	onMeasure();

	DWORD sty = WS_TABSTOP | WS_CHILD | WS_VISIBLE |
		ES_LEFT;

	if (!isEnable) {
		sty |= WS_DISABLED;
	}
	if (mutiLine) {
		sty |= ES_MULTILINE | ES_AUTOVSCROLL;
	}
	else {
		sty |= ES_AUTOHSCROLL;
	}

	if (showVScroll) {
		sty |= WS_VSCROLL;
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
	auto y = getShowY();
	handle_ = CreateWindowEx(
		//WS_EX_CLIENTEDGE,
		0,
		tt_("EDIT"),  // Predefined class; Unicode assumed
		text.c_str(),      // Button text
		sty,   // Styles  |BS_OWNERDRAW
		getShowX(),         // x position
		y,         // y position
		getWidthNoBorder(),        // Button width
		getHeighNoBorder(),        // Button height
		getParentHandle(),     // Parent window
		NULL,       // No menu.
		Control::progInstance_,
		NULL);      // Pointer not needed.

	if (!handle_) {
		DF_ERR(tt_("CreateWindow EDIT failed!"));
		return;
	}

	Control::Init();

	gdi_.Init(handle_);
	//gdi_.SetPen(Pen::GetWhitePen());
	//gdi_.SetBrush(BlueBrush_);
	//gdi_.SetTextColor(Color::white);
	//文字背景透明
	//gdi_.SetTextBackColor();
	//背景透明
	//gdi_.SetBrush(Brush::GetNullBrush());

	if (style.font.hasFont()) {
		setFont(Font::getFont(style.font));
	}
	else {
		setFont(Window::GlobalFont());
	}
	prevMsgProc_ = (WNDPROC)SetWindowLongPtr(handle_, GWLP_WNDPROC, (LONG_PTR)Control::ButtonProc);
	initAllSub();

}


bool sdf::TextBox::ControlProc(HWND, UINT msg, WPARAM wParam, LPARAM, LRESULT& ret) {

	if (msg == WM_PAINT) {

		fixPos();
		//return false;
	}
	else if (msg == WM_COMMAND) {
		switch (HIWORD(wParam)) {
		case EN_CHANGE:
			if (onChange_)
				onChange_();
			break;
		case EN_SETFOCUS: {
			focused = true;
			onDraw();
			if (onFocus_)
				onFocus_();
			break;
		}
		case EN_KILLFOCUS: {
			focused = false;
			onDraw();
			if (onLeave_)
				onLeave_();
			break;
		}
		}
	}
	return true;
}


/////////////////////////////////ListBox/////////////////////////////////////////


void sdf::ListBox::onMouseMove(int32_t x, int32_t y) {
	doParentMove();
	Control::onMouseMove(x, y);
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

	if (!isEnable) {
		sty |= WS_DISABLED;
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

	gdi_.Init(handle_);

	if (style.font.hasFont())
		setFont(Font::getFont(style.font));
	else
		setFont(Window::GlobalFont());

	prevMsgProc_ = (WNDPROC)SetWindowLongPtr(handle_, GWLP_WNDPROC, (LONG_PTR)Control::ButtonProc);
	initAllSub();
}

bool sdf::ListBox::ControlProc(HWND, UINT msg, WPARAM wParam, LPARAM, LRESULT& ret) {
	if (msg == WM_PAINT) {
		fixPos();
		//updateHandleXy(gdi_, nullptr);
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
	SIZE wid = { 0 };
	GetTextExtentPoint32(currentWindow_->gdi_.GetDc(), str.char_, (int)str.length_, &wid);
	//COUT(wid.cx);
	SendMessage(handle_, LB_SETHORIZONTALEXTENT, (WPARAM)(wid.cx + 5), 0);
	textLength_ = (int)str.length_;
}



///////////////////////////////////ComBox///////////////////////////////////////


void sdf::ComBox::onMouseMove(int32_t x, int32_t y) {
	doParentMove();
	Control::onMouseMove(x, y);
}

void sdf::ComBox::onMeasure() {
	Control::onMeasure();
	auto wNo = getWidthNoBorder();
	auto hNo = getHeighNoBorder();
	if (wNo != showW_ || hNo != showH_)
		setHW(wNo, hNo);
}

void sdf::ComBox::Init() {
	onMeasure();

	DWORD sty = WS_TABSTOP | WS_CHILD | WS_VISIBLE | CBS_AUTOHSCROLL | WS_VSCROLL;


	if (editAble)
		sty |= CBS_DROPDOWN;
	else
		sty |= CBS_DROPDOWNLIST;

	if (!isEnable) {
		sty |= WS_DISABLED;
	}

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

	gdi_.Init(handle_);
	//gdi_.SetPen(Pen::GetWhitePen());
	//gdi_.SetBrush(BlueBrush_);
	//gdi_.SetTextColor(Color::white);
	//文字背景透明
	//gdi_.SetTextBackColor();
	//背景透明
	//gdi_.SetBrush(Brush::GetNullBrush());

	if (style.font.hasFont())
		setFont(Font::getFont(style.font));
	else
		setFont(Window::GlobalFont());


	prevMsgProc_ = (WNDPROC)SetWindowLongPtr(handle_, GWLP_WNDPROC, (LONG_PTR)Control::ButtonProc);
	initAllSub();

}

bool sdf::ComBox::ControlProc(HWND, UINT msg, WPARAM wParam, LPARAM, LRESULT& ret) {
	if (msg == WM_PAINT) {
		fixPos();
		//updateHandleXy(gdi_, nullptr);
		//return false;
	}

	if (msg == WM_COMMAND) {

		auto lp = LOWORD(wParam);
		switch (HIWORD(wParam)) {
		case CBN_SELCHANGE:
			if (onSelectChange_ && lp == 0)
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




bool sdf::CheckBox::onLeftUp() {

	DF_SCOPE_GUARD
	{
		isPress = false;
		onDraw();
	};

	if (isPress) {
		CheckGroup* check = dynamic_cast <CheckGroup*>(parent_);
		if (check) {
			check->setCheck(this);
		}
		else {
			isCheck = !isCheck;
		}
		if (onClick_) {
			onClick_();
		}
	}

	return false;
}

void sdf::CheckBox::onDrawText(RECT& rect, ControlStyle& style, DrawBuffer* draw) {

	if (!draw)
		return;

	if (button) {
		return Button::onDrawText(rect, style, draw);
	}

	//COUT(tt_("draw checkbox"));
	int32_t drawX = drawX_ + pos.paddingLeft;
	int32_t drawY = drawY_ + pos.paddingTop;

	int32_t size = (int32_t)(1 * scale_);
	int32_t w = pos.h - pos.paddingTop - pos.paddingBottom;
	int bufW = draw->buttonBmp_.GetWidth();
	uint32_t* buf = (uint32_t*)draw->buttonBmpBuf_;

	uint32_t borderColor = style.borderColor;

	if (isDisable) {
		if (dot) {
			Gdiplus::Pen bluePen(Gdiplus::Color(styleDisable.borderColor), (float)size * 2);
			// Create a Rect object that bounds the ellipse.
			Gdiplus::RectF ellipseRect((float)drawX, (float)drawY, (float)w, (float)w);
			draw->graph_->DrawEllipse(&bluePen, ellipseRect);
		}
		else {
			//横
			for (int y = df::positive(drawY); y < drawY + size; y++) {
				for (int i = df::positive(drawX + size); i < drawX + w - size; i++) {
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
			for (int y = df::positive(drawY); y < drawY + size; y++) {
				for (int i = df::positive(drawX + size); i < drawX + w - size; i++) {
					buf[y * bufW + i] = borderColor;
					if ((y + w - size) * bufW + i >= 0)
						buf[(y + w - size) * bufW + i] = borderColor;
				}
			}

			//中间
			for (int y = df::positive(drawY + size); y < drawY + w - size; y++) {
				for (int i = df::positive(drawX); i < drawX + w; i++) {
					buf[y * bufW + i] = borderColor;
				}
			}

			Gdiplus::Pen pen(Gdiplus::Color(Color::white), 2 * scale_);
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
	rect.left = drawX + w + pos.paddingLeft;
	rect.top = getDrawY();

	draw->buttonBmp_.Txt(rect, text, pos.textAlignX, pos.textAlignY);
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
	DF_SCOPE_GUARD
	{
		::FreeResource(lpRsrc);
	};


	///重新申请一块内存
	HGLOBAL m_hMem = GlobalAlloc(GMEM_FIXED, len);
	BYTE* pmem = (BYTE*)GlobalLock(m_hMem);
	memcpy(pmem, lpRsrc, len);

	IStream* pstm;
	CreateStreamOnHGlobal(m_hMem, FALSE, &pstm);
	// load from stream
	releaseImg();
	imgp_ = Gdiplus::Image::FromStream(pstm);

	DF_SCOPE_GUARD
	{
		::GlobalUnlock(m_hMem);
		pstm->Release();
		::GlobalFree(m_hMem);
	};


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

char* sdf::Bitmap::CreateDib(int w, int h, int bitCount) {
	Init();
	////////////////////
	BITMAPINFO info = { {0} };
	info.bmiHeader.biSize = sizeof(info.bmiHeader);
	info.bmiHeader.biWidth = w;
	//info.bmiHeader.biHeight        = h;
	info.bmiHeader.biHeight = -h;
	info.bmiHeader.biPlanes = 1;
	info.bmiHeader.biBitCount = bitCount;
	info.bmiHeader.biCompression = BI_RGB;
	info.bmiHeader.biSizeImage = w * h * (bitCount / 8);

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


void sdf::FormOk::onCreate() {
	//v.topMost = true;
	v.maxBox = false;
	v.minBox = false;
	v.resizeAble = false;
	v.pos.centerInParent();
	v.pos.w = min(400, (int32_t)getScreenW());

	ui_view
	{
		v.pos.vertical = true;

		v.pos.flexX = 1;
		v.style.backColor = Color::white;

		ui_view
			{
				v.pos.flexX = 1;
				v.pos.textMutiline = true;
				v.pos.padding(20);
				v.text = content;
			};
		ui_ok_view
			{
				v.onOk = [&]
				{
					if (onOk_)
						onOk_();
					close();
				};

				v.onCancel = [&]
				{
					close();
				};
			};

	};


}

void sdf::FormOk::onInit() {
	//toTop();
	v.topMost = false;
	updateStyle();
}


void sdf::FormMenu::onInit() {
	toTop();
	v.topMost = false;
	updateStyle();
}

void sdf::FormMenu::onCreate() {
	//v.pos.centerInParent();
	v.noBorder = true;
	v.topMost = true;
	ui_scroll
	{
		if (width > 0)
			v.pos.w = width;
		v.style.backColor = Color::white;
		v.style.border(1);
		v.style.borderColor = Color::blue;

		v.stylePress = v.style;
		v.styleHover = v.style;
		v.styleDisable = v.style;
		v.pos.maxH = 500;
		v.pos.padding(1);

		v.pos.vertical = true;


		for (size_t itemI = 0; itemI < itemList_.size(); itemI++) {
			ui_button
				{
					if (width > 0) {
						v.pos.flexX = 1;
						//v.pos.centerX();
					}
					v.pos.centerY();
					sdf::Button::setMenuStyle(&v);

					ui_onclick
						{
							close();
							if (itemList_[itemI].onClick)
								itemList_[itemI].onClick();
						};
					if (itemList_[itemI].img) {
						ui_image
							{
								v.pos.w = 25;
								v.setBitmap(itemList_[itemI].img);
							};
					}
					ui_view
						{
							v.pos.marginLeft = 5;
							v.pos.marginRight = 5;
							v.text = itemList_[itemI].text;
						};


				};
		}

	};
}


void sdf::Timer::onTimer(uint32_t id) {
	auto it = timerMap.find(id);
	if (it == timerMap.end()) {
		return;
	}
	it->second.func_();
}

sdf::Timer sdf::Timer::set(Control* con, uint32_t time, std::function<void()>&& func) {
	auto id = idCount()++;
	if (!con) {
		Throw_df(tt_("set Timer Error:Null Control"));
	}
	auto topCon = con->getTopParent();
	if (!topCon) {
		topCon = con;
	}
	if (!topCon || topCon->GetHandle() == nullptr) {
		Throw_df(tt_("set Timer Error:Null Handle"));
	}

	auto weakPtr = std::weak_ptr(con->sharedBase<Control>());
	timerMap.emplace(id,
		TimerItem(topCon->GetHandle(), [func = std::move(func), weakPtr]
			{
				if (auto ptr = weakPtr.lock()) {
					func();
				}
			}));
	::SetTimer(topCon->GetHandle(), id, time, NULL);
	return Timer(id);

}

void sdf::Timer::reset() {
	if (id == 0) {
		return;
	}
	auto it = timerMap.find(id);
	if (it == timerMap.end()) {
		return;
	}
	::KillTimer(it->second.handle_, id);
	timerMap.erase(it);
	id = 0;
}


void sdf::TabBar::doCreate() {
	CheckGroup::doCreate();

	tabView_ = dynamic_cast<TabView*>(parent_);
	if (!tabView_) {
		DF_BREAK_POINT_MSG("Can not find parent TabView");
		return;
	}

	if (!tabView_->content_) {
		DF_BREAK_POINT_MSG("Can not find  TabView content");
		return;
	}

	for (auto& v : memberList_) {
		TabButton* bv = dynamic_cast<TabButton*>(v.get());
		if (!bv)
			continue;


		if (bv->isCheck && tabView_->content_->memberList_.size() == 0) {
			Window::runOnUi([this, bv]
				{
					tabView_->content_->addMember(bv->getContentView(), false);
				});

		}

		bv->onClick_ = [this, bv]
		{
			tabView_->content_->removeAllMember();
			tabView_->content_->addMember(bv->getContentView(), true);
		};
	}
}


void sdf::WebView::navigate(const std::wstring& szUrl) {
	url_ = szUrl;
	if (web)
		web->Navigate(szUrl);
}

void sdf::WebView::onMouseMove(int32_t x, int32_t y) {
	doParentMove();
	Control::onMouseMove(x, y);
}

void sdf::WebView::onMeasure() {
	Control::onMeasure();
	auto wNo = pos.w;
	auto hNo = pos.h;
	if (wNo != showW_ || hNo != showH_) {
		setHW(wNo, hNo, true, false);
		if (web) {
			RECT rect;
			rect.left = 0;
			rect.top = 0;
			rect.right = rect.left + wNo;
			rect.bottom = rect.top + hNo;
			web->SetRect(rect);
		}

	}
}

void sdf::WebView::onDraw() {
	updateDrawXY();

	DrawBuffer* draw = getDraw();
	//update();
	updateHandleXy(gdi_, draw);
}

void sdf::WebView::Init() {
	onMeasure();


	int32_t x = getDrawX();
	int32_t y = getDrawY();
	handle_ = CreateWindow(
		tt_("BUTTON"),  // Predefined class; Unicode assumed
		text.c_str(),      // Button text
		WS_VISIBLE | WS_CHILD | BS_OWNERDRAW | WS_CLIPCHILDREN,  // Styles  |BS_OWNERDRAW
		x,         // x position
		y,         // y position
		pos.controlW,        // Button width
		pos.controlH,        // Button height
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
	if (style.font.hasFont())
		gdi_.setFont(Font::getFont(style.font));
	else
		gdi_.setFont(Window::GlobalFont());

	prevMsgProc_ = (WNDPROC)SetWindowLongPtr(handle_, GWLP_WNDPROC, (LONG_PTR)Control::ButtonProc);


	web = std::unique_ptr<df::WebBrowser>(new df::WebBrowser(GetHandle()));
	//设置浏览器大小
	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = rect.left + pos.controlW;
	rect.bottom = rect.top + pos.controlH;
	web->SetRect(rect);


	if (url_.length() > 0) {
		web->Navigate(url_);
	}
}

bool sdf::WebView::ControlProc(HWND, UINT msg, WPARAM wParam, LPARAM, LRESULT& ret) {
	//if (msg == WM_PAINT) {

	//	//fixPos();
	//	ret = 1;
	//	return false;
	//}
	return true;

}
