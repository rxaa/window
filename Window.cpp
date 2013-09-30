#include "StdAfx.h"

#include "Control.h"

THREAD_LOCAL_VAR HWND sdf::Control::currentHandle_ = NULL;
sdf::Window * sdf::Control::currentWindow_ = nullptr;



HWND sdf::Control::mouseHandle_ = 0;
LONG_PTR sdf::Control::buttonPro_ = 0;
LONG_PTR sdf::TabControl::tabProc_ = 0;

sdf::Bitmap sdf::Button::buttonBmp_;
char * sdf::Button::buttonBmpBuf_ = nullptr;

sdf::Bitmap sdf::Window::backGround_;
sdf::Brush sdf::Window::backBrush_;


int  sdf::Window::mouseY_ = 0;

int  sdf::Window::mouseX_ = 0;

ULONG_PTR sdf::Gdip::gdiplusToken_ = 0;

Gdiplus::GdiplusStartupInput sdf::Gdip::gdiplusStartupInput_ = 0;

////////////////////////////////Brush//////////////////////////////////////////

void sdf::Brush::SetFromBitmap(Bitmap & bmp)
{
	ReleaseBrush();
	brush_ = CreatePatternBrush(bmp.GetBitmap());
}

////////////////////////////////Window//////////////////////////////////////////

sdf::Window::Window(void)
{
	onClose_ = [&]{
		Close();
	};
}


sdf::Window::~Window(void)
{
}

intptr_t __stdcall sdf::Window::ModalProc(HWND hDlg, uint message, WPARAM wParam, LPARAM lParam)
{


	if (message == WM_INITDIALOG)
	{
		Window * winP = (Window *)(lParam);
		if (winP == nullptr)
			return FALSE;

		winP->handle_ = hDlg;
		currentHandle_ = winP->handle_;
		currentWindow_ = winP;

		winP->InitUserData();
		winP->InitWinData();


		return TRUE;
	}

	return  WndProc(hDlg, message, wParam, lParam);
}

