#ifndef Button_h__2013_8_1__14_27
#define Button_h__2013_8_1__14_27

namespace sdf
{
	class Button
		: public Control
	{
		//绘图回调
		typedef void (*DrawCallBack)(Button & but);
		static void DefaultDraw(Button & but, DWORD normal, DWORD hover);
	public:
		Gdi buttonGdi_;

		//按钮状态
		enum
		{
			StateNormal = 0
			, StateHover = 1
			, StatePressed = 2
		};
		int buttonState;
		bool isFocused_;
		bool isDisable_;

		//事件
		std::function<void()> onClick_;
		//全局绘图缓冲
		static Bitmap buttonBmp_;
		static char * buttonBmpBuf_;


		DrawCallBack onDraw_;

		//颜色(ARGB)
		//内存:(BGRA)
		static const DWORD blueColor_ = 0xFF4192E1;
		static const DWORD blueHoverColor_ = 0xFF1B80E4;
		static const DWORD orangeColor_ = 0xFFE19241;
		static const DWORD orangeHoverColor_ = 0xFFE4801B;
		static const DWORD greenColor_ = 0xFF1ABC6E;
		static const DWORD greenHoverColor_ = 0xFF0FB264;
		static const DWORD redColor_ = 0xFFFF2F2F;
		static const DWORD redHoverColor_ = 0xFFF00000;

		inline static void BlueButton(Button & but)
		{
			DefaultDraw(but, blueColor_, blueHoverColor_);
		}

		inline static void OrangeButton(Button & but)
		{
			DefaultDraw(but, orangeColor_, orangeHoverColor_);
		}

		inline static void GreenButton(Button & but)
		{
			DefaultDraw(but, greenColor_, greenHoverColor_);
		}

		inline static void RedButton(Button & but)
		{
			DefaultDraw(but, redColor_, redHoverColor_);
		}

		Button()
			: buttonState(StateNormal)
			, isFocused_(false)
			, isDisable_(false)
			, onDraw_(BlueButton)
		{

		}

		///用资源标识符初始化
		void Init(int id);

	protected:
		static LRESULT  __stdcall ButtonProc(HWND hDlg, uint message, WPARAM wParam, LPARAM lParam);
		bool ControlProc(HWND, UINT, WPARAM, LPARAM) override;
	};

}

#endif // Button_h__2013_8_1__14_27
