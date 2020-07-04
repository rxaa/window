#pragma once

namespace sdf {
	class LoadAnim
		: public Control {
	protected:
		uint32_t timerId = 0;
		int16_t startAngle = 0;
		int16_t arcLen = 6;
		int16_t arcInc = 3;

	public:
		Gdi gdi_;
		float dotSize = 0;
		uint32_t dotColor = 0;
		bool showDot = false;
		bool showAnim = true;

		LoadAnim() {

		}

		void enableAnim(bool anim){
            showAnim = anim;
            if (showAnim) {
                setTimer(timerId, animInterval_);
            }
            else {
                killTimer(timerId);
            }
		}

		void showContent(bool anim, bool dot) {
			showDot = dot;
			enableAnim(anim);
		}
		 

		virtual ~LoadAnim() {
			//COUT(tt_("gone"));
			if (timerId > 0) {
				killTimer(timerId);
			}
		}


		virtual void getContentWH(int32_t& w, int32_t& h) {
			w = Gdi::GetScreen().GetTextPixel(text).cx;
			if (w > 0) {
				h = GlobalFont().GetFontSize();
			}
		}

		virtual void onDraw();
		virtual void doCreate() override;

	protected:
		///初始化
		virtual void Init();

		virtual void onTimer(uint32_t) override;

		//virtual bool ControlProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& ret) override;

	};


}

#define ui_load_anim ui_control(sdf::LoadAnim) 
