#ifndef ListView_h__2013_8_11__14_16
#define ListView_h__2013_8_11__14_16

#include <commctrl.h>

namespace sdf
{
	class ListView
		: public Control
	{
	protected:
		int columnCount_;
		int rowCount_;
	public:
		//参数:行
		std::function<void(int)> onSelectChange_;

		std::function<void(int)> onDoubleClick_;

		struct Style
		{
			//选中某行使整行高亮（只适用与report风格的listctrl）
			static const int FullRowSelect=LVS_EX_FULLROWSELECT;
			//网格线（只适用与report风格的listctrl）
			static const int GridLines=LVS_EX_GRIDLINES;
			//item前生成checkbox控件
			static const int CheckBox=LVS_EX_CHECKBOXES;
		};

		//添加列(标题,大小)
		bool AddColumn(const CC & name,int size=80)
		{
			LVCOLUMN lvc;
			lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			// 填充LVCOLUMN结构
			lvc.cx = size;						// 长度
			lvc.iImage = 0;
			lvc.fmt = LVCFMT_LEFT;	// 向左对齐
			lvc.pszText = (TCHAR*) name.char_;
			lvc.iSubItem = columnCount_;

			if (ListView_InsertColumn(handle_, columnCount_,	&lvc) == -1)
			{
				return false;
			}

			columnCount_++;
			return true;
		}

		void Init(int id,int style=Style::FullRowSelect | Style::GridLines)
		{
			Control::Init(id);
			rowCount_=0;
			columnCount_=0;


			DWORD dwStyle=ListView_GetExtendedListViewStyle(handle_);
			dwStyle |= style;	
			ListView_SetExtendedListViewStyle(handle_, dwStyle);
		}

		template<class... Args>
		void AddRow(const CC & name,Args &&... args)
		{
			AddItem(name);
			AddRowSubItem<1>(args...);
		}

		template<class... Args>
		void AddImageRow(int imageIndex,const CC & name, Args &&... args)
		{
			AddItem(name, imageIndex);
			AddRowSubItem<1>(args...);
		}


		template<int i,class CT, class... Args>
		void AddRowSubItem(CT && name, Args &&... args)
		{
			MY_ASSERT(i < columnCount_);
			AddSubItem(name,i);
			AddRowSubItem<i + 1>(args...);
		}

		template<int i>
		void AddRowSubItem()
		{
		}

		void SetImageList(ImageList & li)
		{
			SendMessage(handle_, LVM_SETIMAGELIST, (WPARAM)(LVSIL_SMALL), (LPARAM)(li.GetHandle()));
		}

		///添加项
		bool AddItem(const CC & name,int imageI=0)
		{
			LVITEM lvI={0};
			lvI.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE | LVIF_IMAGE;
			// 填充LVITEM
			lvI.state = 0;
			lvI.stateMask = 0;
			lvI.iItem = rowCount_;
			lvI.iImage = imageI;
			lvI.iSubItem = 0;
			// 项的文本和长度
			lvI.pszText = (TCHAR*) name.char_;
			//lvI.cchTextMax = lstrlen(lvI.pszText)+1;
			// 插入项
			if(ListView_InsertItem(handle_, &lvI) == -1)
				return false;
			rowCount_++;
			return true;
		}

		///添加子项(内容,列号)
		void AddSubItem(const CC & name,int column)
		{
			MY_ASSERT(column>0);
			MY_ASSERT(column<columnCount_);
			ListView_SetItemText(handle_ , rowCount_-1 , column , (LPTSTR)name.char_);
		}

		///选中行号
		int GetSelectIndex()
		{
			return ListView_GetSelectionMark(handle_);
		}
		//删除行
		bool Delete(int row)
		{
			MY_ASSERT(row>=0);
			MY_ASSERT(row<rowCount_);
			if(ListView_DeleteItem(handle_, row) )
			{
				rowCount_--;
				return true;
			}
			return false;
		}
		//选中项个数
		int GetSelectedCount()
		{
			return ListView_GetSelectedCount(handle_);
		}

		//选中状态
		BOOL GetItemState(int id)
		{
			MY_ASSERT(id>=0);
			MY_ASSERT(id<rowCount_);
			return ListView_GetItemState(handle_,id,LVIS_SELECTED);
		}

		BOOL SetText(int row,int column,const sdf::CC & str )
		{
			MY_ASSERT(row>=0);
			MY_ASSERT(row<rowCount_);
			MY_ASSERT(column>=0);
			MY_ASSERT(column<columnCount_);
			LVITEM lvi;
			lvi.iSubItem = column;
			lvi.pszText = (LPTSTR) str.char_;
			return (BOOL) ::SendMessage(handle_, LVM_SETITEMTEXT, row, (LPARAM)&lvi);
		}

		///获取index行,id列的内容
		sdf::SS GetText(int row,int column)
		{
			MY_ASSERT(row>=0);
			MY_ASSERT(row<rowCount_);
			MY_ASSERT(column>=0);
			MY_ASSERT(column<columnCount_);

			SS ss;
			LVITEM lvi={0};
			lvi.iSubItem = column;

			int nRes=0;
			for(;;)
			{
				lvi.cchTextMax = ss.GetBufferSize();
				lvi.pszText = ss.GetBuffer();
				nRes  = (int)::SendMessage(handle_, LVM_GETITEMTEXT, (WPARAM)row,
					(LPARAM)&lvi);

				if (nRes < ss.GetBufferSize() - 1)
				{
					ss.strLength_ = nRes;
					break;
				}
				

				ss.SetBufSizeNoCopy(ss.GetBufferSize() * 2);
			}

			return ss;
		}
		inline int GetColumnCount() const
		{
			return columnCount_;
		}


		inline int GetRowCount() const
		{
			return rowCount_;
		}

		//清空
		BOOL Clear()
		{
			rowCount_=0;
			return ListView_DeleteAllItems(handle_);
		}


	protected:
		virtual bool ControlNotify(LPNMHDR ) override;

	};


}

#endif // ListView_h__2013_8_11__14_16
