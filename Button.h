#ifndef Button_h__2013_8_1__14_27
#define Button_h__2013_8_1__14_27

namespace sdf
{
	class Button
		: public Control
	{
		//��ͼ�ص�
		typedef void (*DrawCallBack)(Button & but);
		static void DefaultDraw(Button & but, DWORD normal, DWORD hover);
	public:
		//��ť��С
		RECT buttonRect_;
		Gdi buttonGdi_;
		//HDC butDc_;

		//��ť״̬
		enum
		{
			StateNormal = 0
			, StateHover = 1
			, StatePressed = 2
		};
		int buttonState;
		bool isFocused_;
		bool isDisable_;

		//�¼�
		std::function<void()> onClick_;
		//ȫ�ֻ�ͼ����
		static Bitmap buttonBmp_;
		static char * buttonBmpBuf_;


		DrawCallBack onDraw_;

		static const DWORD blueColor_ = 0x4192E1;
		static const DWORD blueHoverColor_ = 0x1B80E4;
		static const DWORD orangeColor_ = 0xE19241;
		static const DWORD orangeHoverColor_ = 0xE4801B;
		static const DWORD greenColor_ = 0x1ABC6E;
		static const DWORD greenHoverColor_ = 0x0FB264;
		static const DWORD redColor_ = 0xFF2F2F;
		static const DWORD redHoverColor_ = 0xF00000;

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

		///����Դ��ʶ����ʼ��
		void Init(int id);

	protected:
		static LRESULT  __stdcall ButtonProc(HWND hDlg, uint message, WPARAM wParam, LPARAM lParam);
		bool ControlProc(HWND, UINT, WPARAM, LPARAM) override;
	};

}

#endif // Button_h__2013_8_1__14_27