intptr_t __stdcall sdf::Window::WndProc(HWND hDlg, uint message, WPARAM wParam, LPARAM lParam)
{
	try
	{
		///��ȡ���ڶ���ָ��

		Window * winP = (Window *)GetUserData(hDlg);
		if (winP == nullptr)
		{
			//currentHandle_ = 0;
			currentWindow_ = nullptr;
			return FALSE;
		}

		if (message == WM_DESTROY)
		{
			winP->Release();
			return TRUE;
		}

		//currentHandle_ = hDlg;
		currentWindow_ = winP;

		switch (message)
		{
		case WM_COMMAND:
			{
				auto l = LOWORD(wParam);
				//auto h = HIWORD(wParam);

				if (l == IDCANCEL)
				{
					if (winP->onClose_)
						winP->onClose_();
					return TRUE;
				}

				if (lParam)
				{
					Control * controlP = GetUserData((HWND)lParam);
					if (controlP)
					{
						controlP->ControlProc(hDlg, message, wParam, lParam);
						return TRUE;
					}
				}

				auto it = winP->commandMap_.find(l);
				if (it != winP->commandMap_.end())
				{
					it->second();
				}

				return TRUE;
					   }
		case WM_NOTIFY:
			{
				LPNMHDR lpn = (LPNMHDR)lParam;
				Control * controlP = GetUserData(lpn->hwndFrom);
				if (controlP)
					controlP->ControlNotify(lpn);
				return TRUE;
					  }
		case WM_MOUSEMOVE:
			{
				GetMousePos(lParam);
				//����Ƴ��ؼ�
				if (mouseHandle_)
				{
					Button * bp = (Button *)GetUserData(mouseHandle_);
					if (bp == nullptr)
						return TRUE;

					bp->buttonState = Button::StateNormal;
					//����뿪��ͼ
					if (bp->onDraw_)
					{
						//RECT itemRect;
						//GetClientRect(WinVar::mouseHandle_, &itemRect); 
						//��ⰴť�Ƿ����
						//BOOL bIsDisabled=IsWindowEnabled(WinVar::mouseHandle_);
						(bp->onDraw_)(*bp);
					}
					mouseHandle_ = 0;
				}
				return TRUE;
						 }
		case WM_RBUTTONDOWN:
			winP->OnMouseRight(true);
			return TRUE;
		case WM_RBUTTONUP:
			winP->OnMouseRight(false);
			return TRUE;
		case WM_LBUTTONDOWN:
			winP->OnMouseLeft(true);
			return TRUE;
		case WM_LBUTTONUP:
			winP->OnMouseLeft(false);
			return TRUE;
		case WM_TIMER:
			winP->OnTimer((uint)wParam);
			return TRUE;
		case WM_PAINT:
			{
				//PAINTSTRUCT ps={0} ;
				//HDC dd=BeginPaint (hDlg, &ps) ;
				//COUT(t_t("dc:")<<winP->gdi_.GetDc()<<t_t(" - ")<<dd);
				winP->OnPaint();
				//EndPaint (hDlg, &ps) ;
				return FALSE;
					 }
			//case WM_ERASEBKGND:
		case WM_DRAWITEM:
			{
				//����һ��ָ��DRAWITEMSTRUCT�ṹ���ָ�벢����ָ��洢�Ű�ť������Ϣ��lParam
				LPDRAWITEMSTRUCT lpDIS = (LPDRAWITEMSTRUCT)lParam;
				Control * controlP = GetUserData(lpDIS->hwndItem);
				if (controlP)
					controlP->ControlProc(hDlg, message, wParam, lParam);
				return TRUE;
						}
		case Tray::TRAY_MESSAGE:
			//cout<<"hwnd:"<<(int)hDlg<<endl;
			if (LOWORD(lParam) == WM_RBUTTONUP) //�Ҽ��˵�
			{
				Tray::OnRightClick()();
			}
			else if (LOWORD(lParam) == WM_LBUTTONUP) //�����ʾ����
			{
				Tray::OnLeftClick()();
			}
			return TRUE;
		case WM_CTLCOLORBTN:
			::SetBkMode((HDC)wParam, TRANSPARENT);
			return (intptr_t)Brush::GetNullBrush();
		case WM_CTLCOLORSTATIC:
			::SetBkMode((HDC)wParam, TRANSPARENT);
		case WM_CTLCOLORDLG:
			return (intptr_t)winP->OnDrawBackground();
		}

	}CATCH_SEH;
	return  FALSE;
}

void sdf::Window::Open(HWND parent/*=0*/, bool show)
{
	isModal_ = false;
	MY_ASSERT(resourceID_ != 0);


	handle_ = ::CreateDialogParam(df::Global::progressInstance_, MAKEINTRESOURCE(resourceID_), parent, WndProc, 0);
	if (handle_ == NULL)
	{
		ERR(t_t("CreateDialog failed"));
		Release();
		return;
	}

	InitUserData();

	HWND oldHandle = currentHandle_;
	//Window * oldWindow = currentWindow_;
	currentHandle_ = handle_;
	currentWindow_ = this;

	::ShowWindow(handle_, show);
	InitWinData();

	::UpdateWindow(handle_);
	currentHandle_ = oldHandle;
	//currentWindow_=oldWindow;
}

void sdf::Window::OnInit()
{

}

void sdf::Window::Release()
{
	currentHandle_ = 0;
	currentWindow_ = nullptr;
	ReleaseUserData();
	if (PtrDecRef() == 0)
		delete this;
}

