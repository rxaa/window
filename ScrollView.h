﻿#pragma once


namespace sdf
{

	class ScrollView : public Control {
	protected:
		Gdi gdi_;

		int vertMax = 0;
		int vertPage = 0;
		int horiMax = 0;
		int horiPage = 0;
		int32_t vertPos = 0;
		int32_t vertRemain = 0;
		int32_t horiPos = 0;
		int32_t horiRemain = 0;
	public:

		ScrollView() {
			pos.vector = true;
			drawBuff_ = new DrawBuffer();
		}
		virtual ~ScrollView() {
			if (drawBuff_)
				delete drawBuff_;
		}


		static int getScrollWidth() {
			return  GetSystemMetrics(SM_CXVSCROLL);
		}

		void setVertScrollInfo(int max, int page) {
			vertMax = max;
			vertPage = page;

			if (!handle_)
				return;

			SCROLLINFO si = { 0 };
			si.cbSize = sizeof(si);
			si.nMin = 0;
			si.nMax = max;
			si.nPage = page;
			si.fMask = SIF_RANGE | SIF_PAGE;

			SetScrollInfo(handle_, SB_VERT, &si, TRUE);
		}

		void setHoriScrollInfo(int max, int page) {
			horiMax = max;
			horiPage = page;

			if (!handle_)
				return;


			SCROLLINFO si = { 0 };
			si.cbSize = sizeof(si);
			si.nMin = 0;
			si.nMax = max;
			si.nPage = page;
			si.fMask = SIF_RANGE | SIF_PAGE;


			SetScrollInfo(handle_, SB_HORZ, &si, TRUE);
		}
		void addVertPos(int val) {
			vertPos += val;

			if (vertPos > vertMax - vertPage + 1) {
				vertPos = vertMax - vertPage + 1;
			}
			else if (vertPos < 0) {
				vertPos = 0;
			}
		}

		void addHoriPos(int val) {
			horiPos += val;

			if (horiPos > horiMax - horiPage + 1) {
				horiPos = horiMax - horiPage + 1;
			}
			else if (horiPos < 0) {
				horiPos = 0;
			}
		}

		//virtual void onHover() {
		//	onDraw();
		//}

		//virtual void onLeave() {
		//	onDraw();
		//}

		virtual int32_t getHoriPos() {
			return horiPos * Control::GlobalFont().GetFontSize();
		}

		virtual int32_t getVertPos() {
			return vertPos * Control::GlobalFont().GetFontSize();
		}

		virtual void onMeasure() override;

		virtual void onDraw() override;

		virtual void Init() override;

	

		virtual bool ControlProc(HWND, UINT, WPARAM, LPARAM, LRESULT& ret) override;
	};

	typedef  std::shared_ptr<sdf::ScrollView> PtrScrollView;

}




#define ui_scroll ui_control(sdf::ScrollView) 

