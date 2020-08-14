#pragma once


namespace sdf {

	class MenuItem {
	public:
		String text;
		PtrBmp img;
		std::function<void()> onClick;
	};

	class FormMenu : public sdf::Window {
	protected:
		std::vector<MenuItem> itemList_;
		std::function<void(df::CC c, size_t i)> onClick_;
	public:
		int32_t width = 0;
		FormMenu() {
		}

		FormMenu(std::function<void(df::CC c, size_t i)> && onClick) {
			onClick_ = std::move(onClick);
		}

		~FormMenu() {
			
		}

		template < class ...Args>
		FormMenu * add(const df::CC& head, Args... rest) {
			size_t index = itemList_.size();
			itemList_.push_back({ head.toString(),nullptr,[index,this]() {
				auto& menu = itemList_[index];
				onClick_(menu.text, index);
				} });
			add(rest...);
			return this;
		}
		FormMenu* add() {
			return this;
		}

		template < class ...Args>
		static std::shared_ptr<FormMenu> create(Args && ... rest) {
			return std::make_shared<FormMenu>(rest...);
		}

		virtual void onInActive() override {
			close();
		}

		void addItem(const MenuItem& item) {
			itemList_.push_back(item);
		}

		virtual bool onClose(int code) override {
			onClick_ = nullptr;
			itemList_.clear();
			return true;
		}

		virtual void onCreate() override;

		virtual void onInit() override;

	};
}
