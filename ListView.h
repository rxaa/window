#pragma once


namespace sdf
{
	template <typename Ty>
	class ListView : public ScrollView {
	protected:
		int32_t dragX = -1;
		int32_t dragY = -1;
		intptr_t dragStart = -1;
		intptr_t dragEndLeft = -1;
		intptr_t dragEndRight = -1;
	public:
		std::vector< std::shared_ptr <Ty>> list_;
		std::function<std::shared_ptr<sdf::View>(std::shared_ptr < Ty>& dat, size_t index)> onCreateView;
		std::function<void(std::shared_ptr <Ty>& dat, size_t index, bool update)> onShowView;
		std::function<void(intptr_t dragEndLeft, intptr_t dragEndRight)> onDrag;
		std::set<size_t> checkedList;
		static const int32_t minDragSize = 10;

		ListView() {
			hasCursor = false;
		}

		virtual ~ListView() {
		}

		std::shared_ptr <Ty>& getData(size_t i) {
			if (i >= list_.size()) {
				Throw_df(i << tt_(" : out of getData range : ") << list_.size());
			}
			return list_[i];
		}

		bool isChecked(size_t i) {
			return checkedList.find(i) != checkedList.end();
		}

		void clearCheck() {
			checkedList.clear();
		}

		size_t checkedCount() {
			return checkedList.size();
		}

		virtual void doCreate() {
			ScrollView::doCreate();
			if (onCreateView) {
				for (size_t i = 0; i < list_.size(); i++) {
					auto con = onCreateView(list_[i], i);
					_addSub(con);
					con->doCreate();
				}
			}
		}

		virtual bool onLeftDown() override;
		virtual void onMouseMove(int32_t x, int32_t y) override;
		virtual bool onCaptureLeftUp() override;

		/// <summary>
		/// 
		/// </summary>
		/// <param name="ind"></param>
		void updateAll(size_t startIndex = 0) {
			for (; startIndex < list_.size(); startIndex++) {
				if (startIndex >= memberList_.size()) {
					auto con = onCreateView(getData(startIndex), startIndex);
					addMember(con);
				}
				else
					onShowView(getData(startIndex), startIndex, true);
			}
			measureUpdate();
		}


		void update(size_t i) {
			if (onShowView) {
				onShowView(getData(i), i, true);
				measureUpdate();
			}
			else {
				COUT(tt_("Empty onShowView!"));
			}
		}

		void del(intptr_t i, bool update) {

			if (i >= 0 && i < (intptr_t)list_.size()) {

				if (update)
					checkedList.erase((size_t)i);

				getMember(i)->_removeFromParent(false);
				memberList_.erase(memberList_.begin() + i);


				list_.erase(list_.begin() + i);

				if (update && onShowView) {
					for (size_t ind = (size_t)i; ind < list_.size(); ind++) {
						onShowView(getData(ind), ind, true);
					}
					measureUpdate();
				}
			}
			else {
				Throw_df(i << tt_(" : ListView del failed index out range:") << list_.size());
			}

		}




		void doDrag(intptr_t endIndexLeft, intptr_t endIndexRight) {
			COUT(tt_("doDrag: ") << endIndexLeft << tt_(" - ") << endIndexRight);
			intptr_t minIndex = endIndexLeft;
			if (minIndex < 0)
				minIndex = 0;
			std::vector<std::shared_ptr< Ty>> listData;
			std::vector<std::shared_ptr<Control>> listView;

			bool hasChecked = true;
			if (checkedList.size() == 0 && dragStart >= 0) {
				if (dragStart < minIndex) {
					minIndex = dragStart;
				}
				listData.emplace_back(std::move(getData(dragStart)));
				listView.emplace_back(std::move(getMember(dragStart)));
				hasChecked = false;
			}

			for (size_t checkI : checkedList) {
				if ((intptr_t)checkI < minIndex) {
					minIndex = checkI;
				}

				listData.emplace_back(std::move(getData(checkI)));
				listView.emplace_back(std::move(getMember(checkI)));
			}

			if (endIndexLeft > 0 && endIndexLeft < (intptr_t)list_.size() - 1) {
				std::stable_sort(list_.begin() + minIndex, list_.begin() + endIndexLeft + 1, [](const auto& left, const auto& right)
					{
						if (right == nullptr && left != nullptr)
							return true;
						return false;
					});
				std::stable_sort(memberList_.begin() + minIndex, memberList_.begin() + endIndexLeft + 1, [](const auto& left, const auto& right)
					{
						if (right == nullptr && left != nullptr)
							return true;
						return false;
					});
			}

			if (endIndexRight>=0 && endIndexRight < (intptr_t)list_.size() - 1) {
				std::stable_sort(list_.begin() + endIndexRight, list_.end(), [](const auto& left, const auto& right)
					{
						if (left == nullptr && right != nullptr)
							return true;
						return false;
					});
				std::stable_sort(memberList_.begin() + endIndexRight, memberList_.end(), [](const auto& left, const auto& right)
					{
						if (left == nullptr && right != nullptr)
							return true;
						return false;
					});
			}
			checkedList.clear();
			for (intptr_t i = minIndex; i < (intptr_t)list_.size(); i++) {
				if (list_[i] == nullptr) {
					for (intptr_t listI = 0; listI < (intptr_t)listData.size(); listI++) {
						list_[i + listI] = std::move(listData[listI]);
						memberList_[i + listI] = std::move(listView[listI]);
						if (hasChecked)
							checkedList.insert(i + listI);
					}
					break;
				}
			}



			for (; minIndex < (intptr_t)list_.size(); minIndex++) {
				onShowView(getData(minIndex), minIndex, false);
			}
			measureUpdate();
		}

