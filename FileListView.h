#include "..\df\Proc.h"
#ifndef FileListView_h__2013_11_5__20_59
#define FileListView_h__2013_11_5__20_59


namespace sdf
{
	class FileListView
		: public ListView
	{
	private:
		std::unique_ptr < ImageList> imageList_;
		
		df::Dir dirList_;
	public:
		SS currentMenu_;
		void ShowMenu(const CC & menu);

		const ImageList &  GetImageList() const { return *imageList_; }

		void InitImage(int disk, int menu, int file);

		SS & NaviMenu(int i)
		{
			if (i < 0)
				return currentMenu_;

			if (i >= dirList_.listDir_.Size())
			{
				df::RunProc(currentMenu_ +tcc_("\\") + GetText(i, 0));
				return currentMenu_;
			}

			currentMenu_ << GetText(i, 0) + cct_("\\");
			ShowMenu(currentMenu_);
			return currentMenu_;
		}

		List<df::Dir::DirInfo> & GetDirList()
		{
			return dirList_.listDir_;
		}

		List<df::Dir::DirInfo> & GetFileList()
		{
			return dirList_.listFile_;
		}

		SS & Refresh()
		{
			ShowMenu(currentMenu_);
			
			return currentMenu_;
		}

		SS & Back()
		{
			auto pos = currentMenu_.Find(tt_('\\'), currentMenu_.Length() - 2, false);
			if (pos > 0)
				currentMenu_.SetSize(pos + 1);
			else
				currentMenu_.ClearString();

			ShowMenu(currentMenu_);
			SetFocus();
			return currentMenu_;

		}

	};




}

#endif // FileListView_h__2013_11_5__20_59
