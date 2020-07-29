#include "WebBrowser.h"

#define HRTEST_E(fn, wstr)  if(FAILED(fn)) {::MessageBox(NULL,wstr,TEXT("´íÎó"),MB_OK);goto RETURN;}

df::WebBrowser::WebBrowser(HWND _hWndParent)
{
	static bool inited = false;
	if (!inited) {
		OleInitialize(NULL);
		inited = true;
	}
	iComRefCount = 0;
	//::SetRect(&rObject, -300, -300, 300, 300);
	hWndParent = _hWndParent;

	if (CreateBrowser() == FALSE)
	{
		return;
	}

	//ShowWindow(GetControlWindow(), SW_SHOW);

	//this->Navigate(_T("about:blank"));
}

df::WebBrowser::~WebBrowser()
{
	Release();

	if (oleObject)
		oleObject->Release();

	if (oleInPlaceObject)
		oleInPlaceObject->Release();
	
	if (webBrowser2) {
		webBrowser2->Release();
	}
	COUT(tt_("WebBrowser gone"));
}

bool df::WebBrowser::CreateBrowser()
{
	HRESULT hr;
	hr = ::OleCreate(CLSID_WebBrowser,
		IID_IOleObject, OLERENDER_DRAW, 0, this, this,
		(void**)&oleObject);

	if (FAILED(hr))
	{
		MessageBox(NULL, _T("Cannot create oleObject CLSID_WebBrowser"),
			_T("Error"),
			MB_ICONERROR);
		return FALSE;
	}

	hr = oleObject->SetClientSite(this);
	hr = OleSetContainedObject(oleObject, TRUE);

	RECT posRect;
	::SetRect(&posRect, -300, -300, 300, 300);
	hr = oleObject->DoVerb(OLEIVERB_INPLACEACTIVATE,
		NULL, this, -1, hWndParent, &posRect);
	if (FAILED(hr))
	{
		MessageBox(NULL, _T("oleObject->DoVerb() failed"),
			_T("Error"),
			MB_ICONERROR);
		return FALSE;
	}

	hr = oleObject->QueryInterface(&webBrowser2);
	if (FAILED(hr))
	{
		MessageBox(NULL, _T("oleObject->QueryInterface(&webBrowser2) failed"),
			_T("Error"),
			MB_ICONERROR);
		return FALSE;
	}


	//* ¹Ò½ÓDWebBrwoser2Event
	IConnectionPointContainer* pCPC = NULL;
	IConnectionPoint* pCP = NULL;
	DWORD dwCookie = 0;
	HRTEST_E(webBrowser2->QueryInterface(IID_IConnectionPointContainer, (void**)&pCPC), TEXT("Ã¶¾ÙIConnectionPointContainer½Ó¿ÚÊ§°Ü"));
	HRTEST_E(pCPC->FindConnectionPoint(DIID_DWebBrowserEvents2, &pCP), TEXT("FindConnectionPointÊ§°Ü"));
	HRTEST_E(pCP->Advise((IUnknown*)(void*)this, &dwCookie), TEXT("IConnectionPoint::AdviseÊ§°Ü"));
	pCP->Release();
	pCPC->Release();

RETURN:

	return TRUE;
}

RECT df::WebBrowser::PixelToHiMetric(const RECT& _rc)
{
	static bool s_initialized = false;
	static int s_pixelsPerInchX, s_pixelsPerInchY;
	if (!s_initialized)
	{
		HDC hdc = ::GetDC(0);
		s_pixelsPerInchX = ::GetDeviceCaps(hdc, LOGPIXELSX);
		s_pixelsPerInchY = ::GetDeviceCaps(hdc, LOGPIXELSY);
		::ReleaseDC(0, hdc);
		s_initialized = true;
	}

	RECT rc;
	rc.left = MulDiv(2540, _rc.left, s_pixelsPerInchX);
	rc.top = MulDiv(2540, _rc.top, s_pixelsPerInchY);
	rc.right = MulDiv(2540, _rc.right, s_pixelsPerInchX);
	rc.bottom = MulDiv(2540, _rc.bottom, s_pixelsPerInchY);
	return rc;
}

void df::WebBrowser::SetRect(const RECT& _rc)
{
	rObject = _rc;

	{
		RECT hiMetricRect = PixelToHiMetric(rObject);
		SIZEL sz;
		sz.cx = hiMetricRect.right - hiMetricRect.left;
		sz.cy = hiMetricRect.bottom - hiMetricRect.top;
		oleObject->SetExtent(DVASPECT_CONTENT, &sz);
	}

	if (oleInPlaceObject != 0)
	{
		oleInPlaceObject->SetObjectRects(&rObject, &rObject);
	}
}

// ----- Control methods -----

