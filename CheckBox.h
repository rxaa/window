#pragma once


namespace sdf
{

	class CheckBox
		: public Button
	{
	public:

		bool dot = false;

		CheckBox(bool dot = false) :dot(dot) {
			pos.padding(5);
			style.borderColor = Color::greyMid;
			styleDisable.borderColor = Color::grey;
			styleCheck.borderColor = Color::blue;
			styleFocused.borderColor = Color::blue;
			styleHover.backColor = Color::blueLight;
			styleHover.borderColor = Color::blue;
			stylePress.shadowSize = 3;
			stylePress.borderColor = Color::blue;
			stylePress.backColor = Color::blueLight;
			pos.textAlignX = AlignType::start;
		}

		virtual ~CheckBox() {
			//COUT(tt_("gone"));
		}
		virtual void getContentWH(int32_t& w, int32_t& h) {
			h = GlobalFont().GetFontSize() + 4;
			w = Gdi::GetScreen().GetTextPixel(text).cx + h + 4;
		}

		virtual void onDrawText(RECT& rect, ControlStyle& style, DrawBuffer* draw) override;

	protected:

		virtual bool ControlProc(HWND, UINT, WPARAM, LPARAM, LRESULT& ret) override;
	};


	class CheckGroup :public View {
	public:
		CheckGroup() {
		}


		//返回选中成员索引
		//没有选中则返回-1
		intptr_t getCheck() {
			intptr_t checkI = 0;
			for (auto& cont : memberList_) {

				CheckBox* check = dynamic_cast <CheckBox*>(cont.get());
				if (check) {
                    if (check->isCheck) {
						return checkI;
					}
					checkI++;
				}
			}
			return -1;
		}

		void setCheck(CheckBox* box)
		{
			intptr_t checkI = 0;
			for (auto& cont : memberList_) {

				CheckBox* check = dynamic_cast <CheckBox*>(cont.get());
				if (check) {
					if (check == box) {
						check->setCheck(true);
					}
					else {
						check->setCheck(false);
					}
					checkI++;
				}
			}
		}

		///设置单选
		void setCheck(intptr_t index)
		{
			DF_ASSERT(index >= 0);
			DF_ASSERT(index < (intptr_t)memberList_.size());
			intptr_t checkI = 0;
			for (auto& cont : memberList_) {

				CheckBox* check = dynamic_cast <CheckBox*>(cont.get());
				if (check) {
					if (checkI == index) {
						check->setCheck(true);
					}
					else {
						check->setCheck(false);
					}
					checkI++;
				}
			}
		}
	};

}


#define ui_check_group ui_control(sdf::CheckGroup) 

#define ui_check_dot ui_control2(sdf::CheckBox, true) 


#define ui_check_box ui_control(sdf::CheckBox) 