void sdf::Window::MessageLoop()
{
	MSG msg;
	while (::GetMessage(&msg, NULL, 0, 0))
	{
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

void sdf::Window::Close(bool exit/*=false*/)
{
	(isModal_) ? ::EndDialog(handle_, 0) : ::DestroyWindow(handle_);
	if (exit)
		::PostQuitMessage(0);
	mouseHandle_ = 0;
}

void sdf::Window::Run(bool show/*=true*/)
{

	onClose_ = [&]{
		Close(true);
	};

	Open(nullptr, show);
	MessageLoop();
}

void sdf::Window::OpenModal(HWND parent)
{
	isModal_ = true;
	MY_ASSERT(resourceID_ != 0);
	::DialogBoxParam(df::Global::progressInstance_, MAKEINTRESOURCE(resourceID_), parent, ModalProc, (LPARAM)this);
}

void sdf::Window::InitImage(int backGround)
{
	backGround_.LoadBMP(backGround);
	backBrush_.SetFromBitmap(backGround_);
	Gdip::Init();
}

HBRUSH sdf::Window::OnDrawBackground()
{
	if (backBrush_.GetBrush())
		return backBrush_.GetBrush();
	else
		return NULL;
}

void sdf::Window::OnPaint()
{
	//COUT(L"�ػ�");
}

struct PopMsgStruct
{
	sdf::SS msg_;
	int transparent_;
	int time_;
	sdf::Font font_;
};

void sdf::Window::PopMessage(const CC & msg,int time)
{
	const TCHAR * szAppName = tt_("_PopMessage");
	static bool isReg = true;
	if (isReg)
	{
		isReg = false;
		WNDCLASS wndclass = { 0 };
		wndclass.style = CS_VREDRAW | CS_HREDRAW;
		wndclass.lpfnWndProc = PopMessageProc;
		wndclass.cbClsExtra = 0;
		wndclass.cbWndExtra = 0;
		wndclass.hInstance = df::Global::progressInstance_;
		wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);;
		wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
		wndclass.hbrBackground = Brush::GetBlackBrush();//���ڱ�Ӱ��ˢΪ��
		wndclass.lpszMenuName = NULL;
		wndclass.lpszClassName = szAppName;

		if (!RegisterClass(&wndclass))
		{
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
		df::Global::progressInstance_,
		NULL);
	if (hwnd == NULL)
	{
		ERR(cct_("CreateWindowEx failed"));
		return;
	}

	::SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)new PopMsgStruct{ msg, 0, time });
	//::ShowWindow(hwnd, 1);
	::SetWindowPos(hwnd, 0, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOACTIVATE);

	//::UpdateWindow(hwnd);

}

LRESULT  __stdcall sdf::Window::PopMessageProc(HWND hDlg, uint message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		// ���÷ֲ����� 
		::SetTimer(hDlg, 1, 20, 0);
		SetWindowLongPtr(hDlg, GWL_EXSTYLE, GetWindowLongPtr(hDlg, GWL_EXSTYLE) | WS_EX_LAYERED);
		// ����͸���� 0 - completely transparent   255 - opaque  
		::SetLayeredWindowAttributes(hDlg, 0, 0, LWA_ALPHA);
		return 0;

	case WM_TIMER:{
		//wParam
		PopMsgStruct * mp = (PopMsgStruct *)::GetWindowLongPtr(hDlg, GWLP_USERDATA);
		if (!mp)
			return 0;
		if (wParam == 1)
		{
			mp->transparent_ += 20;
			::SetLayeredWindowAttributes(hDlg, 0, (BYTE)mp->transparent_, LWA_ALPHA);
			if (mp->transparent_ > 160)
			{
				::KillTimer(hDlg, 1);
				::SetTimer(hDlg, 2, mp->time_, 0);
			}
		}
		else if (wParam == 2)
		{
			::KillTimer(hDlg, 2);
			::SetTimer(hDlg, 3, 20, 0);
		}
		else if (wParam == 3)
		{
			mp->transparent_ -= 40;
			if (mp->transparent_ < 0)
			{
				::KillTimer(hDlg, 3);
				::SetWindowLongPtr(hDlg, GWLP_USERDATA,0);
				::DestroyWindow(hDlg);
				delete mp;
				return 0;
			}
			::SetLayeredWindowAttributes(hDlg, 0, (BYTE)mp->transparent_, LWA_ALPHA);
		}

		return 0;
				  }
	case WM_PAINT:{
		PAINTSTRUCT ps;
		HDC hdc = ::BeginPaint(hDlg, &ps);
		PopMsgStruct * mp = (PopMsgStruct *)::GetWindowLongPtr(hDlg, GWLP_USERDATA);
		if (!mp)
			return 0;

		::SetTextColor(hdc, Color::white);
		::SetBkMode(hdc, TRANSPARENT);
		::SelectObject(hdc, mp->font_.GetFont());
		SIZE wid;
		GetTextExtentPoint32(hdc, mp->msg_.GetBuffer(), mp->msg_.Length(), &wid);

		int h = mp->font_.GetFontSize() * 2;
		RECT rect{ 0, 0, wid.cx + h, h };

		::SetWindowPos(hDlg, 0, (GetScreenWidth() - rect.right) / 2, (GetScreenHeight() - h) / 2, rect.right, h, SWP_NOZORDER | SWP_NOCOPYBITS | SWP_NOACTIVATE);

		::DrawText(hdc, mp->msg_.GetBuffer(), mp->msg_.Length(), &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

		::EndPaint(hDlg, &ps);

		return 0; }
	}
	return DefWindowProc(hDlg, message, wParam, lParam);
}


