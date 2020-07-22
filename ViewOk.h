#pragma once

namespace sdf {

    class ViewOk : public sdf::View {
    protected:

    public:
        std::function<void()> onOk;
        std::function<void()> onCancel;

        ViewOk() {

            v.style.backColor = Color::yellowLight;
            v.pos.flexX = 1;

            ui_view
                {
                    v.pos.flexX = 2;
                };
            ui_button
            {
                v.pos.flexX = 1;
                buttonTrans(v, Color::green);
                v.text = df::lang().ok;
                v.onClick_ = onOk;

            };
            ui_button
                {
                    v.pos.flexX = 1;
                    buttonTrans(v, Color::red);
                    v.text = df::lang().cancel;
                    v.onClick_ = onCancel;

                };
           
        }

        ~ViewOk() {

        }

        static void buttonTrans(sdf::Button &but, uint32_t color) {

            but.pos.paddingX(20);
            but.pos.paddingY(10);
            but.style.backColor = 0;
            but.style.color = color;
            but.fontBold();
            but.fontSize(17);

            but.styleHover = but.style;
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
    };
}

#define ui_ok_view ui_control(sdf::ViewOk)