#pragma once


namespace sdf
{
	class CheckBox
		: public Control
	{
	public:
		Gdi gdi_;
		//多行显示

		std::function<void()> onClick_;
		std::function<void(CheckBox&)> onCreate_;
		bool check_ = false;

		CheckBox(Control* parent) {
			setParent(parent);
		}

		virtual ~CheckBox() {
			//COUT(tt_("gone"));
		}

		virtual void onDraw() {
			//COUT(tt_("重绘TextBox"));
			//update();
		}

	protected:
		///初始化
		virtual void initCreate();
		virtual void Init();
		

		virtual bool ControlProc(HWND, UINT, WPARAM, LPARAM) override;
	};

}


#define ui_check_box sdf::CheckBox * DF_MIX_LINENAME(UIBUTTON, __LINE__)=new sdf::CheckBox(&v);DF_MIX_LINENAME(UIBUTTON, __LINE__)->onCreate_=[&](sdf::CheckBox &v)