////////////////////////////////Control//////////////////////////////////////////

bool sdf::Control::PopMenu(int menuId, WinHandle hWnd)
{
	HMENU hMenu = ::LoadMenu(df::Global::progressInstance_, MAKEINTRESOURCE(menuId));
	if (!hMenu)
	{
		ERR(cct_("LoadMenu faild:MenuId=") + menuId);
		return false;
	}

	HMENU hSubMenu = ::GetSubMenu(hMenu, 0);
	if (!hSubMenu)
	{
		ERR2;
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




////////////////////////////////Button//////////////////////////////////////////

///����Դ��ʶ����ʼ��
void sdf::Button::Init(int id)
{
	Control::Init(id);
	//������Ϣ������

	buttonGdi_.Init(handle_);
	//buttonGdi_.SetPen(Pen::GetWhitePen());
	//buttonGdi_.SetBrush(BlueBrush_);
	//buttonGdi_.SetTextColor(Color::white);
	//���ֱ���͸��
	buttonGdi_.SetTextBackColor();
	//����͸��
	//buttonGdi_.SetBrush(Brush::GetNullBrush());
	//ʹ�ø���������
	::SelectObject(buttonGdi_.GetDc(), Window::GetFont(currentHandle_));
	buttonPro_ = SetWindowLongPtr(handle_, GWL_WNDPROC, (LONG_PTR)ButtonProc);
}

bool sdf::Button::ControlProc(HWND, UINT msg, WPARAM, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DRAWITEM:{
		LPDRAWITEMSTRUCT lpDIS = (LPDRAWITEMSTRUCT)lParam;

		isFocused_ = (lpDIS->itemState & ODS_FOCUS) > 0;
		isDisable_ = (lpDIS->itemState & ODS_DISABLED) > 0;

		//butDc_ = lpDIS->hDC;
		//��ť�ľ�������
		buttonRect_ = lpDIS->rcItem;

		if (!onDraw_)
			break;

		if (lpDIS->itemState & ODS_SELECTED)
		{
			buttonState = StatePressed;
		}
		else
		{
			//�ڰ�ť��̧��
			if (mouseHandle_ == lpDIS->hwndItem)
				buttonState = StateHover;
			//�ڰ�ť��̧��
			else
				buttonState = StateNormal;
		}

		onDraw_(*this);
		break;
					 }
	case WM_COMMAND:{

		if (onClick_)
			onClick_();
		break;
					}
	}
	return true;
}

LRESULT  __stdcall sdf::Button::ButtonProc(HWND hDlg, uint message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_MOUSEMOVE:
		{
			Window::GetMousePos(lParam);
			//�������ؼ�
			if (mouseHandle_ == 0)
			{
				Button * bp = (Button *)GetUserData(hDlg);
				if (bp == nullptr)
					break;

				bp->buttonState = StateHover;
				//��������ͼ
				if (bp->onDraw_)
				{
					//RECT itemRect;
					//GetClientRect(hDlg, &itemRect); 
					//BOOL bIsDisabled=IsWindowEnabled(hDlg);
					(bp->onDraw_)(*bp);
				}

				mouseHandle_ = hDlg;
			}
			return TRUE;
					 }
	case WM_LBUTTONDBLCLK:
		PostMessage(hDlg, WM_LBUTTONDOWN, wParam, lParam);
		return TRUE;

		break;
	}
	return CallWindowProc((WNDPROC)buttonPro_, hDlg, message, wParam, lParam);
}



