#ifndef ChoiceBox_h__2013_8_10__12_26
#define ChoiceBox_h__2013_8_10__12_26

namespace sdf
{

	class ChoiceBox;

	class ChoiceItem
		: public Control
	{
		friend ChoiceBox;
	protected:
		ChoiceBox & parentBox_;
		int parentIndex_;
	public:
		ChoiceItem(ChoiceBox & parent,int id,int index)
			: parentBox_(parent)
			, parentIndex_(index)
		{
			Init(id);
		}
		/*
		ChoiceItem(ChoiceItem && r)
			: Control(std::move(r))
			, parentBox_(r.parentBox_)
			, parentIndex_(r.parentIndex_)
		{
		}*/

		///获取在容器ChoiceBox中的索引
		int GetIndex()
		{
			return parentIndex_;
		}

	protected:

		bool ControlProc(HWND, UINT, WPARAM, LPARAM) override;
	};



	class ChoiceBox
	{
	public:
		std::function<void(int)> onClick_;
		ChoiceBox(void)
		{
		}

		~ChoiceBox(void)
		{
			for(int i=0;i<itemList.Count();i++)
				delete itemList[i];
		}
		//句柄
		List<ChoiceItem*> itemList;
		void AddItem(int id)
		{
			itemList.Add(new ChoiceItem(*this,id,itemList.Count()));
		}

		///设置单选
		void SetRadio(int index)
		{
			MY_ASSERT(index>=0);
			MY_ASSERT(index<itemList.Size());
			for(int i=0;i<itemList.Size();i++)
				SendMessage(itemList[i]->handle_, BM_SETCHECK, (i==index), 0);
		}

		ChoiceItem & operator[](int i)
		{
			MY_ASSERT(i>=0);
			MY_ASSERT(i<itemList.Size());
			return *itemList[i];
		}

	};

}

#endif // ChoiceBox_h__2013_8_10__12_26
