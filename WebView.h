#pragma once


namespace sdf {
	class WebView :
		public Control
	{
		Gdi gdi_;
		std::unique_ptr< df::WebBrowser> web;
		std::wstring url_;
	public:

		void navigate(const std::wstring & szUrl);
		virtual void onMouseMove(int32_t x, int32_t y) override;
		virtual void onMeasure() override;
		virtual void onDraw() override;
	protected:
		virtual void Init() override;

		virtual bool ControlProc(HWND, UINT msg, WPARAM wParam, LPARAM, LRESULT& ret) override;
	};
}



#define ui_web_view ui_control(sdf::WebView) 