void df::WebBrowser::GoBack()
{
	this->webBrowser2->GoBack();
}

void df::WebBrowser::GoForward()
{
	this->webBrowser2->GoForward();
}

void df::WebBrowser::Refresh()
{
	this->webBrowser2->Refresh();
}

void df::WebBrowser::Navigate(const std::wstring& szUrl)
{
	bstr_t url(szUrl.c_str());
	variant_t flags(0x02u); //navNoHistory
	this->webBrowser2->Navigate(url, &flags, 0, 0, 0);
}

inline void df::WebBrowser::OnDocumentCompleted()
{
	COUT(tt_("OnDocumentCompleted"));
}


// IDispatch Methods

HRESULT _stdcall df::WebBrowser::GetTypeInfoCount(
	unsigned int* pctinfo)
{
	return E_NOTIMPL;
}

HRESULT _stdcall df::WebBrowser::GetTypeInfo(
	unsigned int iTInfo,
	LCID lcid,
	ITypeInfo FAR* FAR* ppTInfo)
{
	return E_NOTIMPL;
}

HRESULT _stdcall df::WebBrowser::GetIDsOfNames(
	REFIID riid,
	OLECHAR FAR* FAR* rgszNames,
	unsigned int cNames,
	LCID lcid,
	DISPID FAR* rgDispId
)
{
	return E_NOTIMPL;
}
HRESULT _stdcall df::WebBrowser::Invoke(
	DISPID dispIdMember,
	REFIID riid,
	LCID lcid,
	WORD wFlags,
	DISPPARAMS* pDispParams,
	VARIANT* pVarResult,
	EXCEPINFO* pExcepInfo,
	unsigned int* puArgErr
)
{
	if (dispIdMember == DISPID_DOCUMENTCOMPLETE)
	{
		OnDocumentCompleted();

		return S_OK;
	}
	if (dispIdMember == DISPID_BEFORENAVIGATE2)
	{
		return S_OK;
	}
	return E_NOTIMPL;
}

// ----- IUnknown -----

