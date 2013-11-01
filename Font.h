#ifndef Font_h__
#define Font_h__

namespace sdf
{
	class Font
	{
	public:
		Font(long size = 20, CC name = cct_("宋体"))
		{
			memset(&logFont_, 0, sizeof(LOGFONT));
			if (name.length_ > 31)
				name.length_ = 31;
			memcpy(logFont_.lfFaceName, name.char_, name.length_*sizeof(TCHAR));
			logFont_.lfHeight = size;
			font_ = CreateFontIndirect(&logFont_);
		}
		~Font()
		{
			DeleteObject(font_);
		}

		HFONT SetFont(long size = 20, CC name = cct_("宋体"))
		{
			memset(&logFont_, 0, sizeof(LOGFONT));
			
			if (name.length_ > 31)
				name.length_ = 31;
			memcpy(logFont_.lfFaceName, name.char_, name.length_*sizeof(TCHAR));

			logFont_.lfHeight = size;
			DeleteObject(font_);
			font_ = CreateFontIndirect(&logFont_);
			return font_;
		}

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
		static int CALLBACK EnumFontFamExProc(const LOGFONT *lpelfe, const TEXTMETRIC *, DWORD FontType, LPARAM lParam)
		{
			if (FontType & TRUETYPE_FONTTYPE && lpelfe->lfFaceName[0] != t_t('@'))
			{
				(*(LamT*) lParam)(lpelfe->lfFaceName);
				//COUT(lpelfe->lfFaceName);
			}
			return 1;
		}
	private:
		HFONT font_;
		LOGFONT logFont_;
	};
}

#endif // Font_h__
