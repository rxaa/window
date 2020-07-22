#pragma once


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
			pos.vertical = true;
			drawBuff_ = new DrawBuffer(&gdi_);
		}
		virtual ~ScrollView() {
			if (drawBuff_)
				delete drawBuff_;
		}


		static int getScrollWidth() {
			return GetSystemMetrics(SM_CXVSCROLL);
		}

		void setVertScrollInfo(int max, int page);

		void setHoriScrollInfo(int max, int page);

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

		virtual void onMouseMove(int32_t x, int32_t y) override;
		virtual void onLeave()  override;

		//更新在父容器中的位置
		/*virtual void updateDrawXY() override {
			 updateHandleXy();
		}*/

		virtual void onMeasure() override;

		virtual void onDraw() override;

		virtual void Init() override;


		virtual bool ControlProc(HWND, UINT, WPARAM, LPARAM, LRESULT& ret) override;
	};

	typedef  std::shared_ptr<sdf::ScrollView> PtrScrollView;

}




#define ui_scroll ui_control(sdf::ScrollView) 

