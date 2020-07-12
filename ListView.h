#pragma once


namespace sdf
{
	template <typename Ty>
	class ListView : public ScrollView {
	protected:

	public:
		std::vector< std::shared_ptr <Ty>> list_;
		std::function<std::shared_ptr<sdf::View>(std::shared_ptr < Ty>& dat, size_t index)> onCreateView;
		std::function<void(std::shared_ptr <Ty>& dat, size_t index)> onShowView;
		ListView() {
		}

		virtual ~ListView() {
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

		void update(intptr_t i) {
			if (onShowView) {
				DF_ASSERT(i >= 0 && i < (intptr_t)list_.size());
				if (i >= 0 && i < (intptr_t)list_.size()) {
					onShowView(list_[i], (size_t)i);
				}
				measureUpdate();
			}
			else {
				COUT(tt_("Empty onShowView!"));
			}
		}

		void del(intptr_t i, bool update) {
			DF_ASSERT(i >= 0 && i < (intptr_t)list_.size());
			if (i >= 0 && i < (intptr_t)list_.size()) {
				memberList_[i]->_removeFromParent(false);
				memberList_.erase(memberList_.begin() + i);
				list_.erase(list_.begin() + i);
			}
			if (update && onShowView) {
				for (size_t ind = (size_t)i; ind < list_.size(); ind++) {
					onShowView(list_[ind], ind);
				}
				onDraw();
			}
		}

		void add(const std::shared_ptr<Ty>& dat) {
			size_t i = list_.size();
			list_.push_back(dat);
			auto con = onCreateView(list_[i], i);
			addMember(con);
		}

		void clear() {
			removeAllMember();
			list_.clear();
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


}




#define ui_list_view(Ty) ui_control(sdf::ListView<Ty>) 

#define ui_list_bind(Ty,...)  v.onCreateView = [&](auto& dat, size_t index) {\
	auto con = std::make_shared<Ty>(__VA_ARGS__);\
	con->dat = dat;\
	con->index = index;\
	return con;\
};\
v.onShowView = [&](auto& dat, size_t index) {\
	auto con = v.castMember<Ty>(index);\
	con->dat = dat;\
	con->index = index;\
	con->bindUpdate();\
};