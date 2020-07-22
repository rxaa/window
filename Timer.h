#pragma once

namespace sdf {

	class TimerItem {
		DF_DISABLE_COPY_ASSIGN(TimerItem);
	public:
		HWND handle_ = 0;
		std::function<void()> func_;


		TimerItem(TimerItem&& t) :handle_(t.handle_), func_(std::move(t.func_)) {

		}

		TimerItem(HWND handle, std::function<void()>&& func) :handle_(handle), func_(std::move(func)) {

		}
	};

	class Timer {

		DF_DISABLE_COPY_ASSIGN(Timer);

	public:
		static std::unordered_map<uint32_t, TimerItem> timerMap;

		uint32_t id = 0;

		static uint32_t& idCount() {
			static uint32_t count = 38171;
			return count;
		}
		Timer() {
		}

		Timer(uint32_t i) :id(i) {

		}

		Timer(Timer&& t) :id(t.id) {
			t.id = 0;
		}

		inline operator bool() const
		{
			return id != 0;
		}

		inline Timer& operator = (Timer&& wf)
		{
			id = wf.id;
			wf.id = 0;
			return *this;
		}

		static void onTimer(uint32_t id);

		static Timer set(Control* con, uint32_t time, std::function<void()>&& func);

		void reset();

		~Timer() {
			reset();
		}
	};
}