HRESULT STDMETHODCALLTYPE df::WebBrowser::QueryInterface(REFIID riid,
	void** ppvObject)
{
	if (riid == __uuidof(IUnknown))
	{
		(*ppvObject) = static_cast<IOleClientSite*>(this);
	}
	else if (riid == IID_IDispatch)	*ppvObject = (IDispatch*)this;
	else if (riid == __uuidof(IOleInPlaceSite))
	{
		(*ppvObject) = static_cast<IOleInPlaceSite*>(this);
	}
	else
	{
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}

ULONG STDMETHODCALLTYPE df::WebBrowser::AddRef(void)
{
	return ::InterlockedIncrement(&iComRefCount);
}

ULONG STDMETHODCALLTYPE df::WebBrowser::Release(void)
{
	return ::InterlockedDecrement(&iComRefCount);
}

// ---------- IOleWindow ----------

HRESULT STDMETHODCALLTYPE df::WebBrowser::GetWindow(
	__RPC__deref_out_opt HWND* phwnd)
{
	(*phwnd) = hWndParent;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE df::WebBrowser::ContextSensitiveHelp(
	BOOL fEnterMode)
{
	return E_NOTIMPL;
}

// ---------- IOleInPlaceSite ----------

HRESULT STDMETHODCALLTYPE df::WebBrowser::CanInPlaceActivate(void)
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE df::WebBrowser::OnInPlaceActivate(void)
{
	OleLockRunning(oleObject, TRUE, FALSE);
	oleObject->QueryInterface(&oleInPlaceObject);
	oleInPlaceObject->SetObjectRects(&rObject, &rObject);

	return S_OK;

}

HRESULT STDMETHODCALLTYPE df::WebBrowser::OnUIActivate(void)
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE df::WebBrowser::GetWindowContext(
	__RPC__deref_out_opt IOleInPlaceFrame** ppFrame,
	__RPC__deref_out_opt IOleInPlaceUIWindow** ppDoc,
	__RPC__out LPRECT lprcPosRect,
	__RPC__out LPRECT lprcClipRect,
	__RPC__inout LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
	HWND hwnd = hWndParent;

	(*ppFrame) = NULL;
	(*ppDoc) = NULL;
	(*lprcPosRect).left = rObject.left;
	(*lprcPosRect).top = rObject.top;
	(*lprcPosRect).right = rObject.right;
	(*lprcPosRect).bottom = rObject.bottom;
	*lprcClipRect = *lprcPosRect;

	lpFrameInfo->fMDIApp = false;
	lpFrameInfo->hwndFrame = hwnd;
	lpFrameInfo->haccel = NULL;
	lpFrameInfo->cAccelEntries = 0;

	return S_OK;
}

HRESULT STDMETHODCALLTYPE df::WebBrowser::Scroll(
	SIZE scrollExtant)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE df::WebBrowser::OnUIDeactivate(
	BOOL fUndoable)
{
	return S_OK;
}

HWND df::WebBrowser::GetControlWindow()
{
	if (hWndControl != 0)
		return hWndControl;

	if (oleInPlaceObject == 0)
		return 0;

	oleInPlaceObject->GetWindow(&hWndControl);
	return hWndControl;
}

HRESULT STDMETHODCALLTYPE df::WebBrowser::OnInPlaceDeactivate(void)
{
	hWndControl = 0;
	oleInPlaceObject = 0;

	return S_OK;
}

HRESULT STDMETHODCALLTYPE df::WebBrowser::DiscardUndoState(void)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE df::WebBrowser::DeactivateAndUndo(void)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE df::WebBrowser::OnPosRectChange(
	__RPC__in LPCRECT lprcPosRect)
{
	return E_NOTIMPL;
}

// ---------- IOleClientSite ----------

HRESULT STDMETHODCALLTYPE df::WebBrowser::SaveObject(void)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE df::WebBrowser::GetMoniker(
	DWORD dwAssign,
	DWORD dwWhichMoniker,
	__RPC__deref_out_opt IMoniker** ppmk)
{
	if ((dwAssign == OLEGETMONIKER_ONLYIFTHERE) &&
		(dwWhichMoniker == OLEWHICHMK_CONTAINER))
		return E_FAIL;

	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE df::WebBrowser::GetContainer(
	__RPC__deref_out_opt IOleContainer** ppContainer)
{
	return E_NOINTERFACE;
}

HRESULT STDMETHODCALLTYPE df::WebBrowser::ShowObject(void)
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE df::WebBrowser::OnShowWindow(
	BOOL fShow)
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE df::WebBrowser::RequestNewObjectLayout(void)
{
	return E_NOTIMPL;
}

// ----- IStorage -----

HRESULT STDMETHODCALLTYPE df::WebBrowser::CreateStream(
	__RPC__in_string const OLECHAR* pwcsName,
	DWORD grfMode,
	DWORD reserved1,
	DWORD reserved2,
	__RPC__deref_out_opt IStream** ppstm)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE df::WebBrowser::OpenStream(
	const OLECHAR* pwcsName,
	void* reserved1,
	DWORD grfMode,
	DWORD reserved2,
	IStream** ppstm)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE df::WebBrowser::CreateStorage(
	__RPC__in_string const OLECHAR* pwcsName,
	DWORD grfMode,
	DWORD reserved1,
	DWORD reserved2,
	__RPC__deref_out_opt IStorage** ppstg)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE df::WebBrowser::OpenStorage(
	__RPC__in_opt_string const OLECHAR* pwcsName,
	__RPC__in_opt IStorage* pstgPriority,
	DWORD grfMode,
	__RPC__deref_opt_in_opt SNB snbExclude,
	DWORD reserved,
	__RPC__deref_out_opt IStorage** ppstg)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE df::WebBrowser::CopyTo(
	DWORD ciidExclude,
	const IID* rgiidExclude,
	__RPC__in_opt  SNB snbExclude,
	IStorage* pstgDest)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE df::WebBrowser::MoveElementTo(
	__RPC__in_string const OLECHAR* pwcsName,
	__RPC__in_opt IStorage* pstgDest,
	__RPC__in_string const OLECHAR* pwcsNewName,
	DWORD grfFlags)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE df::WebBrowser::Commit(
	DWORD grfCommitFlags)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE df::WebBrowser::Revert(void)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE df::WebBrowser::EnumElements(
	DWORD reserved1,
	void* reserved2,
	DWORD reserved3,
	IEnumSTATSTG** ppenum)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE df::WebBrowser::DestroyElement(
	__RPC__in_string const OLECHAR* pwcsName)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE df::WebBrowser::RenameElement(
	__RPC__in_string const OLECHAR* pwcsOldName,
	__RPC__in_string const OLECHAR* pwcsNewName)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE df::WebBrowser::SetElementTimes(
	__RPC__in_opt_string const OLECHAR* pwcsName,
	__RPC__in_opt const FILETIME* pctime,
	__RPC__in_opt const FILETIME* patime,
	__RPC__in_opt const FILETIME* pmtime)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE df::WebBrowser::SetClass(
	__RPC__in REFCLSID clsid)
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE df::WebBrowser::SetStateBits(
	DWORD grfStateBits,
	DWORD grfMask)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE df::WebBrowser::Stat(
	__RPC__out STATSTG* pstatstg,
	DWORD grfStatFlag)
{
	return E_NOTIMPL;
}