#ifndef TextBox_h__2013_8_1__16_39
#define TextBox_h__2013_8_1__16_39


namespace sdf
{
	class TextBox
		: public Control
	{
	public:
		//Gdi gdi_;
		//多行显示
		bool mutiLine = false;
		bool onlyNumber = false;
		bool password = false;
		bool readOnly = false;

		std::function<void(TextBox&)> onCreate_;

		std::function<void()> onChange_;
		std::function<void()> onFocus_;
		std::function<void()> onLeave_;

		TextBox(Control* parent) {
			setParent(parent);
		}

		virtual ~TextBox() {
			//COUT(tt_("gone"));
		}

		virtual void onDraw() {
			//COUT(tt_("重绘TextBox"));
			update();
		}

	protected:
		///初始化
		virtual void initCreate();
		virtual void Init();
		

		virtual bool ControlProc(HWND, UINT, WPARAM, LPARAM) override;
	};

}


#define ui_onchange v.onChange_ = [&]()

#define ui_text_box ui_control(sdf::TextBox) 


#endif // TextBox_h__2013_8_1__16_39
