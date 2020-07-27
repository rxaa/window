#ifndef ListBox_h__2013_8_1__19_52
#define ListBox_h__2013_8_1__19_52

namespace sdf {
	class ListBox
		: public Control {
	private:
		int textLength_;
	public:
		bool mutiSelect = false;
		bool mutiColumn = false;
		std::function<void()> onClick_;


		ListBox()
			: textLength_(0) {
		}

		inline void clear() {
			DF_ASSERT(handle_ != NULL);
			SendMessage(handle_, LB_RESETCONTENT, 0, 0);
		}

		//统计个数
		inline int count() {
			DF_ASSERT(handle_ != NULL);
			return (int)SendMessage(handle_, LB_GETCOUNT, 0, 0);
		}

		//删除index
		inline BOOL del(int Index) {
			DF_ASSERT(Index >= 0 && Index < count());
			return (BOOL)SendMessage(handle_, LB_DELETESTRING, Index, 0) + 1;
		}

		//获取选中的index
		int getSelectIndex() {
			DF_ASSERT(handle_ != NULL);
			return (int)SendMessage(handle_, LB_GETCURSEL, 0, 0); //获取选中值
		}

		inline BOOL setSelectIndex(int index) {
			DF_ASSERT(index >= 0 && index < count());
			return (BOOL)SendMessage(handle_, LB_SETCURSEL, index, 0) + 1;
		}


		void SetHorizontal(const df::CC& str);

		inline int add(const df::CC& str) {
			if (!handle_) {
				if (!initList)
					initList.reset(new std::vector<String>());

				initList->push_back(str.ToString());
				return 0;
			}
			if (str.length_ > textLength_ && currentWindow_) {
				SetHorizontal(str);
			}
			int i = (int)SendMessage(handle_, LB_ADDSTRING, 0, (LPARAM)str.char_);
			return i;
		}

		//获取index的字符串内容
		df::String getIndexText(int Index) {
			df::String str;
			getIndexText(Index, str);
			return str;
		}

		//获取index的字符串内容
		void getIndexText(int Index, df::String& str) {
			DF_ASSERT(Index >= 0 && Index < count());
			intptr_t len = SendMessage(handle_, LB_GETTEXTLEN, Index, 0) + 1;
			str.resize(len);
			len = SendMessage(handle_, LB_GETTEXT, Index, (LPARAM)&str[0]);//提取选中字符

		}
		virtual void onDraw() {
			//COUT(tt_("重绘ListBox"));
			if (pos.w > 0 && pos.h > 0) {
				updateDrawXY();

				DrawBuffer* draw = getDraw();

				drawStyle(draw, style, parent_->needDraw, false);
				//update();
				updateHandleXy(gdi_, draw);
			}
		}

		virtual void onMouseMove(int32_t x, int32_t y) override;
	protected:
		Gdi gdi_;
		std::unique_ptr<std::vector<String>> initList;

		///初始化
		virtual void Init();

		virtual bool ControlProc(HWND, UINT, WPARAM, LPARAM, LRESULT& ret) override;

	};


}

#define ui_list_box ui_control(sdf::ListBox) 


#endif // ListBox_h__2013_8_1__19_52
