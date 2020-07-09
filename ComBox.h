#ifndef ComBox_h__2013_8_9__22_40
#define ComBox_h__2013_8_9__22_40


namespace sdf
{
	class ComBox
		: public Control
	{
	public:
		std::function<void()> onSelectChange_;
		std::function<void()> onChange_;
		bool editAble = false;

		ComBox() {
			pos.w = Control::GlobalFont().getRawSize() * 5;
			pos.h = Control::GlobalFont().getRawSize() + 7;
		}
		virtual ~ComBox() {

		}

		inline void clear()
		{
			DF_ASSERT(handle_ != NULL);
			SendMessage(handle_, CB_RESETCONTENT, 0, 0);
		}

		//统计个数
		inline int count()
		{
			DF_ASSERT(handle_ != NULL);
			return (int) ::SendMessage(handle_, CB_GETCOUNT, 0, 0);
		}
		//删除index
		inline BOOL del(int Index)
		{
			DF_ASSERT(Index >= 0 && Index < count());
			return (BOOL) ::SendMessage(handle_, CB_DELETESTRING, Index, 0) + 1;
		}

		//获取选中的index
		int getSelectIndex()
		{
			DF_ASSERT(handle_ != NULL);
			return (int) ::SendMessage(handle_, CB_GETCURSEL, 0, 0); //获取选中值
		}

		inline BOOL setSelectIndex(int index)
		{
			DF_ASSERT(index >= 0 && index < count());
			return (BOOL) ::SendMessage(handle_, CB_SETCURSEL, index, 0) + 1;
		}


		inline int add(const df::CC& str)
		{
			if (!handle_) {
				if (!initList)
					initList.reset(new std::vector<String>());

				initList->push_back(str.ToString());
				return 0;
			}

			int i = (int)SendMessage(handle_, CB_ADDSTRING, 0, (LPARAM)str.char_);
			return i;
		}


		//获取index的字符串内容
		df::String getIndexText(int Index)
		{
			df::String str;
			getIndexText(Index, str);
			return str;
		}

		//获取index的字符串内容
		void getIndexText(int Index, df::String& str)
		{
			DF_ASSERT(Index >= 0 && Index < count());
			intptr_t len = SendMessage(handle_, CB_GETLBTEXTLEN, Index, 0) + 1;
			str.resize(len);
			len = SendMessage(handle_, CB_GETLBTEXT, Index, (LPARAM)&str[0]);//提取选中字符

		}
		virtual void onDraw() {
			COUT(tt_("onDraw ComBox"));
			updateDrawXY();
			update();
			DrawBuffer* draw = getDraw();
			gdi_.DrawTo(draw->buttonBmp_, drawX_, drawY_, pos.w, pos.h);
		}
	protected:
		Gdi gdi_;
		std::unique_ptr<std::vector<String>> initList;

		///初始化
		virtual void Init();
		bool ControlProc(HWND, UINT, WPARAM, LPARAM, LRESULT& ret) override;

	};



}
#define ui_onselect_change v.onSelectChange_ = [&]()

#define ui_combox ui_control(sdf::ComBox) 


#endif // ComBox_h__2013_8_9__22_40
