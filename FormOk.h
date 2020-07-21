#pragma once

namespace sdf {

	class FormOk : public sdf::Window {
	protected:
		
		String content;
	public:
		std::function<void()> onOk_;
		FormOk(df::CC t) {
			content = t.ToString();
		}
		FormOk(df::CC t,df::CC title) {
			text = title.ToString();
			content = t.ToString();
		}

		FormOk(df::CC t,const std::function<void()>& onClick) {
			content = t.ToString();
			onOk_ = onClick;
		}

		~FormOk() {
		}

		virtual bool onClose(int code) override {
			onOk_ = nullptr;
			return true;
		}

		template < class ...Args>
		static std::shared_ptr<FormOk> create(Args&& ... rest) {
			return std::make_shared<FormOk>(rest...);
		}

		virtual void onCreate() override;

		virtual void onInit() override;

	};
}
