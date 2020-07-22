#ifndef TextBox_h__2013_8_1__16_39
#define TextBox_h__2013_8_1__16_39


namespace sdf
{
	class TextBox
		: public Control
	{
	public:

		//多行显示
		bool mutiLine = false;
		bool onlyNumber = false;
		bool password = false;
		bool readOnly = false;

		std::function<void()> onChange_;
		std::function<void()> onFocus_;
		std::function<void()> onLeave_;

		TextBox() {

			pos.w = Control::GlobalFont().getRawSize() * 5;
			pos.h = Control::GlobalFont().getRawSize() + 5;
		}

		virtual ~TextBox() {
			//COUT(tt_("gone"));
		}

		virtual void onDraw() {
			//COUT(tt_("重绘TextBox"));
			updateHandleXy();
			DrawBuffer* draw = getDraw();
			update();
			if (pos.w > 0 && pos.h > 0 && !mutiLine) {
				gdi_.DrawTo(draw->buttonBmp_, getDrawX(), getDrawY(), pos.w, pos.h);
			}

		}

	protected:
		Gdi gdi_;
		///初始化
		virtual void Init();


		virtual bool ControlProc(HWND, UINT, WPARAM, LPARAM, LRESULT& ret) override;
	};

}


#define ui_onchange v.onChange_ = [&]()

#define ui_text_box ui_control(sdf::TextBox) 


#endif // TextBox_h__2013_8_1__16_39
