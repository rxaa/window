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
			styleHover.borderColor = Color::blue;
		}

		virtual ~CheckBox() {
			//COUT(tt_("gone"));
		}
		virtual void getContentWH(int32_t& w, int32_t& h) {
			h = GlobalFont().GetFontSize() + 4;
			w = Gdi::GetScreen().GetTextPixel(text) + h + 3;
		}

		virtual void onDrawText(RECT& rect);

	protected:

		virtual bool ControlProc(HWND, UINT, WPARAM, LPARAM) override;
	};


	class CheckGroup :public View {
	public:
		CheckGroup() {
		}


		//返回选中成员索引
		//没有选中则返回-1
		int getCheck() {
			int checkI = 0;
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
			int checkI = 0;
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
		void setCheck(int index)
		{
			DF_ASSERT(index >= 0);
			DF_ASSERT(index < memberList_.size());
			int checkI = 0;
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

