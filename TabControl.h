#ifndef TabControl_h__2013_9_2__20_49
#define TabControl_h__2013_9_2__20_49


	/*
	#define TCN_SELCHANGE           (TCN_FIRST - 1)
	#define TCN_SELCHANGING         (TCN_FIRST - 2)
	#if (_WIN32_IE >= 0x0400)
	#define TCN_GETOBJECT           (TCN_FIRST - 3)
	#endif      // _WIN32_IE >= 0x0400
	#if (_WIN32_IE >= 0x0500)
	#define TCN_FOCUSCHANGE         (TCN_FIRST - 4)
	#endif      // _WIN32_IE >= 0x0500
	#endif      // NOTABCONTROL
	*/

namespace sdf
{
	class Window;

	class TabControl
		:public Control
	{
	protected:
		//tab¸öÊý
		int count_;
		Window * showWindow_;
		sdf::List<Window*> winList_;

		static LONG_PTR tabProc_;
	public:
		std::function<void(int)> onSelectChange_;
		TabControl()
			: count_(0)
			, showWindow_(nullptr)
		{

		}
		~TabControl()
		{

		}

		void SwitchWindow(int index);

		void Init(int id)
		{
			Control::Init(id);
			//tabProc_=SetWindowLongPtr(handle_, GWL_WNDPROC, (LONG_PTR) TabProc);
		}

		void Add(const CC & title,Window & win);

		inline int GetSelect()
		{
			return (int)::SendMessage(handle_, TCM_GETCURSEL, 0, 0L);
		}
		inline void SetSelect(int nItem)
		{
			if(nItem<0 && nItem>=count_)
			{
				BREAK_POINT_MSG("Ë÷Òý³¬·¶Î§");
				return;
			}
			int prev=(int)::SendMessage(handle_, TCM_SETCURSEL, nItem, 0L);
			if(prev!=nItem  && onSelectChange_  &&  prev>=0 )
				SwitchWindow(nItem);
		}

		bool ControlProc( HWND, UINT msg, WPARAM wParam, LPARAM ) override;
		bool ControlNotify(LPNMHDR ) override;

	private:
		static LRESULT  __stdcall TabProc( HWND hDlg, uint message, WPARAM wParam, LPARAM lParam );
		TabControl(const TabControl& ) ;
		TabControl& operator = (const TabControl& );
	};


}

#endif // TabControl_h__2013_9_2__20_49
