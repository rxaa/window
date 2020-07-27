#pragma once

namespace sdf
{
	class Button
		: public Control
	{
	protected:
	public:
		//事件
		std::function<void()> onClick_;
		Button& v{ *this };

		bool isCheck = false;
		ControlStyle styleCheck;
		ControlStyle styleFocus;
		Button()
		{
			hasCursor = true;
			pos.paddingX(10);
			pos.paddingY(6);
		}

		void fontSize(uint32_t size) {
			Control::fontSize(size);
			styleCheck.font.size = size;
			styleFocus.font.size = size;
		}

		void fontBold() {
			if (style.font.size == 0) {
				fontSize(Font::initSize);
			}
			Control::fontBold();
			styleCheck.font.bold = true;
			styleFocus.font.bold = true;
		}

		static void setMenuStyle(sdf::Button* but) {
			but->pos.paddingX(12);
			but->pos.paddingY(7);
			but->style.backColor = Color::white;
			but->style.color = Color::black;
			but->styleHover.backColor = Color::blueLight;
			//but->styleHover.border(1);
			//but->styleHover.borderColor = Color::blue;

			but->stylePress = but->styleHover;
			but->stylePress.shadowSize = 3;

			but->styleDisable = but->style;
			but->styleDisable.color = Color::grey4;

			but->styleCheck = but->style;
			but->styleFocus = but->style;
		}

		void setBackColor(int32_t color) {
			style.backColor = color;
			stylePress.backColor = color;
			styleHover.backColor = color;
			styleDisable.backColor = color;
			styleCheck.backColor = color;
			styleFocus.backColor = color;
		}

		void setColorLight(uint32_t col) {
			style.color = Color::black;
			style.backColor = col;
			styleHover = style;

			styleHover.backColor = Color::mixColor(style.backColor, 10);
			stylePress = styleHover;
			stylePress.shadowSize = 5;
			styleDisable = style;
			styleDisable.color = Color::grey5;

			styleFocus = style;
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
			styleDisable.color = Color::grey4;
			styleFocus = style;
			styleCheck = style;
		}

		virtual void setCheck(bool Check) {
			isCheck = Check;
			onDraw();
		}


		virtual ~Button() {
			//COUT(tt_("gone"));
		}

		virtual bool onLeftDown() override {
			if (!isEnable) {
				return false;
			}

			isPress = true;
			onDraw();
			return false;
		}

		virtual bool onLeftUp() override {
		
			if (isPress) {
				isPress = false;
				onDraw();
				if (onClick_)
					onClick_();
			}
			return false;
		}

		virtual void onHover()  override {
			if (!isEnable) {
				return;
			}
			auto par = getTopParent();
			if (par) {
				par->hasCursor = true;
			}

			SetCursor(LoadCursor(NULL, IDC_HAND));
			onDraw();
		}

		virtual void onLeave() override {
			if (!isEnable) {
				return;
			}
			//SetCursor(LoadCursor(NULL, IDC_ARROW));
			auto par = getTopParent();
			if (par) {
				par->hasCursor = false;
			}
			isPress = false;
			onDraw();
		}

		virtual void getContentWH(int32_t& w, int32_t& h)  override {
			w = Gdi::GetScreen().GetTextPixel(text).cx;
			if (w > 0)
				h = GlobalFont().GetFontSize();
		}


		virtual void onDraw() override;

		virtual void doCreate() override {
			Control::doCreate();
			scaleStyle(styleCheck);
			scaleStyle(styleFocus);
		}

	protected:
		friend View;

		//初始化位于initCreate之后执行
		virtual void Init();

	};


	typedef  std::shared_ptr<sdf::Button> PtrButton;
}



#define ui_onclick v.onClick_ = [=,&v=v]()

#define ui_button ui_control(sdf::Button)


