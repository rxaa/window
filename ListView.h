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
		//����:��
		std::function<void(int)> onSelectChange_;

		std::function<void(int)> onDoubleClick_;

		struct Style
		{
			//ѡ��ĳ��ʹ���и�����ֻ������report����listctrl��
			static const int FullRowSelect=LVS_EX_FULLROWSELECT;
			//�����ߣ�ֻ������report����listctrl��
			static const int GridLines=LVS_EX_GRIDLINES;
			//itemǰ����checkbox�ؼ�
			static const int CheckBox=LVS_EX_CHECKBOXES;
		};

		//�����(����,��С)
		bool AddColumn(const CC & name,int size=80)
		{
			LVCOLUMN lvc;
			lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			// ���LVCOLUMN�ṹ
			lvc.cx = size;						// ����
			lvc.iImage = 0;
			lvc.fmt = LVCFMT_LEFT;	// �������
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

		///�����
		bool AddItem(const CC & name,int imageI=0)
		{
			LVITEM lvI={0};
			lvI.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE | LVIF_IMAGE;
			// ���LVITEM
			lvI.state = 0;
			lvI.stateMask = 0;
			lvI.iItem = rowCount_;
			lvI.iImage = imageI;
			lvI.iSubItem = 0;
			// ����ı��ͳ���
			lvI.pszText = (TCHAR*) name.char_;
			//lvI.cchTextMax = lstrlen(lvI.pszText)+1;
			// ������
			if(ListView_InsertItem(handle_, &lvI) == -1)
				return false;
			rowCount_++;
			return true;
		}

		///�������(����,�к�)
		void AddSubItem(const CC & name,int column)
		{
			MY_ASSERT(column>0);
			MY_ASSERT(column<columnCount_);
			ListView_SetItemText(handle_ , rowCount_-1 , column , (LPTSTR)name.char_);
		}

		///ѡ���к�
		int GetSelectIndex()
		{
			return ListView_GetSelectionMark(handle_);
		}
		//ɾ����
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
		//ѡ�������
		int GetSelectedCount()
		{
			return ListView_GetSelectedCount(handle_);
		}

		//ѡ��״̬
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

		///��ȡindex��,id�е�����
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

		//���
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
