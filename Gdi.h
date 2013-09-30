#ifndef Gdi_h__2013_8_3__10_07
#define Gdi_h__2013_8_3__10_07



namespace sdf
{
	class Bitmap;


	struct Color
	{
		enum{
			//windows��
			grey = 0xF0F0F0,
			//��
			black = 0x000000,
			//���
			darkGrey = 0xA0A0A0,
			//��ɫ
			red = 0x2840ff,
			//��ɫ
			purple = 0xFF00FF,
			//��ɫ
			blue = 0xFF9F00,
			//����
			barkBlue = 0xFF0000,
			//��ɫ
			white = 0xFFFFFF
		};

		static COLORREF AddColor(COLORREF rgb, int value)
		{
			int r = (rgb & 0xFF) * value / 128;
			int g = ((rgb >> 8) & 0xFF) * value / 128;
			int b = ((rgb >> 16) & 0xFF) * value / 128;
			return RGB(r, g, b);
		}
	};

	class Pen
	{
	private:
		HPEN pen_;
	public:
		Pen(HPEN p = NULL)
			: pen_(p)
		{

		}

		explicit Pen(COLORREF cc, int Psize = 1, int style = PS_SOLID)
			: pen_(CreatePen(style, Psize, cc))
		{

		}

		~Pen()
		{
			ReleasePen();
		}


		void ReleasePen()
		{
			if (pen_)
				::DeleteObject(pen_);
		}

		///���Ļ�����ɫ
		void SetPen(COLORREF cc, int Psize = 1, int style = PS_SOLID)
		{
			ReleasePen();
			pen_ = ::CreatePen(style, Psize, cc);
		}

		inline HPEN GetPen()
		{
			return pen_;
		}

		static inline HPEN GetWhitePen()
		{
			return (HPEN)::GetStockObject(WHITE_PEN);
		}

		static inline HPEN GetBlackPen()
		{
			return (HPEN)::GetStockObject(BLACK_PEN);
		}
	private:
		Pen(const Pen &);
		Pen& operator=(const Pen &);
	};

	class Brush
	{
	private:
		HBRUSH brush_;
	public:
		Brush(HBRUSH bru = NULL)
			: brush_(bru)
		{

		}
		explicit Brush(COLORREF cc)
			: brush_(CreateSolidBrush(cc))
		{

		}


		~Brush()
		{
			ReleaseBrush();
		}

		void ReleaseBrush()
		{
			if (brush_)
				::DeleteObject(brush_);
		}

		///��ˢ��ɫ
		inline void SetBrush(COLORREF cc)
		{
			ReleaseBrush();
			brush_ = ::CreateSolidBrush(cc);
		}

		inline void SetFromBitmap(Bitmap & bmp);

		inline HBRUSH GetBrush()
		{
			return brush_;
		}

		//͸����ˢ
		static inline HBRUSH GetNullBrush()
		{
			return (HBRUSH)::GetStockObject(NULL_BRUSH);
		}

		static inline HBRUSH GetWhiteBrush()
		{
			return (HBRUSH)::GetStockObject(WHITE_BRUSH);
		}

		static inline HBRUSH GetBlackBrush()
		{
			return (HBRUSH)::GetStockObject(BLACK_BRUSH);
		}
		static inline HBRUSH GetGrayBrush()
		{
			return (HBRUSH)::GetStockObject(GRAY_BRUSH);
		}

	private:
		Brush(const Brush &);
		Brush& operator=(const Brush &);

	};


	struct Hdc
	{
	protected:
		HDC hdc_;
	public:
		Hdc(HDC dc = NULL)
			: hdc_(dc)
		{
		}
		HDC GetDc() const
		{
			return hdc_;
		}
	};

