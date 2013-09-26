#pragma once

namespace sdf
{
	class Window;

	typedef df::IntoPtr<Window> WinPtr;

	class Window :
		public Control
	{
	protected:
		PTR_REF_COUNT;
		bool isModal_;
		std::function<void()> onClose_;
		Gdi gdi_;
		static Bitmap backGround_;
		static Brush backBrush_;
		static int mouseX_ , mouseY_;

		std::map<int,std::function<void()>> commandMap_;
	public:

		Window(void);
		virtual ~Window(void);

		//��ȡ��������
		inline static HFONT GetFont(HWND handle)
		{
			return (HFONT)::SendMessage(handle , WM_GETFONT, 0, 0);
		}
		//���ô���ͼ��
		inline static void SetIcon(HWND h , int id)
		{
			::SendMessage(h, WM_SETICON, TRUE, (LPARAM)LoadIcon(df::Global::progressInstance_ , MAKEINTRESOURCE(id)) );
		}

		inline void SetIcon(int id)
		{
			::SendMessage(handle_, WM_SETICON, TRUE, (LPARAM)LoadIcon(df::Global::progressInstance_ , MAKEINTRESOURCE(id)) );
		}
		inline static void GetMousePos( LPARAM lParam )
		{
			mouseX_=LOWORD (lParam);
			mouseY_=HIWORD (lParam);
		}

		//��ʼ������ͼƬ
		static void InitImage(int backGround);

		///������,��ʾ��ģ̬�Ի���,parent:�����ھ��,
		void Open(HWND parent = NULL, bool show = true);
		void Open(Control & parent, bool show = true)
		{
			Open(parent.GetHandle(), show);
		}
		//����,��ʾģ̬�Ի���
		void OpenModal(HWND parent);
		void OpenModal(Control & parent)
		{
			OpenModal(parent.GetHandle());
		}
		void OpenModal(Control * parent)
		{
			MY_ASSERT(parent!=nullptr);
			OpenModal(parent->GetHandle());
		}
		//�رմ���
		void Close(bool exit = false);

		//�жϴ����Ƿ��Ѿ��ر�
		bool IsClosed()
		{
			return handle_ == 0;
		}

		//��ʾ����,����ʼ��Ϣѭ��
		void Run(bool show = true);

		template<class T>
		void AddEvent(int id, T cb)
		{
			commandMap_[id]=cb;
		}

		static int GetScreenWidth()
		{
			return ::GetSystemMetrics(SM_CXSCREEN);
		}

		static int GetScreenHeight()
		{
			return ::GetSystemMetrics(SM_CYSCREEN);
		}

		static void PopMessage(const CC & msg,int time=1200);
		static LRESULT  __stdcall PopMessageProc(HWND hDlg, uint message, WPARAM wParam, LPARAM lParam);

		//////////////////////////////////�¼�////////////////////////////////////////
		virtual void OnInit();
		virtual HBRUSH OnDrawBackground();
		virtual void OnPaint();
		//����Ϊture :���� false ̧��
		virtual void OnMouseLeft(bool ){}
		virtual void OnMouseRight(bool ){}

		//��Ϣѭ��
		static void MessageLoop();
	private:
		void InitWinData()
		{
			gdi_.Init(handle_);
			try
			{
				OnInit();
			}CATCH_SEH;
		}

		void Release();

	
		//ģ̬��������Ϣ����
		static intptr_t __stdcall ModalProc(HWND hDlg, uint message, WPARAM wParam, LPARAM lParam);
		//ģ̬�����ģ̬������
		static intptr_t __stdcall WndProc(HWND hDlg, uint message, WPARAM wParam, LPARAM lParam);
	};


}