void sdf::Button::DefaultDraw(Button & but, DWORD normal, DWORD hover)
{
	RECT rect = but.buttonRect_;

	int w = rect.right - rect.left;
	int h = rect.bottom - rect.top;

	if (buttonBmp_.GetWidth() < w || buttonBmp_.GetHeight() < h)
	{
		buttonBmpBuf_ = buttonBmp_.CreateDib(w, h);
	}
	int32_t * buf = (int32_t*)buttonBmpBuf_;
	if (buf == nullptr)
		return;
	int bufW = buttonBmp_.GetWidth();

	switch (but.buttonState)
	{
	case StateNormal:
		{
			for (int i = 0; i < h; i++)
			{
				for (int y = 0; y < w; y++)
				{
					buf[i*bufW + y] = normal;
				}
			}
			break;

					}
	case StateHover:
		{
			for (int i = 0; i < h; i++)
			{
				for (int y = 0; y < w; y++)
				{
					buf[i*bufW + y] = hover;
				}
			}
			break;
				   }
	case StatePressed:
		{
			//����Ӱ
			const int shadowSize = 5;

			DWORD col[shadowSize] = { 0 };//����
			for (int i = 1; i <= shadowSize; i++)
			{
				col[shadowSize - i] = Color::AddColor(hover, 24 * i);
			}


			//������Ӱ
			for (int y = 0; y < shadowSize; y++)
			{
				for (int i = y; i < w; i++)
				{
					buf[y*bufW + i] = col[shadowSize - 1 - y];
				}
			}
			//����
			for (int y = 0; y < shadowSize; y++)
			{
				for (int i = y; i < h; i++)
				{
					buf[y + i*bufW] = col[shadowSize - 1 - y];
				}
			}
			//������
			for (int i = shadowSize; i < h; i++)
			{
				for (int y = shadowSize; y < w; y++)
				{
					buf[i*bufW + y] = hover;
				}
			}

			rect.top += 2;
			rect.left += 2;
			break;
					 }
	}

	if (but.isFocused_)
	{
		//but.buttonGdi_.SetPen(WhiteDotPen_);
		//but.buttonGdi_.SetBrush(Brush::GetNullBrush());
		//but.buttonGdi_.Rect(but.buttonRect_.left + 3, but.buttonRect_.top + 3, but.buttonRect_.right - 3, but.buttonRect_.bottom - 3);
	}
	but.buttonGdi_.DrawFrom(buttonBmp_, 0, 0, w, h);


	if (but.isDisable_)
	{
		but.buttonGdi_.SetTextColor(0xAAAAAA);
	}
	else
	{
		but.buttonGdi_.SetTextColor(0xFFFFFF);
	}
	but.buttonGdi_.Txt(rect, but.GetText());
}

////////////////////////////////////TextBox//////////////////////////////////////

