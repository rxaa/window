#ifndef WINDOWSTAP_VIEW_H
#define WINDOWSTAP_VIEW_H

namespace sdf {
	class View
		: public Control {
	public:
		Gdi gdi_;
		View& v;

		View():v(*this) {
		}


		virtual ~View() {
			//COUT(tt_("gone"));
		}

	
		virtual void getContentWH(int32_t& w, int32_t& h) {
			w = Gdi::GetScreen().GetTextPixel(text);
			if (w > 0) 
				h = GlobalFont().GetFontSize();
			
		}


		virtual void onDraw();



	protected:
		///初始化
		virtual void Init();
		virtual bool ControlProc(HWND, UINT, WPARAM, LPARAM) override {
			return true;
		}

	};

	typedef  std::shared_ptr<sdf::View> PtrView;
}

#define ui_view ui_control(sdf::View) 


#endif //WINDOWSTAP_VIEW_H
