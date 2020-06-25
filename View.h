﻿#ifndef WINDOWSTAP_VIEW_H
#define WINDOWSTAP_VIEW_H

namespace sdf {
	class View
		: public Control {
	public:
		Gdi gdi_;
		View& v;

		View() :v(*this) {
		}


		virtual ~View() {
			//COUT(tt_("gone"));
		}


		virtual void getContentWH(int32_t& w, int32_t& h)override {
			if (text.size() == 0) {
				w = 0;
				h = 0;
				return;
			}
			if (gdi_.GetDc()) {
				auto size = gdi_.GetTextPixel(text);
				w = size.cx;
				h = size.cy;
			}
			else {
				Gdi::gobalGdi().setFont(getFont());
				auto size = Gdi::gobalGdi().GetTextPixel(text);
				w = size.cx;
				h = size.cy;
			}

		}
		virtual void reGetContentWH(int32_t& w, int32_t& h)override {

			int32_t showW = pos.w - pos.paddingLeft - pos.paddingRight;

			if (!pos.wrapX && pos.wrapY && pos.textMutiline && text.size() > 0 && showW > 0) {

				auto dc = gdi_.GetDc() ? gdi_.GetDc() : Gdi::gobalGdi().GetDc();
				RECT r1 = { 0, 0, showW, h };
				DrawText(dc, text.c_str(), text.length(), &r1,
					DT_CALCRECT | DT_WORDBREAK
				);

				h = r1.bottom;

			}
		}

		virtual void onDraw() override;


	protected:
		///初始化
		virtual void Init() override;
		virtual bool ControlProc(HWND, UINT, WPARAM, LPARAM, LRESULT& ret) override {
			return true;
		}

	};

	typedef  std::shared_ptr<sdf::View> PtrView;
}

#define ui_view ui_control(sdf::View) 


#endif //WINDOWSTAP_VIEW_H
