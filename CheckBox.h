#pragma once


namespace sdf {

    class CheckBox
        : public Button {
    public:

        bool dot = false;
        bool button = false;

        CheckBox(bool dot = false, bool butto = false) : dot(dot), button(butto) {
           
            style.borderColor = Color::grey4;
            styleDisable.borderColor = Color::grey;
            styleCheck.borderColor = Color::blue;
            styleFocus.borderColor = styleCheck.borderColor;
            styleHover.backColor = Color::blueLight;
            styleHover.borderColor = styleCheck.borderColor;
            stylePress.shadowSize = 3;
            stylePress.borderColor = styleCheck.borderColor;
            stylePress.backColor = styleHover.backColor;
            if (button) {
                pos.padding(8);
                style.color = Color::blue;
                styleCheck.color = style.color;
                styleCheck.backColor = Color::blueLight;
                styleCheck.borderBottom = 3;
            } else {
                style.color = Color::black;
                pos.padding(5);
                pos.textAlignX = AlignType::start;
            }
            styleHover.color = style.color;
            stylePress.color = style.color;

        }

        virtual ~CheckBox() {
            //COUT(tt_("gone"));
        }

        virtual void getContentWH(int32_t &w, int32_t &h) {
            if (button) {
                return Button::getContentWH(w, h);
            }
            h = GlobalFont().GetFontSize() + 4;
            w = Gdi::GetScreen().GetTextPixel(text).cx + h + 4;
        }

        virtual bool onLeftUp() override;

        virtual void onDrawText(RECT &rect, ControlStyle &style, DrawBuffer *draw) override;

    protected:
    };


    class CheckGroup : public View {
    public:
        CheckGroup() {
        }


        //返回选中成员索引
        //没有选中则返回-1
        intptr_t getCheck() {
            intptr_t checkI = 0;
            for (auto &cont : memberList_) {

                CheckBox *check = dynamic_cast <CheckBox *>(cont.get());
                if (check) {
                    if (check->isCheck) {
                        return checkI;
                    }
                    checkI++;
                }
            }
            return -1;
        }

        void setCheck(CheckBox *box) {
            intptr_t checkI = 0;
            for (auto &cont : memberList_) {

                CheckBox *check = dynamic_cast <CheckBox *>(cont.get());
                if (check) {
                    if (check == box) {
                        check->setCheck(true);
                    } else {
                        check->setCheck(false);
                    }
                    checkI++;
                }
            }
        }

        ///设置单选
        void setCheck(intptr_t index) {
            DF_ASSERT(index >= 0);
            DF_ASSERT(index < (intptr_t) memberList_.size());
            intptr_t checkI = 0;
            for (auto &cont : memberList_) {

                CheckBox *check = dynamic_cast <CheckBox *>(cont.get());
                if (check) {
                    if (checkI == index) {
                        check->setCheck(true);
                    } else {
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
#define ui_check_button ui_control2(sdf::CheckBox, false , true)

#define ui_check_box ui_control(sdf::CheckBox) 

