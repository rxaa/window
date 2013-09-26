#ifndef ComBox_h__2013_8_9__22_40
#define ComBox_h__2013_8_9__22_40


namespace sdf
{
	class ComBox
		: public Control
	{
	public:
		std::function<void()> onSelectChange_;
		std::function<void()> onTextChange_;

		inline void Clear()
		{
			MY_ASSERT(handle_ != NULL);
			SendMessage(handle_, CB_RESETCONTENT, 0, 0);
		}

		//统计个数
		inline int Count()
		{
			MY_ASSERT(handle_ != NULL);
			return (int) SendMessage(handle_, CB_GETCOUNT, 0, 0);
		}
		//删除index
		inline BOOL Delete(int Index)
		{
			MY_ASSERT(Index >= 0 && Index < Count());
			return (BOOL) SendMessage(handle_, CB_DELETESTRING, Index, 0) + 1;
		}

		//获取选中的index
		int GetSelectIndex()
		{
			MY_ASSERT(handle_ != NULL);
			return (int) SendMessage(handle_, CB_GETCURSEL, 0, 0); //获取选中值
		}

		inline BOOL SetSelectIndex(int index)
		{
			MY_ASSERT(index >= 0 && index < Count());
			if (onSelectChange_)
				onSelectChange_();
			return (BOOL) SendMessage(handle_, CB_SETCURSEL, index, 0) + 1;
		}


		inline int Add(const CC & str)
		{
			int i = (int) SendMessage(handle_, CB_ADDSTRING, 0, (LPARAM) str.char_);
			return i;
		}

		//获取index的字符串内容
		SS GetIndexText(int Index)
		{
			MY_ASSERT(Index >= 0 && Index < Count());
			int len = (int) SendMessage(handle_, CB_GETLBTEXTLEN, Index, 0) + 1;
			SS str(len + 1);
			len = SendMessage(handle_, CB_GETLBTEXT, Index, (LPARAM) str.GetBuffer());//提取选中字符
			if (len > 0)
				str.strLength_ = len;
			return str;
		}

		//获取index的字符串内容
		void GetIndexText(int Index, SS & str)
		{
			MY_ASSERT(Index >= 0 && Index < Count());
			int len = (int) SendMessage(handle_, CB_GETLBTEXTLEN, Index, 0) + 1;
			str.SetBufSizeNoCopy(len + 1);
			len = SendMessage(handle_, CB_GETLBTEXT, Index, (LPARAM) str.GetBuffer());//提取选中字符
			if (len > 0)
				str.strLength_ = len;
		}

	protected:
		bool ControlProc(HWND, UINT, WPARAM, LPARAM) override;

	};



}


#endif // ComBox_h__2013_8_9__22_40
