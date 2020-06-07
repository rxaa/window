#pragma once

namespace sdf
{
	class Button
		: public Control
	{
	protected:

	public:
		Gdi buttonGdi_;
		//事件
		std::function<void()> onClick_;
		Button& v{ *this };

		bool oldStyle = false;
		bool isCheck = false;
		ControlStyle styleCheck;
		Button()
		{
			pos.paddingX(10);
			pos.paddingY(6);
		}

		static void setMenuStyle(sdf::Button*  but) {
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
			but->styleDisable.color = Color::darkGrey;

			but->styleCheck = but->style;
			but->styleFocused = but->style;
		}

		void setBackColor(int32_t color) {
			style.backColor = color;
			stylePress.backColor = color;
			styleHover.backColor = color;
			styleDisable.backColor = color;
			styleCheck.backColor = color;
			styleFocused.backColor = color;
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

		virtual void setCheck(bool Check) {
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
			w = Gdi::GetScreen().GetTextPixel(text);
			if (w > 0)
				h = GlobalFont().GetFontSize();
		}



		virtual void onDraw();


	protected:
		friend View;

		//初始化位于initCreate之后执行
		virtual void Init();

		bool ControlProc(HWND, UINT, WPARAM, LPARAM) override;

	};


	typedef  std::shared_ptr<sdf::Button> PtrButton;
}



#define ui_onclick v.onClick_ = [&]()

#define ui_button ui_control(sdf::Button)


