#ifndef TextBox_h__2013_8_1__16_39
#define TextBox_h__2013_8_1__16_39


namespace sdf
{
	class TextBox
		: public Control
	{
	public:

		bool mutiLine = false;
		bool onlyNumber = false;
		bool password = false;
		bool readOnly = false;
		bool focused = false;
		bool showVScroll = false;
	

		ControlStyle styleFocus;

		std::function<void()> onChange_;
		std::function<void()> onFocus_;
		std::function<void()> onLeave_;

		TextBox() {

			pos.w = Control::GlobalFont().getRawSize() * 5;
			pos.h = Control::GlobalFont().getRawSize() + 5;
			style.border(1);
			styleFocus.border(1);
			styleFocus.borderColor = Color::darkBlue;
			style.borderColor = Color::blueBorder;
			pos.padding(1);
		}



		virtual ~TextBox() {
			//COUT(tt_("gone"));
		}

	
		virtual void onMeasure() override;
		virtual void onMouseMove(int32_t x, int32_t y) override;

		virtual void onDraw();

		virtual void doCreate() override {
			Control::doCreate();
			scaleStyle(styleFocus);
		}
	protected:
		Gdi gdi_;
		virtual void Init();

		virtual bool ControlProc(HWND, UINT, WPARAM, LPARAM, LRESULT& ret) override;
	};

}


#define ui_onchange v.onChange_ = [=,&v=v]()

#define ui_text_box ui_control(sdf::TextBox) 


#endif // TextBox_h__2013_8_1__16_39
