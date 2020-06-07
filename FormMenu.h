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
    public:
        FormMenu() {
        }

        ~FormMenu() {
            COUT(tt_("FormMenu gone"));
        }

        static std::shared_ptr<FormMenu> create() {
            return std::make_shared<FormMenu>();
        }


        virtual void onInActive() override {
            close();
        }

        void addItem(const MenuItem &item) {
            itemList_.push_back(item);
        }

        virtual void onCreate() override;
       
    };
}
