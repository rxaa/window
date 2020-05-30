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
		ChoiceItem(ChoiceBox & parent,int index)
			: parentBox_(parent)
			, parentIndex_(index)
		{
			//Init(id);
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
			for(int i=0;i<itemList.size();i++)
				delete itemList[i];
		}
		//句柄
		std::vector<ChoiceItem*> itemList;
		void AddItem()
		{
			itemList.push_back(new ChoiceItem(*this, (int)itemList.size()));
		}

		///设置单选
		void SetRadio(int index)
		{
			DF_ASSERT(index>=0);
			DF_ASSERT(index<itemList.size());
			for(int i=0;i<itemList.size();i++)
				SendMessage(itemList[i]->handle_, BM_SETCHECK, (i==index), 0);
		}

		ChoiceItem & operator[](int i)
		{
			DF_ASSERT(i>=0);
			DF_ASSERT(i<itemList.size());
			return *itemList[i];
		}

	/*	ChoiceItem ** begin() const
		{
			return itemList.begin();
		}

		ChoiceItem ** end() const
		{
			return itemList.end();
		}*/

	};

}

#endif // ChoiceBox_h__2013_8_10__12_26
