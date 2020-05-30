#ifndef gdiP_h__2013_9_5__16_57
#define gdiP_h__2013_9_5__16_57

#include <gdiplus.h>

namespace sdf
{
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
