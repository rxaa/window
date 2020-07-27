#pragma once

namespace sdf {


	class TabButton
		: public CheckBox {

	protected:
		std::shared_ptr<View> contentView;
	public:
        friend class TabBar;
        std::function<sdf::View* ()> view;

        TabButton() :CheckBox(false, true) {
            v.pos.flexX = 1;
        }

		std::shared_ptr<View> getContentView() {
			if (!contentView)
				contentView = std::shared_ptr<View>(view());
			return contentView;
		}


	};

#define ui_tab_button ui_control(sdf::TabButton) 



	class TabView
		: public View {
	protected:
		View* content_ = nullptr;
	public:
		friend class TabBar;
		TabView() {
			v.pos.vertical = true;
		}

		virtual ~TabView() {
			//COUT(tt_("gone"));
		}

		//Placement tabView content
		sdf::View * content() {
			auto ptr = std::make_shared<sdf::View>();
			ptr->pos.flex(1);
			content_ = ptr.get();
			v._addSub(ptr);
			return ptr.get();
		}

	};


#define ui_tab_view ui_control(sdf::TabView) 


	class TabBar
		: public CheckGroup {
	protected:
		TabView* tabView_ = nullptr;
	public:

		TabBar() {
			v.pos.flexX = 1;
			v.pos.vertical = false;
		}

		virtual void doCreate() override;
	};
}


#define ui_tab_bar ui_control(sdf::TabBar) 

