#ifndef ListBox_h__2013_8_1__19_52
#define ListBox_h__2013_8_1__19_52

namespace sdf
{
	class ListBox
		: public Control
	{
	private:
		int textLength_;
	public:
		std::function<void()> onClick_;
		ListBox()
			: textLength_(0)
		{

		}
		inline void Clear()
		{
			MY_ASSERT(handle_ != NULL);
			SendMessage(handle_, LB_RESETCONTENT, 0, 0);
		}

		//统计个数
		inline int Count()
		{
			MY_ASSERT(handle_ != NULL);
			return (int) SendMessage(handle_, LB_GETCOUNT, 0, 0);
		}
		//删除index
		inline BOOL Delete(int Index)
		{
			MY_ASSERT(Index >= 0 && Index < Count());
			return (BOOL) SendMessage(handle_, LB_DELETESTRING, Index, 0) + 1;
		}

		//获取选中的index
		int GetSelectIndex()
		{
			MY_ASSERT(handle_ != NULL);
			return (int) SendMessage(handle_, LB_GETCURSEL, 0, 0); //获取选中值
		}

		inline BOOL SetSelectIndex(int index)
		{
			MY_ASSERT(index >= 0 && index < Count());
			if(onClick_)
				onClick_();
			return (BOOL) SendMessage(handle_, LB_SETCURSEL, index, 0) + 1;
		}


		void SetHorizontal(const CC & str);

		inline int Add(const CC & str)
		{
			if (str.length_ > textLength_ && currentWindow_)
			{
				SetHorizontal(str);
			}
			int i = (int) SendMessage(handle_, LB_ADDSTRING, 0, (LPARAM) str.char_);
			return i;
		}

		//获取index的字符串内容
		SS GetIndexText(int Index)
		{
			MY_ASSERT(Index >= 0 && Index < Count());
			int len = (int) SendMessage(handle_, LB_GETTEXTLEN, Index, 0) + 1;
			SS str(len + 1);
			len = SendMessage(handle_, LB_GETTEXT, Index, (LPARAM) str.GetBuffer());//提取选中字符
			if (len > 0)
				str.strLength_ = len;
			return std::move(str);
		}

		//获取index的字符串内容
		void GetIndexText(int Index, SS & str)
		{
			MY_ASSERT(Index >= 0 && Index < Count());
			int len = (int) SendMessage(handle_, LB_GETTEXTLEN, Index, 0) + 1;
			str.SetBufSizeNoCopy(len + 1);
			len = SendMessage(handle_, LB_GETTEXT, Index, (LPARAM) str.GetBuffer());//提取选中字符
			if (len > 0)
				str.strLength_ = len;
		}

	protected:
		virtual bool ControlProc(HWND, UINT, WPARAM, LPARAM) override;

	};


}

#endif // ListBox_h__2013_8_1__19_52
