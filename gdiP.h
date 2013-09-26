#ifndef gdiP_h__2013_9_5__16_57
#define gdiP_h__2013_9_5__16_57

#include <gdiplus.h>

namespace sdf
{
	class gdiP
	{
	public:
		static void Init()
		{
			Gdiplus::GdiplusStartup(&gdiplusToken_, &gdiplusStartupInput_, NULL);
			atexit(Shutdown);
		}

		static void Shutdown()
		{
			Gdiplus::GdiplusShutdown(gdiplusToken_);
		}

	private:
		static Gdiplus::GdiplusStartupInput gdiplusStartupInput_;
		static ULONG_PTR gdiplusToken_;
	};

	STATIC_INIT ULONG_PTR gdiP::gdiplusToken_=0;

	STATIC_INIT Gdiplus::GdiplusStartupInput gdiP::gdiplusStartupInput_=0;

}


#endif // gdiP_h__2013_9_5__16_57
