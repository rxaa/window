#ifndef Bitmap_h__2013_8_3__12_57
#define Bitmap_h__2013_8_3__12_57

namespace sdf {

	enum class BitmapScaleType : uint8_t {
		//按比例居中
		center,
		//按比例裁剪铺满
		centerClip,
		//拉伸填满
		fillStretch,
		start,
		startClip,
		repeatX,
		repeatY,
		repeatXY,
	};


	class Bitmap : public Gdi {
		DF_DISABLE_COPY_ASSIGN(Bitmap)

	protected:
		HBITMAP img_;
		int width_;
		int height_;
		char* imgBuf_;
		bool hasAlpha = true;
		Gdiplus::Image* imgp_ = 0;
		static std::unordered_map<int32_t, std::shared_ptr<Bitmap>>* bmpBuffer;
	public:

		Bitmap()
			: img_(0), width_(0), height_(0), imgBuf_(nullptr) {
		}

		bool alpha() {
			return hasAlpha;
		}

		static std::shared_ptr<Bitmap> get(int32_t id) {
			auto res = bmpBuffer->find(id);
			if (res != bmpBuffer->end())
				return res->second;

			auto bmp = create(id);
			(*bmpBuffer)[id] = bmp;
			return bmp;
		}

		Bitmap(int id, const df::CC& resType = tcc_("png"))
			: img_(0), width_(0), height_(0), imgBuf_(nullptr) {
			Load(id, resType);
		}

		Bitmap(const df::CC& name)
			: img_(0), width_(0), height_(0), imgBuf_(nullptr) {
			Load(name);
		}

		Bitmap(Bitmap&& bmp) {
			img_ = bmp.img_;
			width_ = bmp.width_;
			height_ = bmp.height_;
			imgBuf_ = bmp.imgBuf_;
			hasAlpha = bmp.hasAlpha;
			hdc_ = bmp.hdc_;
			imgp_ = bmp.imgp_;
			bmp.hdc_ = 0;
			bmp.imgBuf_ = 0;
			bmp.img_ = 0;
			bmp.imgp_ = 0;
		}

		~Bitmap() {
			ReleaseBmp();
			releaseImg();
			ReleaseDc();

		}

		Gdiplus::Image* getImg() {
			return imgp_;
		}

		//从资源文件中加载
		bool Load(int id, const df::CC& resType = tcc_("png"));

		//从文件加载图片
		bool Load(const df::CC& name);

		static std::shared_ptr<Bitmap> create() {
			return std::make_shared<Bitmap>();
		}

		static std::shared_ptr<Bitmap> create(int id, const df::CC& resType = tcc_("png")) {
			return std::make_shared<Bitmap>(id, resType);
		}

		static std::shared_ptr<Bitmap> create(const df::CC& name) {
			return std::make_shared<Bitmap>(name);
		}

		int GetWidth() {
			return width_;
		}

		int GetHeight() {
			return height_;
		}


		inline HBITMAP GetBitmap() {
			return img_;
		}


		void GetHW() {
			BITMAP bm;
			::GetObject(img_, sizeof(bm), &bm);
			width_ = bm.bmWidth;
			height_ = bm.bmHeight;
		}

		char* GetBuf() {
			return imgBuf_;
		}


		///创建并关联32位ARGB位图
		//返回像素数据首指针(大小w*h*4)
		char* CreateDib(int w, int h,int bitCount=32);

		BOOL CreateBitmap(int w, int h) {
			Init();
			img_ = ::CreateCompatibleBitmap(GetScreen().GetDc(), w, h);
			if (img_ == 0) {
				DF_ERR(_T("CreateCompatibleBitmap failed!"));
				return false;
			}
			width_ = w;
			height_ = h;
			return ::SelectObject(hdc_, img_) != NULL;
		}


