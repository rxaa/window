#pragma once

namespace sdf {
	class ImageView
		: public Control {
	protected:
		intptr_t showI = 0;
		uint32_t timerId = 0;
	public:
		Gdi gdi_;
		std::vector<std::shared_ptr<Bitmap>> imageList_;
		//间隔时间,毫秒
		uint32_t interval = 50;
		bool loop = true;
		ImageView() {

		}

		virtual ~ImageView() {
			//COUT(tt_("gone"));
			if (timerId > 0) {
				killTimer(timerId);
			}
		}


		template <class T, class ...Args>
		void add(T head, Args... rest) {
			imageList_.push_back(head);
			add(rest...);
		}

		void add() {

		}

		virtual void getContentWH(int32_t& w, int32_t& h) {
			w = Gdi::GetScreen().GetTextPixel(text).cx;
			if (w > 0) {
				h = GlobalFont().GetFontSize();
			}
		}

		virtual void onDraw();

		virtual void doCreate() override {


			Control::doCreate();

			if (imageList_.size() > 0) {
				auto& img = *imageList_[0];
				if (pos.w > 0 && pos.h < 0) {
					pos.h = pos.w * img.GetHeight() / img.GetWidth();
				}
				else if (pos.h > 0 && pos.w < 0) {
					pos.w = pos.h * img.GetWidth() / img.GetHeight();
				}
				else if (pos.h < 0 && pos.w < 0) {
					pos.h = img.GetHeight();
					pos.w = img.GetWidth();
				}
				pos.wrapX = false;
				pos.wrapY = false;
			}
		}

	protected:
		///初始化
		virtual void Init();

		virtual void onTimer(uint32_t) override;

		//virtual bool ControlProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& ret) override;

	};

	typedef  std::shared_ptr<sdf::ImageView> PtrImage;

}

#define ui_image ui_control(sdf::ImageView) 
