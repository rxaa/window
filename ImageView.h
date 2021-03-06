﻿#pragma once

namespace sdf {
	class ImageView
		: public Control {
	protected:
		intptr_t showI = 0;
	public:
		//std::vector<std::shared_ptr<Bitmap>> imageList_;

		std::shared_ptr<Bitmap> img_;

		ImageView() {

		}

		virtual ~ImageView() {
			//COUT(tt_("gone"));
		}


		/*template <class T, class ...Args>
		void add(T head, Args... rest) {
			imageList_.push_back(head);
			add(rest...);
		}
		void add() {

		}*/

		void setBitmap(const std::shared_ptr<Bitmap> & head) {
			img_ = head;
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

			if (img_) {
				auto& img = *img_;
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


	};

	typedef  std::shared_ptr<sdf::ImageView> PtrImage;

}

#define ui_image ui_control(sdf::ImageView) 
