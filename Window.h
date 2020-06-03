#pragma once

namespace sdf {
    class Window;

    typedef std::shared_ptr<Window> WinPtr;

    class Window :
            public Control {
    protected:

        //是否为模态话对话框
        bool isModal_;

        //关闭事件
        std::function<void()> onClose_;

        static Bitmap backGround_;
      
        Brush backBrush_;


        std::map<int, std::function<void()>> commandMap_;
    public:
        static int mouseX_, mouseY_;

        //构造
        Window(void);

        virtual ~Window(void);

        int32_t minHeight = 0;
        int32_t minWeight = 0;
        int32_t maxHeight = 0;
        int32_t maxWeight = 0;
        //是否有最大化按钮
        bool maxBox = true;
        bool minBox = true;
        //是否允许调整大小
        bool resizeAble = true;
        //初始为最大化
        bool initMaxSize = false;
        //初始为最小化
        bool initMixSize = false;

        Window &v;
        Gdi gdi_;
        //标题边框大小
        int16_t titleHeight_ = 0;

        //左边框大小
        int16_t borderSize_ = 0;


        int16_t GetTitleHeight() const { return titleHeight_; }

        int16_t GetBorderSize() const { return borderSize_; }

        //获取窗口字体
        inline static HFONT GetFont(HWND handle) {
            return (HFONT) ::SendMessage(handle, WM_GETFONT, 0, 0);
        }


        static Font &SetGlobalFont(Font &f) {
            GlobalFont().SetFont(f);
            Gdi::GetScreen().setFont(f);
        }

        static float getScale();

        static void scalePos(ControlPos &pos) {
            auto sca = getScale();
            pos.x = (int32_t) ((float) pos.x * sca);
            pos.y = (int32_t) ((float) pos.y * sca);
            if (pos.w > 0)
                pos.w = (int32_t) ((float) pos.w * sca);
            if (pos.h > 0)
                pos.h = (int32_t) ((float) pos.h * sca);

            pos.paddingLeft = (int32_t) ((float) pos.paddingLeft * sca);
            pos.paddingTop = (int32_t) ((float) pos.paddingTop * sca);
            pos.paddingRight = (int32_t) ((float) pos.paddingRight * sca);
            pos.paddingBottom = (int32_t) ((float) pos.paddingBottom * sca);

            pos.marginLeft = (int32_t) ((float) pos.marginLeft * sca);
            pos.marginTop = (int32_t) ((float) pos.marginTop * sca);
            pos.marginRight = (int32_t) ((float) pos.marginRight * sca);
            pos.marginBottom = (int32_t) ((float) pos.marginBottom * sca);
        }

        static void scaleStyle(ControlStyle &sty) {
            auto sca = getScale();
            sty.shadowSize = (int32_t) ((float) sty.shadowSize * sca);
            sty.borderTop = (int16_t) ((float) sty.borderTop * sca);
            sty.borderRight = (int16_t) ((float) sty.borderRight * sca);
            sty.borderLeft = (int16_t) ((float) sty.borderLeft * sca);
            sty.borderBottom = (int16_t) ((float) sty.borderBottom * sca);
        }

        inline HFONT GetFont() {
            return GetFont(handle_);
        }

        //设置窗口图标
        inline static void setIcon(HWND h, int id) {
            ::SendMessage(h, WM_SETICON, TRUE, (LPARAM) LoadIcon(Control::progInstance_, MAKEINTRESOURCE(id)));
        }

        inline void setIcon(int id) {
            ::SendMessage(handle_, WM_SETICON, TRUE, (LPARAM) LoadIcon(Control::progInstance_, MAKEINTRESOURCE(id)));
        }

        inline static void GetMousePos(LPARAM lParam) {
            mouseX_ = LOWORD(lParam);
            mouseY_ = HIWORD(lParam);
        }


        ///非阻塞,显示非模态对话框,parent:父窗口句柄,
        void open(HWND parent = NULL, bool show = true);

        void open(Control &parent, bool show = true) {
            open(parent.GetHandle(), show);
        }

        //阻塞,显示模态对话框
        void openModal(HWND parent);

        void openModal(Control &parent) {
            openModal(parent.GetHandle());
        }

        void OpenModal(Control *parent) {
            DF_ASSERT(parent != nullptr);
            openModal(parent->GetHandle());
        }

        //关闭窗口
        void close(bool exit = false);

        //判断窗口是否已经关闭
        bool isClosed() {
            return handle_ == 0;
        }

        //显示窗口,并开始消息循环
        void run(bool show = true);

        template<class T>
        void AddEvent(int id, T cb) {
            commandMap_[id] = cb;
        }


        /**
         * 设置定时器
         * 每隔time毫秒触发onTimer
         * @param id
         * @param time 毫秒时间
         */
        void setTimer(uint32_t id, uint32_t time) {
            ::SetTimer(handle_, id, time, 0);
        }

        void killTimer(uint32_t id) {
            ::KillTimer(handle_, id);
        }

        void AdjustLayout();

        static int GetScreenWidth() {
            return ::GetSystemMetrics(SM_CXSCREEN);
        }

        static int GetScreenHeight() {
            return ::GetSystemMetrics(SM_CYSCREEN);
        }

        static void PopMessage(const df::CC &msg, int time = 1500);

        static LRESULT  __stdcall PopMessageProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

        //////////////////////////////////事件////////////////////////////////////////



        virtual void onInit();

        virtual HBRUSH OnDrawBackground();

        virtual void onPaint();

        //参数为ture :按下 false 抬起
        virtual void onMouseLeft(bool) {}

        virtual void onMouseRight(bool) {}

        virtual void onKeyUp(int) {}

        virtual void onTimer(UINT) {}

        virtual void onResize() {

        }

        //布局更改事件
        virtual void onLayout() {

        }

        //窗口移动事件
        virtual void onMove() {

        }

        //消息循环
        static void MessageLoop();


        //*******************************************
        // Summary : 获取边框大小
        //*******************************************
        void UpdateBorderSize() {
            POINT pon;
            pon.x = pos.x;
            pon.y = pos.y;
            ::ScreenToClient(handle_, &pon);
            titleHeight_ = (int16_t) std::abs(pon.y);
            borderSize_ = (int16_t) std::abs(pon.x);
        }

    private:
        void InitWinData();

        void Release();


        //模态化窗口消息处理
        static intptr_t __stdcall ModalProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

        //模态化与非模态话共用
        static intptr_t __stdcall WndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


    DF_DISABLE_COPY_ASSIGN(Window);
    };


    inline int32_t dp2px(int32_t dv) {
        return (int32_t) (dv * Window::getScale());
    }

}


