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
	public:
		Bitmap()
			: img_(0), width_(0), height_(0)
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

		///创建并关联32位ARGB位图
		//返回像素数据首指针(大小w*h*4)
		char * CreateDib(int w,int h)
		{
			Init();
			////////////////////
			BITMAPINFO info                = {{0}};    
			info.bmiHeader.biSize            = sizeof(info.bmiHeader);    
			info.bmiHeader.biWidth            = w;
			//info.bmiHeader.biHeight        = h;
			info.bmiHeader.biHeight        =  - h;    
			info.bmiHeader.biPlanes            = 1;    
			info.bmiHeader.biBitCount        = 32;    
			info.bmiHeader.biCompression    = BI_RGB;    
			info.bmiHeader.biSizeImage        = w * h * (32 / 8);  

			// 创建一块内存纹理并获取其数据指针
			void* pBits = NULL;
			img_ = ::CreateDIBSection(hdc_, &info, DIB_RGB_COLORS, &pBits, NULL, 0);
			if (img_ == 0)
			{
				ERR(_T("CreateDIBSection failed"));
				return nullptr;
			}
			::SelectObject(hdc_, img_);
			//DIBSECTION GDIBSection;
			//::GetObject(img_ , sizeof(DIBSECTION), &GDIBSection);
			
			width_ = w;
			height_ = h;
			//这里指向图像的内存区域
			//return (char *)GDIBSection.dsBm.bmBits;
			return (char*)pBits;
		}

		BOOL Create(int w, int h)
		{
			Init();
			img_ = ::CreateCompatibleBitmap(GetScreen().GetDc(), w, h);
			if (img_ == 0)
			{
				ERR(_T("CreateCompatibleBitmap错误"));
				return false;
			}
			width_ = w;
			height_ = h;
			return ::SelectObject(hdc_, img_) != NULL;
		}

		bool LoadBMP(const CC & name)
		{
			Init();
			img_ = (HBITMAP)::LoadImage(NULL, name.char_, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
			if (img_ == 0)
			{
				ERR(name + t_t(" LoadImage failed"));
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
				ERR(t_t("LoadBitmap failed,id:") + id);
				return false;
			}
			GetHW();
			return ::SelectObject(hdc_, img_) != NULL;
		}

		inline BOOL DrawTo(Hdc toGdi, int toX, int toY)
		{
			return ::BitBlt(toGdi.GetDc(), toX, toY, width_, height_, hdc_, 0, 0, SRCCOPY);
		}

		inline BOOL DrawStretchTo(Hdc toGdi, int toX, int toY, int toW, int toH)
		{
			//  //通过SetStretchBltMode的设置能使StretchBlt在缩放图像更加清晰
			//SetStretchBltMode(hdc, COLORONCOLOR);
			return  ::StretchBlt(toGdi.GetDc(), toX, toY, toW, toH, hdc_, 0, 0, width_, height_, SRCCOPY);
		}

		//透明贴图
		inline BOOL DrawTransparentTo(Hdc toGdi, int toX, int toY, uint col)
		{
			return ::TransparentBlt(toGdi.GetDc(), toX, toY, width_, height_,
				hdc_, 0, 0, width_, height_, col);
		}


	};
}

#endif // Bitmap_h__2013_8_3__12_57