	class Gdi
		: public Hdc
	{
	public:
		Gdi(HDC dc = NULL)
			: Hdc(dc)
		{
		}


		~Gdi(void)
		{
			ReleaseDc();
		}

		static Gdi & GetScreen()
		{
			static Gdi screen(::GetDC(0));
			return screen;
		}

		inline void ReleaseDc() const
		{
			if (hdc_)
				::DeleteDC(hdc_);
		}

		inline void Init(Control & cont)
		{
			hdc_ = ::GetDC(cont.GetHandle());
		}

		inline void Init(Control * cont)
		{
			MY_ASSERT(cont != nullptr);
			hdc_ = ::GetDC(cont->GetHandle());
		}

		inline void Init(HWND hwnd)
		{
			hdc_ = ::GetDC(hwnd);
		}

		inline void Init(HDC dc)
		{
			hdc_ = dc;
		}

		inline HDC GetDc() const
		{
			return hdc_;
		}

		inline bool SetObject(HGDIOBJ obj)
		{
			return ::SelectObject(hdc_, obj) != NULL;
		}

		inline bool SetPen(Pen & pen)
		{
			return ::SelectObject(hdc_, pen.GetPen()) != NULL;
		}

		inline bool SetPen(HPEN pen)
		{
			return ::SelectObject(hdc_, pen) != NULL;
		}

		inline bool SetBrush(Brush & bru)
		{
			return ::SelectObject(hdc_, bru.GetBrush()) != NULL;
		}

		inline bool SetBrush(HBRUSH bru)
		{
			return ::SelectObject(hdc_, bru) != NULL;
		}

		inline uint GetTextPixel(const CC & str)
		{
			SIZE wid;
			GetTextExtentPoint32(hdc_, str.char_, str.length_, &wid);
			return wid.cx;
		}

		///�������
		inline BOOL Txt(int x, int y, const CC & str) const
		{
			return ::TextOut(hdc_, x, y, str.char_, str.length_);
		}

		///��RECT����������ֵ���
		inline BOOL Txt(RECT & rect, const CC & str) const
		{
			return ::DrawText(hdc_, str.char_, str.length_, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		}

		inline BOOL Fill(RECT & rect, HBRUSH brush = Brush::GetNullBrush())
		{
			return ::FillRect(hdc_, &rect, brush);
		}

		inline void SetTextColor(COLORREF cc) const
		{
			::SetTextColor(hdc_, cc);
		}


		//���ֱ���ɫ
		inline BOOL SetTextBackColor(COLORREF cc) const
		{
			return ::SetBkColor(hdc_, cc);
		}
		//͸��
		inline BOOL SetTextBackColor() const
		{
			return  ::SetBkMode(hdc_, TRANSPARENT);
		}

		///����
		inline BOOL Line(int sx, int sy, int ex, int ey) const
		{
			::MoveToEx(hdc_, sx, sy, 0);
			return ::LineTo(hdc_, ex, ey);
		}


		//������
		inline BOOL Rect(int left, int top, int right, int bottom) const
		{
			return ::Rectangle(hdc_, left, top, right, bottom);
		}

		//������
		static inline BOOL Rect(Hdc dc, int leftx, int topx, int rightx, int boty)
		{
			return ::Rectangle(dc.GetDc(), leftx, topx, rightx, boty);
		}

		//��Բ
		inline BOOL Round(int left, int top, int right, int bottom) const
		{
			return ::Ellipse(hdc_, left, top, right, bottom);
		}

		inline BOOL DrawTo(Hdc toGdi, int toX, int toY, int fromW, int fromH, int fromX = 0, int fromY = 0) const
		{
			return ::BitBlt(toGdi.GetDc(), toX, toY, fromW, fromH, hdc_, fromX, fromY, SRCCOPY);
		}

		inline BOOL DrawFrom(Hdc fromGdi, int toX, int toY, int fromW, int fromH, int fromX = 0, int fromY = 0) const
		{
			return ::BitBlt(hdc_, toX, toY, fromW, fromH, fromGdi.GetDc(), fromX, fromY, SRCCOPY);
		}

		inline BOOL DrawStretchTo(Hdc toGdi, int toX, int toY, int toW, int toH, int fromW, int fromH, int fromX = 0, int fromY = 0) const
		{
			//  //ͨ��SetStretchBltMode��������ʹStretchBlt������ͼ���������
			//SetStretchBltMode(hdc, COLORONCOLOR);
			return  ::StretchBlt(toGdi.GetDc(), toX, toY, toW, toH, hdc_, fromX, fromY, fromW, fromH, SRCCOPY);
		}

		inline BOOL DrawStretchFrom(Hdc fromGdi, int toX, int toY, int toW, int toH, int fromW, int fromH, int fromX = 0, int fromY = 0) const
		{
			//  //ͨ��SetStretchBltMode��������ʹStretchBlt������ͼ���������
			//SetStretchBltMode(hdc, COLORONCOLOR);
			return  ::StretchBlt(hdc_, toX, toY, toW, toH, fromGdi.GetDc(), fromX, fromY, fromW, fromH, SRCCOPY);
		}

		//͸����ͼ
		inline BOOL DrawTransparentTo(Hdc toGdi, int toX, int toY, int toW, int toH, int fromW, int fromH, int fromX, int fromY, uint col) const
		{
			return ::TransparentBlt(toGdi.GetDc(), toX, toY, toW, toH,
				hdc_, fromX, fromY, fromW, fromH, col);
		}

		//͸����ͼ
		inline BOOL DrawTransparentFrom(Hdc fromGdi, int toX, int toY, int toW, int toH, int fromW, int fromH, int fromX, int fromY, uint col) const
		{
			return ::TransparentBlt(hdc_, toX, toY, toW, toH,
				fromGdi.GetDc(), fromX, fromY, fromW, fromH, col);
		}



	private:
		Gdi(const Gdi & gdi);
		Gdi& operator=(const Gdi & gdi);
	};



}

#endif // Gdi_h__2013_8_3__10_07
