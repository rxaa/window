#ifndef Font_h__
#define Font_h__

class FontType {
public:
	uint32_t size = 0;
	uint16_t font = 0;
	bool bold = false;
	bool italic = false;
	bool underLine = false;
	bool strikeOut = false;

	bool hasFont() {
		return size > 0;
	}

	static std::array<df::CC, 3> getFontName() {
		return { tt_("新宋体"),tt_("Microsoft Yahei"),tt_("黑体") };
	}

	size_t hash() const {
		return (size << 16) ^ (font << 8) ^ (bold << 7) ^ (italic << 6) ^ (underLine << 5) ^ (strikeOut << 4);
	}

	bool operator==(const FontType& a) const {
		return size == a.size && font == a.font && bold == a.bold
			&& italic == a.italic && underLine == a.underLine && strikeOut == a.strikeOut;
	}
};




namespace std {
	template<>
	struct hash<FontType> {
		size_t operator()(const FontType& key) const {
			return key.hash();
		}
	};
}

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
		DF_DISABLE_COPY_ASSIGN(Font)
	public:
		static std::unordered_map< FontType, Font> cache_;

		static const uint32_t initSize = 16;
		Font(long size = initSize, df::CC name = tcc_("新宋体"));

		static Font& getFont(const FontType& f) {
			auto it = cache_.find(f);
			if (it != cache_.end()) {
				return it->second;
			}
			else {
				auto res = cache_.emplace(f, f);
				return res.first->second;
			}


		}

		Font(const FontType& type);

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
