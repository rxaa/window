#pragma once


namespace sdf
{
	template <typename Ty>
	class ListView : public ScrollView {
	protected:

	public:
		std::vector<Ty> list_;
		std::function<std::shared_ptr<sdf::View>(Ty& dat, size_t index)> onCreateView;
		std::function<void(Ty& dat, size_t index)> onShowView;
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
			}
			else {
				COUT(tt_("Empty onShowView!"));
			}
		}

		void del(intptr_t i) {
			DF_ASSERT(i >= 0 && i < (intptr_t)list_.size());
			if (i >= 0 && i < (intptr_t)list_.size()) {
				memberList_[i]->removeFromParent(false);
				memberList_.erase(i);
				list_.erase(i);
			}
		}

		void add(const Ty& dat) {
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
	con->dat = &dat;\
	con->index = index;\
	return con;\
};\
v.onShowView = [&](auto& dat, size_t index) {\
	auto con = v.castMember<Ty>(index);\
	con->dat = &dat;\
	con->index = index;\
	con->bindUpdate();\
};