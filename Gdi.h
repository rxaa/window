#ifndef Gdi_h__2013_8_3__10_07
#define Gdi_h__2013_8_3__10_07


namespace sdf {



	struct Color {
		enum {
			//windows灰
			grey = 0xffF0F0F0,
			//黑
			black = 0xff000000,

			greyLight = 0xFF888888,
			greyFont = 0xFF333333,
			//深灰
			greyMid = 0xffBBBBBB,
			//深灰
			darkGrey = 0xffA0A0A0,
			//红色
			red = 0xFFff4028,
			pink = 0xFFffc8d0,

			//紫色
			purple = 0xFFd85ad8,
			//浅蓝
			blueLight = 0xFFd0edfe,
			blueLight2 = 0xFFeef9ff,
			yellow = 0xFFf3dd60,
			yellowLight = 0xFFfff8d1,
			//蓝色
			blue = 0xFF4192E1,
			//深蓝
			barkBlue = 0xff0000FF,
			//白色
			white = 0xFFFFFFFF
		};

		//颜色(ARGB)
		//内存:(BGRA)
		static const uint32_t blueColor_ = 0xFF4192E1;
		static const uint32_t orangeColor_ = 0xFFE19241;
		static const uint32_t greenColor_ = 0xFF1ABC6E;
		static const uint32_t redColor_ = 0xFFFF2F2F;

		static COLORREF toRGB(uint32_t argb) {
			int b = (argb & 0xFF);
			int g = ((argb >> 8) & 0xFF);
			int r = ((argb >> 16) & 0xFF);
			return RGB(r, g, b);
		}

		//percent0-128 正值加深颜色, 负值减淡颜色
		static uint32_t mixColor(uint32_t argb, int percent) {
			int b = (argb & 0xFF);
			int g = ((argb >> 8) & 0xFF);
			int r = ((argb >> 16) & 0xFF);
			int a = ((argb >> 24) & 0xFF);

			if (percent >= 0) {
				b = b - (b)*percent / 128;
				g = g - (g)*percent / 128;
				r = r - (r)*percent / 128;
			}
			else {
				percent = -percent;
				b = b + (0xFF - b) * percent / 128;
				g = g + (0xFF - g) * percent / 128;
				r = r + (0xFF - r) * percent / 128;
			}


			return (a << 24) | (r << 16) | (g << 8) | b;
		}
	};

	class Pen {
	private:
		HPEN pen_;
	public:
		Pen(HPEN p = NULL)
			: pen_(p) {

		}

		explicit Pen(uint32_t cc, int Psize = 1, int style = PS_SOLID)
			: pen_(CreatePen(style, Psize, Color::toRGB(cc))) {

		}
		bool vaild() {
			return pen_ != nullptr;
		}

		~Pen() {
			ReleasePen();
		}


		void ReleasePen() {
			if (pen_)
				::DeleteObject(pen_);
		}

		///更改画笔颜色
		void SetPen(uint32_t cc, int Psize = 1, int style = PS_SOLID) {
			ReleasePen();
			pen_ = ::CreatePen(style, Psize, Color::toRGB(cc));
		}

		inline HPEN GetPen() {
			return pen_;
		}

		static inline HPEN GetWhitePen() {
			return (HPEN) ::GetStockObject(WHITE_PEN);
		}

		static inline HPEN GetBlackPen() {
			return (HPEN) ::GetStockObject(BLACK_PEN);
		}

	private:
		Pen(const Pen&);

		Pen& operator=(const Pen&);
	};

	class Brush {
	private:
		HBRUSH brush_;
	public:
		Brush(HBRUSH bru = NULL)
			: brush_(bru) {

		}

		explicit Brush(uint32_t cc)
			: brush_(CreateSolidBrush(Color::toRGB(cc))) {

		}


		~Brush() {
			ReleaseBrush();
		}

		void ReleaseBrush() {
			if (brush_)
				::DeleteObject(brush_);
		}

		///画刷颜色
		inline void SetBrush(uint32_t cc) {
			ReleaseBrush();
			brush_ = ::CreateSolidBrush(Color::toRGB(cc));
		}