		bool LoadBMP(const df::CC& name) {
			Init();
			img_ = (HBITMAP) ::LoadImage(NULL, name.char_, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
			if (img_ == 0) {
				DF_ERR(name << tcc_(" LoadImage failed"));
				return false;
			}
			GetHW();
			return ::SelectObject(hdc_, img_) != NULL;
		}

		bool LoadBMP(int id);

		inline BOOL DrawTo(Hdc toGdi) {
			return ::BitBlt(toGdi.GetDc(), 0, 0, width_, height_, hdc_, 0, 0, SRCCOPY);
		}

		inline BOOL DrawTo(Hdc toGdi, int toX, int toY) {
			return ::BitBlt(toGdi.GetDc(), toX, toY, width_, height_, hdc_, 0, 0, SRCCOPY);
		}

		inline BOOL DrawStretchTo(Hdc toGdi, int toX, int toY, int toW, int toH) {
			//  //通过SetStretchBltMode的设置能使StretchBlt在缩放图像更加清晰
			//SetStretchBltMode(toGdi.GetDc(), COLORONCOLOR);
			return ::StretchBlt(toGdi.GetDc(), toX, toY, toW, toH, hdc_, 0, 0, width_, height_, SRCCOPY);
		}

		inline BOOL DrawStretchTo(Hdc toGdi, int toX, int toY, int toW, int toH, int fromW, int fromH, int fromX = 0,
			int fromY = 0) {
			return ::StretchBlt(toGdi.GetDc(), toX, toY, toW, toH, hdc_, fromX, fromY, fromW, fromH, SRCCOPY);
		}

		//透明贴图
		inline BOOL DrawTransparentTo(Hdc toGdi, int toX, int toY, uint32_t col) {
			return ::TransparentBlt(toGdi.GetDc(), toX, toY, width_, height_,
				hdc_, 0, 0, width_, height_, col);
		}

		inline BOOL DrawAlphaTo(Hdc toGdi, int toX, int toY) {
			return DrawAlphaTo(toGdi.GetDc(), toX, toY, width_, height_, 0, 0);
		}

		inline BOOL DrawAlphaTo(Hdc toGdi, int toX, int toY, int toW, int toH) {
			BLENDFUNCTION blendFunction;
			blendFunction.BlendFlags = 0; //没用
			blendFunction.AlphaFormat = AC_SRC_ALPHA; //AC_SRC_ALPHA表示图像中包含ALPHA值
			blendFunction.BlendOp = AC_SRC_OVER; //固定
			blendFunction.SourceConstantAlpha = 0xFF; //透明度

			return ::AlphaBlend(toGdi.GetDc(), toX, toY, toW, toH, hdc_, 0, 0, width_, height_, blendFunction);

		}

		inline BOOL
			DrawAlphaTo(Hdc toGdi, int toX, int toY, int toW, int toH, int fromW, int fromH, int fromX = 0, int fromY = 0) {
			BLENDFUNCTION blendFunction;
			blendFunction.BlendFlags = 0; //没用
			blendFunction.AlphaFormat = AC_SRC_ALPHA; //AC_SRC_ALPHA表示图像中包含ALPHA值
			blendFunction.BlendOp = AC_SRC_OVER; //固定
			blendFunction.SourceConstantAlpha = 0xFF; //透明度

			return ::AlphaBlend(toGdi.GetDc(), toX, toY, toW, toH, hdc_, fromX, fromY, fromW, fromH, blendFunction);

		}

	protected:

		void releaseImg() {
			if (imgp_) {
				delete imgp_;
				imgp_ = 0;
			}
		}

		void ReleaseBmp() {
			if (img_) {
				::DeleteObject(img_);
				img_ = 0;
			}


		}

		void releaseOld() {
			Gdip::Init();
			ReleaseBmp();
			ReleaseDc();
		}

		void Init() {
			releaseOld();
			hdc_ = ::CreateCompatibleDC(GetScreen().GetDc());
			//图像缩放算法
			SetStretchBltMode(hdc_, HALFTONE);
		}

	};


	typedef std::shared_ptr<sdf::Bitmap> PtrBmp;

}

#endif // Bitmap_h__2013_8_3__12_57
