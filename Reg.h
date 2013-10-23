#ifndef Reg_h__2013_10_12__9_06
#define Reg_h__2013_10_12__9_06

namespace df
{
	struct RegKeyValue
	{
		CC key;
		DWORD valueDWORD;
		CC valueString;
		DWORD type;
	};

	class Reg
	{
	private:
		HKEY key_;
	public:
		enum
		{
			KeyBinary = REG_BINARY,
			KeyDWORD = REG_DWORD,
			KeyString = REG_SZ,
		};

		Reg(const sdf::CC & menu = tcc_("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), HKEY root = HKEY_CURRENT_USER, DWORD access = KEY_ALL_ACCESS)
		{
			if (RegOpenKeyEx(root, menu.char_, 0, access, &key_) != ERROR_SUCCESS)
			{
				key_ = 0;
				ERR(tcc_("RegOpenKeyEx failed.menu=") + menu + tcc_(" root=") + (intptr_t)HKEY_CURRENT_USER + tcc_(" access=") + access)
			}
		}

		~Reg()
		{
			RegCloseKey(key_);
		}

		bool SetValue(const sdf::CC & key, DWORD val)
		{
			if (RegSetValueEx(key_, key.char_, 0, REG_DWORD, (const BYTE*)&val, 4) != ERROR_SUCCESS)
			{
				ERR(tcc_("DWORD RegSetValueEx failed.key=") + key + tcc_(" val=") + val);
				return false;
			}
			return true;
		}

		bool SetValue(const sdf::CC & key, const sdf::CC & val)
		{
			if (RegSetValueEx(key_, key.char_, 0, REG_SZ, (const BYTE*)val.char_, val.length_) != ERROR_SUCCESS)
			{
				ERR(tcc_("String RegSetValueEx failed.key=") + key + tcc_(" val=") + val);
				return false;
			}
			return true;
		}

		bool DeleteValue(const sdf::CC & keyValue)
		{
			if (RegDeleteValue(key_, keyValue.char_) != ERROR_SUCCESS)
			{
				ERR(tcc_("DeleteKey failed.key=") + keyValue);
				return false;
			}
			return true;
		}

		int EachKey()
		{
			int count;
			return count;
		}

		template<class LamT>
		int EachKeyValue(LamT lam)
 		{
			DWORD dwIndex = 0,NameCnt,NameMaxLen,Type;

			DWORD KeyCnt,KeyMaxLen,MaxDateLen;

			if (RegQueryInfoKey(key_,NULL,NULL,NULL,&KeyCnt,&KeyMaxLen,NULL,&NameCnt,&NameMaxLen,&MaxDateLen,NULL,NULL) != ERROR_SUCCESS)
			{
				ERR(tcc_("RegQueryInfoKey failed"));
				return 0;
			}
			DWORD DateSize = MaxDateLen + 1;
			DWORD NameSize = NameMaxLen + 1;

			SS name(NameSize);
			SS data(DateSize);
			RegKeyValue regVal;
			for (dwIndex = 0; dwIndex < NameCnt; dwIndex++)    //枚举键值
			{
				DateSize = MaxDateLen + 1;
				NameSize = NameMaxLen + 1;
				RegEnumValue(key_, dwIndex, name.GetBuffer(), &NameSize, NULL, &Type,(LPBYTE) data.GetBuffer(), &DateSize);//读取键值
				name.strLength_ = NameSize;
				data.strLength_ = DateSize;
				regVal.key = name;
				regVal.type = Type;
				if (Type == REG_SZ)
				{
					regVal.valueDWORD = 0;
					regVal.valueString = data;
					if (!df::ExecuteFunctor(lam, std::ref(regVal)))
						break;
				}
				else if (Type == REG_DWORD)
				{
					regVal.valueString = tcc_("");
					regVal.valueDWORD = *(DWORD*)data.GetBuffer();
					if (!df::ExecuteFunctor(lam, std::ref(regVal)))
						break;
				}

			}



			return dwIndex;
		}
	};
}

#endif // Reg_h__2013_10_12__9_06