		void setCheck(size_t index) {
			if (isKeyCtrlPress()) {
				return;
			}
			else if (isKeyShiftPress()) {
				eachLastPress([&](size_t i)
					{
						auto& lv = getMember(i);
						if (!isChecked(i)) {
							checkedList.insert(i);
							lv->bindUpdate();
						}
					});
			}
			else {

				if (!isChecked(index)) {
					checkIndex(index);
				}
			}
			setLastPressIndex();
		}

		void checkIndex(size_t index) {

			if (isKeyShiftPress())
				return;

			if (isKeyCtrlPress()) {
				if (isChecked(index)) {
					checkedList.erase(index);
				}
				else {
					checkedList.insert(index);
				}
				return;
			}

			for (auto it = checkedList.begin(); it != checkedList.end();) {
				auto checkI = *it;
				auto& lv = getMember(checkI);
				if (checkI != index) {
					checkedList.erase(it++);
					lv->bindUpdate();
				}
				else {
					it++;
				}
			}
			checkedList.insert(index);
		}

		void add(const std::shared_ptr<Ty>& dat, bool update) {
			size_t i = list_.size();
			list_.push_back(dat);
			auto con = onCreateView(getData(i), i);
			addMember(con);

			if (update) {
				measureUpdate();
			}
		}

		void clear(bool update = true) {
			clearCheck();
			removeAllMember();
			list_.clear();

			if (update) {
				measureUpdate();
			}
		}

		template <class TT, class ...Args>
		void bindView(Args&& ... rest) {
			onCreateView = [=](auto& dat, size_t index) {
				auto con = std::make_shared<TT>(rest...);
				con->dat = dat;
				con->index = index;
				return con;
			};
			onShowView = [this](auto& dat, size_t index, bool update) {
				auto con = castMember<TT>(index);
				con->dat = dat;
				con->index = index;
				if (update)
					con->bindUpdate();
			};
		}

		virtual void bindUpdate(bool draw = true) {
			if (onBind_) {
				onBind_();
			}
			/*for (auto& sub : memberList_) {
				sub->bindUpdate(false);
			}*/
		}
	};


	template<typename Ty>
	inline bool ListView<Ty>::onLeftDown()
	{
		if (!onDrag)
			return true;
		if (hoverViewIndex >= 0) {

			dragStart = hoverViewIndex;
		}
		else {
			dragStart = -1;
		}
		dragEndLeft = -1;
		dragEndRight = -1;
		dragX = mouseX;
		dragY = mouseY;
		return true;
	}

	template<typename Ty>
	inline void ListView<Ty>::onMouseMove(int32_t x, int32_t y)
	{
		ScrollView::onMouseMove(x, y);

		//COUT(tt_("onMouseMove ") << x << tt_(" - ") << y);

		if (!onDrag) {
			return;
		}

		if (x<0 || y<0 || x>pos.w || y>pos.h) {
			if (linePos >= 0) {
				hasCursor = true;
				SetCursor(LoadCursor(NULL, IDC_NO));
				//COUT(tt_("on leave"));
				linePos = -1;
				onDraw();
			}
			return;
		}


		if (!hoverView) {
			return;
		}

		if (dragX == -1 || dragY == -1 || dragStart == -1)
			return;

		if (pos.vertical) {

			if (std::abs(mouseY - dragY) > minDragSize * Control::scale_) {


				hasCursor = true;
				SetCursor(LoadCursor(NULL, IDC_SIZENS));
				//COUT(tt_("start drag"));
				int32_t vert = getVertPos();

				auto viewY = hoverView->pos.y - vert;

				int32_t newPos = 0;
				if (y <= viewY + hoverView->pos.controlH / 2) {
					newPos = hoverView->pos.y;
					dragEndLeft = hoverViewIndex - 1;
					dragEndRight = hoverViewIndex;

				}
				else {
					newPos = hoverView->pos.y + hoverView->pos.h;
					dragEndLeft = hoverViewIndex;
					dragEndRight = hoverViewIndex + 1;
				}

				if (newPos != linePos)
				{
					linePos = newPos;
					onDraw();
				}

			}
			else {
				if (linePos >= 0) {
					linePos = -1;
					onDraw();
				}
			}
		}
		else {
			if (std::abs(mouseX - dragX) > minDragSize * Control::scale_) {

				hasCursor = true;
				SetCursor(LoadCursor(NULL, IDC_SIZENS));
				//COUT(tt_("start drag"));
				int32_t hori = getHoriPos();
				auto viewX = hoverView->pos.x - hori;

				int32_t newPos = 0;
				if (x <= viewX + hoverView->pos.controlW / 2) {
					newPos = hoverView->pos.x;
					dragEndLeft = hoverViewIndex - 1;
					dragEndRight = hoverViewIndex;
				}
				else {
					newPos = hoverView->pos.x + hoverView->pos.w;
					dragEndLeft = hoverViewIndex;
					dragEndRight = hoverViewIndex + 1;
				}

				if (newPos != linePos)
				{
					linePos = newPos;
					onDraw();
				}
			}
			else {
				if (linePos >= 0) {
					linePos = -1;
					onDraw();
				}
			}
		}
	}

	template<typename Ty>
	inline bool ListView<Ty>::onCaptureLeftUp()
	{
		if (!onDrag)
			return true;

		hasCursor = false;
		dragX = -1;
		dragY = -1;
		if (linePos >= 0) {
			linePos = -1;

			if (dragStart >= 0 && dragStart != dragEndLeft && dragStart != dragEndRight) {
				onDrag(dragEndLeft, dragEndRight);
			}

			onDraw();

			return false;
		}
		return true;
	}



}




#define ui_list_view(Ty) ui_control(sdf::ListView<Ty>) 