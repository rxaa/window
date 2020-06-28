#ifndef gdiP_h__2013_9_5__16_57
#define gdiP_h__2013_9_5__16_57

#include <gdiplus.h>

namespace sdf
{


	class CGraphicsRoundRectPath : public Gdiplus::GraphicsPath
	{

	public:
		CGraphicsRoundRectPath():Gdiplus::GraphicsPath() {

		}
		CGraphicsRoundRectPath(int32_t x, int32_t y, int32_t width, int32_t height, int32_t cornerX, int32_t cornerY) :Gdiplus::GraphicsPath()
		{
			AddRoundRect(x, y, width, height, cornerX, cornerY);
		}

	public:
		void AddRoundRect(int32_t x, int32_t y, int32_t width, int32_t height, int32_t cornerX, int32_t cornerY) {
			int32_t elWid = 2 * cornerX;
			int32_t elHei = 2 * cornerY;

			AddArc(x, y, elWid, elHei, 180, 90); // 左上角圆弧
			AddLine(x + cornerX, y, x + width - cornerX, y); // 上边

			AddArc(x + width - elWid, y, elWid, elHei, 270, 90); // 右上角圆弧
			AddLine(x + width, y + cornerY, x + width, y + height - cornerY);// 右边

			AddArc(x + width - elWid, y + height - elHei, elWid, elHei, 0, 90); // 右下角圆弧
			AddLine(x + width - cornerX, y + height, x + cornerX, y + height); // 下边

			AddArc(x, y + height - elHei, elWid, elHei, 90, 90);
			AddLine(x, y + cornerY, x, y + height - cornerY);

		}
	};
	class Gdip
	{
	public:
		static void Init()
		{
			if (gdiplusToken_ == 0) {
				Gdiplus::GdiplusStartup(&gdiplusToken_, &gdiplusStartupInput_, NULL);
				atexit(Shutdown);
			}
		}

		static void Shutdown()
		{
			Gdiplus::GdiplusShutdown(gdiplusToken_);
		}


	private:
		static Gdiplus::GdiplusStartupInput gdiplusStartupInput_;
		static ULONG_PTR gdiplusToken_;
	};
}


#endif // gdiP_h__2013_9_5__16_57