bool sdf::TextBox::ControlProc(HWND, UINT msg, WPARAM wParam, LPARAM)
{
	if (msg == WM_COMMAND)
	{
		switch (HIWORD(wParam))
		{
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

bool sdf::ListBox::ControlProc(HWND, UINT msg, WPARAM wParam, LPARAM)
{
	if (msg == WM_COMMAND)
	{
		switch (HIWORD(wParam))
		{
		case LBN_SELCHANGE:
			if (onClick_)
				onClick_();
			break;
		}
	}
	return true;
}


void sdf::ListBox::SetHorizontal(const CC & str)
{
	SIZE wid;
	GetTextExtentPoint32(currentWindow_->gdi_.GetDc(), str.char_, str.length_, &wid);
	COUT(wid.cx);
	SendMessage(handle_, LB_SETHORIZONTALEXTENT, (WPARAM)(wid.cx + 5), 0);
	textLength_ = str.length_;
}



///////////////////////////////////ComBox///////////////////////////////////////

bool sdf::ComBox::ControlProc(HWND, UINT msg, WPARAM wParam, LPARAM)
{
	if (msg == WM_COMMAND)
	{
		switch (HIWORD(wParam))
		{
		case CBN_SELCHANGE:
			if (onSelectChange_)
				onSelectChange_();
			break;
		case CBN_EDITCHANGE:
			if (onTextChange_)
				onTextChange_();
			break;
		}
	}
	return true;
}

///////////////////////////////ChoiceItem///////////////////////////////////////////

void sdf::ChoiceItem::SetCheck(BOOL Check)
{
	SendMessage(handle_, BM_SETCHECK, Check, 0);
	if (parentBox_.onClick_)
		parentBox_.onClick_(parentIndex_);
}

bool sdf::ChoiceItem::ControlProc(HWND, UINT msg, WPARAM wParam, LPARAM)
{
	if (msg == WM_COMMAND)
	{
		switch (HIWORD(wParam))
		{
		case BN_CLICKED:
			if (parentBox_.onClick_)
				parentBox_.onClick_(parentIndex_);
			break;
		}
	}
	return true;
}


//////////////////////////////////ListView////////////////////////////////////////


bool sdf::ListView::ControlNotify(LPNMHDR lpn)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(lpn);
	if (lpn->code == LVN_ITEMCHANGED && pNMLV->uChanged == LVIF_STATE && pNMLV->uNewState & LVIS_SELECTED)
	{
		if (onSelectChange_)
			onSelectChange_(pNMLV->iItem);
	}
	else if (lpn->code == (UINT)NM_DBLCLK)
	{
		if (onDoubleClick_)
			onDoubleClick_(pNMLV->iItem);
	}
	return  true;
}

/////////////////////////////////TabControl/////////////////////////////////////////
LRESULT  __stdcall sdf::TabControl::TabProc(HWND hDlg, uint message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_ERASEBKGND:
		{
			HDC dc = (HDC)wParam;
			SelectObject(dc, Brush::GetBlackBrush());
			SetBkMode(dc, TRANSPARENT);
			Gdi::Rect(dc, -100, -100, 1000, 1000);
			return TRUE;
					  }
	}
	return CallWindowProc((WNDPROC)tabProc_, hDlg, message, wParam, lParam);
}


void sdf::TabControl::Add(const CC & title, Window & win)
{
	TCITEM tie = { 0 };
	tie.pszText = (TCHAR *)title.char_;
	tie.mask = TCIF_TEXT;
	if (TabCtrl_InsertItem(handle_, count_, &tie) == -1)
	{
		ERR(cct_("TabCtrl_InsertItem failed"));
		return;
	}
	winList_.Add(&win);

	if (showWindow_ == nullptr)
	{
		win.Open(handle_);
		showWindow_ = &win;
	}
	else
	{
		win.Open(handle_, false);
	}
	win.SetPos(0, 25);
	count_++;

}

bool sdf::TabControl::ControlProc(HWND, UINT msg, WPARAM, LPARAM)
{
	switch (msg)
	{
	case WM_DRAWITEM:
		{
			//LPDRAWITEMSTRUCT lpDIS = (LPDRAWITEMSTRUCT) lParam;
			//COUT(t_t("11"));
			break;
					}
	}
	return  true;
}

void sdf::TabControl::SwitchWindow(int index)
{
	if (showWindow_)
		showWindow_->Hide();

	MY_ASSERT(index < winList_.Size());

	winList_[index]->Show();
	showWindow_ = winList_[index];

	if (onSelectChange_)
		onSelectChange_(index);
}

bool sdf::TabControl::ControlNotify(LPNMHDR lpn)
{
	if (lpn->code == TCN_SELCHANGE)
	{
		int index = GetSelect();
		if (index < 0)
			return true;
		SwitchWindow(index);
	}
	return true;
}

/////////////////////////////////Tray/////////////////////////////////////////

std::function<void()> & sdf::Tray::OnLeftClick()
{
	static std::function<void()> func = [](){};
	return func;
}

std::function<void()> & sdf::Tray::OnRightClick()
{
	static std::function<void()> func = [](){};
	return func;
}