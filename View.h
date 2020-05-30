#ifndef WINDOWSTAP_VIEW_H
#define WINDOWSTAP_VIEW_H

namespace sdf {
	class View
		: public Control {
	public:
		Gdi gdi_;
		std::function<void(View&)> onCreate_;

		View(Control* parent) {
			setParent(parent);
		}

		virtual ~View() {
			//COUT(tt_("gone"));
		}


		virtual void getContentWH(int32_t& w, int32_t& h) {
			h = GlobalFont().GetFontSize();
			w = Gdi::GetScreen().GetTextPixel(text);
		}

		virtual void onDraw();

	protected:
		///初始化
		virtual void Init();
		virtual void initCreate() {
			doCreate(this);
		}
		virtual bool ControlProc(HWND, UINT, WPARAM, LPARAM) override {
			return true;
		}

	};

}


#define ui_view sdf::View * DF_MIX_LINENAME(UIBUTTON, __LINE__)=new sdf::View(&v);DF_MIX_LINENAME(UIBUTTON, __LINE__)->onCreate_=[&](sdf::View &v)


#endif //WINDOWSTAP_VIEW_H
