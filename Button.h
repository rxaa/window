#ifndef Button_h__2013_8_1__14_27
#define Button_h__2013_8_1__14_27

namespace sdf
{
	class Button
		: public Control
	{
		//绘图回调
	public:
		Gdi buttonGdi_;
		//事件
		std::function<void(Button&)> onCreate_;
		std::function<void()> onClick_;
		//全局绘图缓冲
		static Bitmap buttonBmp_;
		static char* buttonBmpBuf_;
		bool oldStyle = false;
		bool isCheck = false;
		ControlStyle styleCheck;
		Button(Control* parent)
		{
			setColorDark(Color::blueColor_);

			pos.paddingX(10);
			pos.paddingY(6);
			setParent(parent);
		}

		void setColorLight(uint32_t col) {
			style.color = Color::black;
			style.backColor = col;
			styleHover = style;

			styleHover.backColor = Color::mixColor(style.backColor, 10);
			stylePress = styleHover;
			stylePress.shadowSize = 5;
			styleDisable = style;
			styleDisable.color = Color::greyLight;

			styleFocused = style;
			styleCheck = style;
		}

		void setColorDark(uint32_t col) {
			style.color = Color::white;
			style.backColor = col;
			styleHover = style;

			styleHover.backColor = Color::mixColor(style.backColor, 10);
			stylePress = styleHover;
			stylePress.shadowSize = 5;
			styleDisable = style;
			styleDisable.color = Color::darkGrey;
			styleFocused = style;
			styleCheck = style;
		}

		void setCheck(bool Check) {
			isCheck = Check;
			onDraw();
		}


		virtual ~Button() {
			//COUT(tt_("gone"));
		}

		virtual void onHover() {
			onDraw();
		}

		virtual void onLeave() {
			onDraw();
		}

		virtual void getContentWH(int32_t& w, int32_t& h) {
			h = GlobalFont().GetFontSize();
			w = Gdi::GetScreen().GetTextPixel(text);
		}

		static bool drawStyle(Control* cont, ControlStyle& style, const String& text);

		virtual void onDraw();


	protected:
		friend View;

		virtual void initCreate() {
			doCreate(this);
		}
		//初始化位于initCreate之后执行
		virtual void Init();

		bool ControlProc(HWND, UINT, WPARAM, LPARAM) override;

	};



}

#define ui_onclick v.onClick_ = [&]()

#define ui_button sdf::Button * DF_MIX_LINENAME(UIBUTTON, __LINE__)=new sdf::Button(&v);DF_MIX_LINENAME(UIBUTTON, __LINE__)->onCreate_=[&](sdf::Button &v)



#endif // Button_h__2013_8_1__14_27
