#ifndef Font_h__
#define Font_h__

namespace sdf
{
	using namespace df::sdf;
	using df::String;

	class Window;

	struct WinHandle;

	class Bitmap;
	class Control;

	class Font
	{
	public:

		static const uint32_t initSize = 16;
		Font(long size = initSize, df::CC name = tcc_("新宋体"));

		~Font()
		{
			DeleteObject(font_);
		}

		void SetFont(const Font& f) {
			font_ = f.font_;
			logFont_ = f.logFont_;
		}

		uint32_t getRawSize() {
			return rawSize;
		}
		

		HFONT SetFont(df::CC name, long size = initSize);


		template<class LamT>
		static void EnumFont(HDC dc, LamT lam)
		{
			LOGFONT lf = { 0 };
			lf.lfCharSet = GB2312_CHARSET;
			EnumFontFamiliesEx(dc, &lf, EnumFontFamExProc<LamT>, (LPARAM)&lam, 0);
		}

		HFONT GetFont() const
		{
			return font_;
		}

		int GetFontSize()
		{
			return logFont_.lfHeight;
		}

	private:

		template<class LamT>
		static int CALLBACK EnumFontFamExProc(const LOGFONT* lpelfe, const TEXTMETRIC*, DWORD FontType, LPARAM lParam)
		{
			if (FontType & TRUETYPE_FONTTYPE && lpelfe->lfFaceName[0] != tt_('@'))
			{
				(*(LamT*)lParam)(lpelfe->lfFaceName);
				//COUT(lpelfe->lfFaceName);
			}
			return 1;
		}
	private:
		uint32_t rawSize = 0;
		HFONT font_;
		LOGFONT logFont_;
	};
}

#endif // Font_h__
