#ifndef TextBox_h__2013_8_1__16_39
#define TextBox_h__2013_8_1__16_39


namespace sdf
{
	class TextBox
		: public Control
	{
	public:
		std::function<void()> onChange_;
		std::function<void()> onFocus_;
		std::function<void()> onLeave_;

	protected:
		virtual bool ControlProc(HWND, UINT, WPARAM, LPARAM) override;
	};

}



#endif // TextBox_h__2013_8_1__16_39
