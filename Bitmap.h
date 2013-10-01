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



		///创建并关联32位ARGB位图
		//返回像素数据首指针(大小w*h*4)
		char * CreateDib(int w, int h);

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

		bool Load(int id, const CC & resType = tcc_("png"));

		bool Load(const CC & name);

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

		inline BOOL DrawAlphaTo(Hdc toGdi, int toX, int toY)
		{
			return DrawAlphaTo(toGdi.GetDc(), toX, toY, width_, height_
				, 0, 0);
		}

		inline BOOL DrawAlphaTo(Hdc toGdi, int toX, int toY, int fromW, int fromH, int fromX = 0, int fromY = 0)
		{
			BLENDFUNCTION  blendFunction;
			blendFunction.BlendFlags = 0; //没用
			blendFunction.AlphaFormat = AC_SRC_ALPHA; //AC_SRC_ALPHA表示图像中包含ALPHA值
			blendFunction.BlendOp = AC_SRC_OVER; //固定
			blendFunction.SourceConstantAlpha = 0xFF; //透明度

			return ::AlphaBlend(toGdi.GetDc(), toX, toY, fromW, fromH
				, hdc_, fromX, fromY, fromW, fromH
				, blendFunction);

		}


	};

	



}

#endif // Bitmap_h__2013_8_3__12_57
