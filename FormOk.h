#pragma once

namespace sdf {

	class FormOk : public sdf::Window {
	protected:
		std::function<void()> onOkClick_;
		String content;
	public:

		FormOk(df::CC t) {
			content = t.ToString();
		}
		FormOk(df::CC t,df::CC title) {
			text = title.ToString();
			content = t.ToString();
		}

		FormOk(const std::function<void()>& onClick) {
			onOkClick_ = onClick;
		}

		~FormOk() {
		}


		template < class ...Args>
		static std::shared_ptr<FormOk> create(Args&& ... rest) {
			return std::make_shared<FormOk>(rest...);
		}

		static void buttonTrans(sdf::Button& but, uint32_t color) {

			but.pos.paddingX(20);
			but.pos.paddingY(10);
			but.style.backColor = 0;
			but.style.color = color;
			but.fontBold();
			but.fontSize(17);

			but.styleHover = but.style;
			but.styleHover.color = Color::blue;
			but.styleHover.backColor = Color::blueLight;
			//but->styleHover.border(1);	
			//but->styleHover.borderColor = Color::blue;

			but.stylePress = but.styleHover;
			but.stylePress.shadowSize = 3;

			but.styleDisable = but.style;
			but.styleDisable.color = Color::darkGrey;

			but.styleCheck = but.style;
			but.styleFocused = but.style;
		}

		virtual void onCreate() override;

		virtual void onInit() override;

	};
}
