#ifndef ImageList_h__2013_9_16__12_25
#define ImageList_h__2013_9_16__12_25


namespace sdf
{
	class ImageList
	{
	private:
		HIMAGELIST imageList_;
	public:
		ImageList(int size=3)
		{
			imageList_ = ImageList_Create(32, 32, ILC_COLOR24 , size, 3);
		}
		~ImageList()
		{
			ImageList_Destroy(imageList_);
		}


		HIMAGELIST GetHandle() const
		{
			return imageList_;
		}

		bool Add(int resourseId)
		{
			HBITMAP bmp = ::LoadBitmap(df::Global::progressInstance_, MAKEINTRESOURCE(resourseId));
			if (bmp == 0)
			{
				ERR(t_t("LoadBitmap failed,id:") + resourseId);
				return false;
			}
			ON_EXIT({
				::DeleteObject(bmp);
			});

			return ImageList_Add(imageList_, bmp, 0) != -1;
		}

	};
}

#endif // ImageList_h__2013_9_16__12_25
