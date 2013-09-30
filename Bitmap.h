#ifndef Bitmap_h__2013_8_3__12_57
#define Bitmap_h__2013_8_3__12_57

namespace sdf
{
	class Bitmap
		: public Gdi
	{
	private:
		HBITMAP img_;
		int width_;
		int height_;
		char * imgBuf_;
	public:
		Bitmap()
			: img_(0)
			, width_(0)
			, height_(0)
			, imgBuf_(nullptr)
		{
		}

		~Bitmap()
		{
			ReleaseBmp();
		}

		int GetWidth()
		{
			return width_;
		}

		int GetHeight()
		{
			return height_;
		}


		inline HBITMAP GetBitmap()
		{
			return img_;
		}

		void GetHW()
		{
			BITMAP  bm;
			::GetObject(img_, sizeof(bm), &bm);
			width_ = bm.bmWidth;
			height_ = bm.bmHeight;
		}
		char * GetBuf()
		{
			return imgBuf_;
		}

		void ReleaseBmp()
		{
			if (img_)
				::DeleteObject(img_);
		}

		void Init()
		{
			ReleaseBmp();
			ReleaseDc();
			hdc_ = ::CreateCompatibleDC(GetScreen().GetDc());
		}



		///����������32λARGBλͼ
		//��������������ָ��(��Сw*h*4)
		char * CreateDib(int w, int h)
		{
			Init();
			////////////////////
			BITMAPINFO info = { { 0 } };
			info.bmiHeader.biSize = sizeof(info.bmiHeader);
			info.bmiHeader.biWidth = w;
			//info.bmiHeader.biHeight        = h;
			info.bmiHeader.biHeight = -h;
			info.bmiHeader.biPlanes = 1;
			info.bmiHeader.biBitCount = 32;
			info.bmiHeader.biCompression = BI_RGB;
			info.bmiHeader.biSizeImage = w * h * (32 / 8);

			// ����һ���ڴ�������ȡ������ָ��
			void* pBits = NULL;
			img_ = ::CreateDIBSection(hdc_, &info, DIB_RGB_COLORS, &pBits, NULL, 0);
			if (img_ == 0)
			{
				ERR(tcc_("CreateDIBSection failed"));
				return nullptr;
			}
			::SelectObject(hdc_, img_);
			//DIBSECTION GDIBSection;
			//::GetObject(img_ , sizeof(DIBSECTION), &GDIBSection);

			width_ = w;
			height_ = h;
			//����ָ��ͼ����ڴ�����
			//return (char *)GDIBSection.dsBm.bmBits;
			imgBuf_ = (char*)pBits;
			return imgBuf_;
		}

		BOOL Create(int w, int h)
		{
			Init();
			img_ = ::CreateCompatibleBitmap(GetScreen().GetDc(), w, h);
			if (img_ == 0)
			{
				ERR(_T("CreateCompatibleBitmap����"));
				return false;
			}
			width_ = w;
			height_ = h;
			return ::SelectObject(hdc_, img_) != NULL;
		}

		bool Load(int id, const CC & resType = tcc_("png"))
		{
			HRSRC hRsrc = ::FindResource(df::Global::progressInstance_, MAKEINTRESOURCE(id), resType.char_);
			if (!hRsrc)
			{
				ERR(tcc_("FindResource failed"))
					return false;
			}
			// load resource into memory  
			DWORD len = ::SizeofResource(df::Global::progressInstance_, hRsrc);
			HGLOBAL lpRsrc = ::LoadResource(df::Global::progressInstance_, hRsrc);
			ON_EXIT({
				::FreeResource(lpRsrc);
			});

			///��������һ���ڴ�
			HGLOBAL m_hMem = GlobalAlloc(GMEM_FIXED, len);
			BYTE* pmem = (BYTE*)GlobalLock(m_hMem);
			memcpy(pmem, lpRsrc, len);

			IStream* pstm;
			CreateStreamOnHGlobal(m_hMem, FALSE, &pstm);
			// load from stream  
			Gdiplus::Image * img = Gdiplus::Image::FromStream(pstm);
			ON_EXIT({
				delete img;
				::GlobalUnlock(m_hMem);
				pstm->Release();
				::GlobalFree(m_hMem);
			});

			if (img == nullptr || img->GetLastStatus() != Gdiplus::Ok)
			{
				ERR(id << tcc_(" Gdiplus Load Image failed! type:") << resType);
				return false;
			}

			if (CreateDib(img->GetWidth(), img->GetHeight()) == nullptr)
				return false;

			Gdiplus::Graphics g(hdc_);
			g.DrawImage(img, 0, 0, width_, height_);

			return true;
		}

		bool Load(const CC & name)
		{
			const WCHAR * wName = (const WCHAR *)name.char_;
#ifndef UNICODE
			SSw wStr = df::AnsiToWide(name);
			wName = wStr.GetBuffer();
#endif // !UNICODE

			Gdiplus::Image * img = new Gdiplus::Image(wName);
			ON_EXIT({
				delete img;
			});

			if (img == nullptr || img->GetLastStatus() != Gdiplus::Ok)
			{
				ERR(name << tcc_(" Gdiplus Load Image failed!"));
				return false;
			}

			if (CreateDib(img->GetWidth(), img->GetHeight()) == nullptr)
				return false;

			Gdiplus::Graphics g(hdc_);
			g.DrawImage(img, 0, 0, width_, height_);
			return true;
		}




		bool LoadBMP(const CC & name)
		{
			Init();
			img_ = (HBITMAP)::LoadImage(NULL, name.char_, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
			if (img_ == 0)
			{
				ERR(name + tcc_(" LoadImage failed"));
				return false;
			}
			GetHW();
			return ::SelectObject(hdc_, img_) != NULL;
		}
		bool LoadBMP(int id)
		{
			Init();
			img_ = ::LoadBitmap(df::Global::progressInstance_, MAKEINTRESOURCE(id));
			if (img_ == 0)
			{
				ERR(tcc_("LoadBitmap failed,id:") + id);
				return false;
			}
			GetHW();
			return ::SelectObject(hdc_, img_) != NULL;
		}

		inline BOOL DrawTo(Hdc toGdi)
		{
			return ::BitBlt(toGdi.GetDc(), 0, 0, width_, height_, hdc_, 0, 0, SRCCOPY);
		}

		inline BOOL DrawTo(Hdc toGdi, int toX, int toY)
		{
			return ::BitBlt(toGdi.GetDc(), toX, toY, width_, height_, hdc_, 0, 0, SRCCOPY);
		}

		inline BOOL DrawStretchTo(Hdc toGdi, int toX, int toY, int toW, int toH)
		{
			//  //ͨ��SetStretchBltMode��������ʹStretchBlt������ͼ���������
			//SetStretchBltMode(hdc, COLORONCOLOR);
			return  ::StretchBlt(toGdi.GetDc(), toX, toY, toW, toH, hdc_, 0, 0, width_, height_, SRCCOPY);
		}

		//͸����ͼ
		inline BOOL DrawTransparentTo(Hdc toGdi, int toX, int toY, uint col)
		{
			return ::TransparentBlt(toGdi.GetDc(), toX, toY, width_, height_,
				hdc_, 0, 0, width_, height_, col);
		}

		inline BOOL DrawAlphaTo(Hdc toGdi, int toX, int toY)
		{
			return DrawAlphaTo(toGdi.GetDc(), toX, toY, width_, height_
				, 0, 0);
		}

		inline BOOL DrawAlphaTo(Hdc toGdi, int toX, int toY, int fromW, int fromH, int fromX = 0, int fromY = 0)
		{
			BLENDFUNCTION  blendFunction;
			blendFunction.BlendFlags = 0; //û��
			blendFunction.AlphaFormat = AC_SRC_ALPHA; //AC_SRC_ALPHA��ʾͼ���а���ALPHAֵ
			blendFunction.BlendOp = AC_SRC_OVER; //�̶�
			blendFunction.SourceConstantAlpha = 0xFF; //͸����

			return ::AlphaBlend(toGdi.GetDc(), toX, toY, fromW, fromH
				, hdc_, fromX, fromY, fromW, fromH
				, blendFunction);

		}


	};
}

#endif // Bitmap_h__2013_8_3__12_57
