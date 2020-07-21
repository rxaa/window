#pragma once

namespace sdf {

	class ImageMore :public sdf::View {
	public:


		ImageMore() {
			style.radius = 360;
			style.backColor = sdf::Color::blue;
		}
		~ImageMore() {

		}

		virtual void onDraw() override
		{
			if (!parent_)
				return;

			updateDrawXY();

			auto* draw = getDraw();

			if (Control::drawStyle(draw, style, parent_->needDraw)) {
				int32_t w = (int32_t)(pos.w / 3);
				w -= (int32_t)(w / 2);

				int32_t drawX = drawX_ + ((pos.w - w * 4) / 2);
				int32_t drawY = drawY_ + (pos.h - w) / 2;

				Gdiplus::SolidBrush brush(Gdiplus::Color((Gdiplus::ARGB)sdf::Color::white));
				// Create a Rect object that bounds the ellipse.
				for (int i = 0; i < 3; i++) {

					Gdiplus::RectF ellipseRect((float)drawX, (float)drawY, (float)w, (float)w);
					draw->graph_->FillEllipse(&brush, ellipseRect);
					drawX += w + w / 2;
				}

				drawMember(gdi_, draw);
			}

		}

	};

}

#define ui_image_more ui_control(sdf::ImageMore) 