		inline void SetFromBitmap(Bitmap& bmp);

		inline HBRUSH GetBrush() {
			return brush_;
		}

		//透明画刷
		static inline HBRUSH GetNullBrush() {
			return (HBRUSH) ::GetStockObject(NULL_BRUSH);
		}

		static inline HBRUSH GetWhiteBrush() {
			return (HBRUSH) ::GetStockObject(WHITE_BRUSH);
		}

		static inline HBRUSH GetBlackBrush() {
			return (HBRUSH) ::GetStockObject(BLACK_BRUSH);
		}

		static inline HBRUSH GetGrayBrush() {
			return (HBRUSH) ::GetStockObject(GRAY_BRUSH);
		}

	private:
		Brush(const Brush&);

		Brush& operator=(const Brush&);

	};


	struct Hdc {
	protected:
		HDC hdc_;
	public:
		Hdc(HDC dc = NULL)
			: hdc_(dc) {
		}

		HDC GetDc() const {
			return hdc_;
		}
	};

	class Gdi
		: public Hdc {
	public:
		friend Bitmap;
		Gdi(HDC dc = NULL)
			: Hdc(dc) {
		}


		~Gdi(void) {
			ReleaseDc();
		}


		static Gdi& GetScreen() {
			static Gdi screen(::GetDC(0));
			return screen;
		}

		static Gdi& gobalGdi() {
			static Gdi g(::CreateCompatibleDC(::GetDC(0)));
			return g;
		}

		inline void ReleaseDc() {
			if (hdc_) {
				::DeleteDC(hdc_);
				hdc_ = nullptr;
			}

		}

		void Init(Control& cont);

		void Init(Control* cont);

		inline void Init(HWND hwnd) {
			hdc_ = ::GetDC(hwnd);
			//图像缩放算法
			//模糊,坑锯齿
			SetStretchBltMode(hdc_, HALFTONE);
			//普通
			//SetStretchBltMode(hdc_, COLORONCOLOR);
		}

		inline void Init(HDC dc) {
			hdc_ = dc;
		}

		inline HDC GetDc() const {
			return hdc_;
		}

		inline bool SetObject(HGDIOBJ obj) {
			return ::SelectObject(hdc_, obj) != NULL;
		}

		inline bool SetPen(Pen& pen) {
			return ::SelectObject(hdc_, pen.GetPen()) != NULL;
		}

		inline bool SetPen(HPEN pen) {
			return ::SelectObject(hdc_, pen) != NULL;
		}

		inline bool SetBrush(Brush& bru) {
			return ::SelectObject(hdc_, bru.GetBrush()) != NULL;
		}

		inline bool SetBrush(HBRUSH bru) {
			return ::SelectObject(hdc_, bru) != NULL;
		}

		inline bool setFont(const Font& f) {
			return ::SelectObject(hdc_, f.GetFont()) != NULL;
		}

		//获取文字像素总宽
		inline SIZE GetTextPixel(const df::CC& str) {
			SIZE wid;
			GetTextExtentPoint32(hdc_, str.char_, (int)str.length_, &wid);
			return wid;
		}

		///输出文字
		inline BOOL Txt(int x, int y, const df::CC& str) const {
			return ::TextOut(hdc_, x, y, str.char_, (int)str.length_);
		}

		///向RECT中央输出多行
		inline BOOL TxtMutiLine(RECT& rect, const df::CC& str) const {
			UINT format = DT_WORDBREAK;
			return ::DrawText(hdc_, str.char_, (int)str.length_, &rect, format);
		}
		///向RECT中央输出文字单行
		inline BOOL Txt(RECT& rect, const df::CC& str, AlignType alignX = AlignType::center, AlignType alignY = AlignType::center) const {

			UINT format = DT_SINGLELINE;
			if (alignX == AlignType::center)
				format |= DT_CENTER;
			else if (alignX == AlignType::end)
				format |= DT_RIGHT;
			else
				format |= DT_LEFT;

			if (alignY == AlignType::center)
				format |= DT_VCENTER;
			else if (alignY == AlignType::end)
				format |= DT_BOTTOM;
			else
				format |= DT_TOP;

			return ::DrawText(hdc_, str.char_, (int)str.length_, &rect, format);
		}



		inline BOOL Fill(RECT& rect, HBRUSH brush = Brush::GetNullBrush()) {
			return ::FillRect(hdc_, &rect, brush);
		}

		inline void SetTextColor(uint32_t cc) const {
			::SetTextColor(hdc_, Color::toRGB(cc));
		}


		//文字背景色
		inline BOOL SetTextBackColor(uint32_t cc) const {
			return ::SetBkColor(hdc_, Color::toRGB(cc));
		}

		//透明
		inline BOOL SetTextBackColor() const {
			return ::SetBkMode(hdc_, TRANSPARENT);
		}

		///画线
		inline BOOL Line(int sx, int sy, int ex, int ey) const {
			::MoveToEx(hdc_, sx, sy, 0);
			return ::LineTo(hdc_, ex, ey);
		}


		//画矩形
		inline BOOL Rect(int left, int top, int right, int bottom) const {
			return ::Rectangle(hdc_, left, top, right, bottom);
		}

		//画矩形
		static inline BOOL Rect(Hdc dc, int leftx, int topx, int rightx, int boty) {
			return ::Rectangle(dc.GetDc(), leftx, topx, rightx, boty);
		}

		//椭圆
		inline BOOL Round(int left, int top, int right, int bottom) const {
			return ::Ellipse(hdc_, left, top, right, bottom);
		}

		inline BOOL DrawTo(Hdc toGdi, int toX, int toY, int fromW, int fromH, int fromX = 0, int fromY = 0) const {
			return ::BitBlt(toGdi.GetDc(), toX, toY, fromW, fromH, hdc_, fromX, fromY, SRCCOPY);
		}


		inline BOOL DrawFrom(Hdc fromGdi, int toX, int toY, int fromW, int fromH, int fromX = 0, int fromY = 0) const {
			return ::BitBlt(hdc_, toX, toY, fromW, fromH, fromGdi.GetDc(), fromX, fromY, SRCCOPY);
		}

		inline BOOL DrawStretchTo(Hdc toGdi, int toX, int toY, int toW, int toH, int fromW, int fromH, int fromX = 0,
			int fromY = 0) const {
			//  //通过SetStretchBltMode的设置能使StretchBlt在缩放图像更加清晰
			//SetStretchBltMode(hdc, COLORONCOLOR);
			return ::StretchBlt(toGdi.GetDc(), toX, toY, toW, toH, hdc_, fromX, fromY, fromW, fromH, SRCCOPY);
		}

		inline BOOL
			DrawStretchFrom(Hdc fromGdi, int toX, int toY, int toW, int toH, int fromW, int fromH, int fromX = 0,
				int fromY = 0) const {
			//  //通过SetStretchBltMode的设置能使StretchBlt在缩放图像更加清晰
			//SetStretchBltMode(hdc, COLORONCOLOR);
			return ::StretchBlt(hdc_, toX, toY, toW, toH, fromGdi.GetDc(), fromX, fromY, fromW, fromH, SRCCOPY);
		}

		//透明贴图
		inline BOOL
			DrawTransparentTo(Hdc toGdi, int toX, int toY, int toW, int toH, int fromW, int fromH, int fromX, int fromY,
				UINT col) const {
			return ::TransparentBlt(toGdi.GetDc(), toX, toY, toW, toH,
				hdc_, fromX, fromY, fromW, fromH, col);
		}

		//透明贴图
		inline BOOL
			DrawTransparentFrom(Hdc fromGdi, int toX, int toY, int toW, int toH, int fromW, int fromH, int fromX, int fromY,
				UINT col) const {
			return ::TransparentBlt(hdc_, toX, toY, toW, toH,
				fromGdi.GetDc(), fromX, fromY, fromW, fromH, col);
		}


	private:
		DF_DISABLE_COPY_ASSIGN(Gdi)
	};


}

#endif // Gdi_h__2013_8_3__10_07
