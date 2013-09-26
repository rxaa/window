#ifndef Tray_h__2013_9_2__18_04
#define Tray_h__2013_9_2__18_04


#include <ShellAPI.h>
namespace sdf
{
	///����
	class Tray
	{
	public:
		enum
		{
			TRAY_MESSAGE = 51010
		};
		Tray(void)
		{
			m_tnd.uID=0;
		}

		~Tray(void)
		{
			RemoveIcon();
		}
		NOTIFYICONDATA  m_tnd;

		static std::function<void()> & OnLeftClick();

		static std::function<void()> & OnRightClick();

		//����������֪ͨ,(֪ͨ����,ͼ����Դ��ʶ��,��������,����ͣ��ʱ��)
		BOOL Init(const CC & szToolTip, uint icon, const CC & szInfo = cct_("����������!"), int uTimeout = 2000)
		{
			HWND hParent=Control::currentHandle_;

			HICON iconp=::LoadIcon(df::Global::progressInstance_, MAKEINTRESOURCE(icon));


			// ��� NOTIFYICONDATA �ṹ

			m_tnd.cbSize = sizeof(NOTIFYICONDATA);
			m_tnd.hWnd   = hParent;
			m_tnd.uID    = icon;
			m_tnd.hIcon  = iconp;
			m_tnd.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
			m_tnd.uCallbackMessage = TRAY_MESSAGE;
			Sprintf(m_tnd.szTip,128,_T("%s"), szToolTip.char_);

			//����
			if(szInfo[0])
			{
				m_tnd.uFlags |= NIF_INFO;
				Sprintf(m_tnd.szInfo,128,_T("%s"),szInfo.char_);
				Sprintf(m_tnd.szInfoTitle,64,_T("%s"), szToolTip.char_);
				m_tnd.uTimeout=uTimeout;
				m_tnd.dwInfoFlags=1;
			}

			return Shell_NotifyIcon(NIM_ADD, &m_tnd);
		}

		//�Ƴ�ͼ��
		void RemoveIcon()
		{
			if(m_tnd.uID)
			{
				m_tnd.uFlags = 0;

				Shell_NotifyIcon(NIM_DELETE, &m_tnd);
			}
		}

	private:
		Tray(const Tray& ) ;
		Tray& operator = (const Tray& ) ;
	};

}

#endif // Tray_h__2013_9_2__18_04
