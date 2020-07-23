#pragma once

namespace sdf {
	class LoadAnim
		: public Control {
	protected:
		int16_t startAngle = 0;
		int16_t arcLen = 0;
		int16_t arcInc = 0;

	public:
		float dotSize = 0;
		uint32_t dotColor = 0;
		bool showDot = false;
		bool showAnim = true;
		Timer timer;

		void valInit() {
			startAngle = 0;
			arcLen = 6;
			arcInc = 3;
		}

		LoadAnim() {
			valInit();
		}

		void enableAnim(bool anim) {
			if (anim && !timer) {
				valInit();
				timer = setTimer(animInterval_, [this] { onTimer(); });
			}
			showAnim = anim;
			if (!anim) {
				timer.reset();
			}
		}

		void showContent(bool anim, bool dot) {
			showDot = dot;
			enableAnim(anim);
		}


		virtual ~LoadAnim() {
			//COUT(tt_("LoadAnim gone"));
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

		void onTimer();

	};


}

#define ui_load_anim ui_control(sdf::LoadAnim) 
