#include "pch.h"
#include <vector>

// 保存个人信息的字符串
wstring selfinfo = L"";

/*
* 外部调用时的返回类型
* message：selfinfo.c_str()
* length：selfinfo字符串长度
*/
struct SelfInfoStruct {
	DWORD message;
	DWORD length;
} ret;

/*
* 供外部调用的获取个人信息接口
* return：DWORD，ret的首地址
*/
DWORD GetSelfInfoRemote() {
	DWORD WeChatWinBase = GetWeChatWinBase();
	vector<DWORD> SelfInfoAddr = {
		WeChatWinBase + 0x236307C,
		WeChatWinBase + 0x2363548,
		WeChatWinBase + 0x23630F4,
		*(DWORD*)(WeChatWinBase + 0x236322C),
		*(DWORD*)(WeChatWinBase + 0x239E11C),
		*(DWORD*)(WeChatWinBase + 0x23633D4),
		WeChatWinBase + 0x23632E8,
		WeChatWinBase + 0x23631FC,
		WeChatWinBase + 0x2363214,
		WeChatWinBase + 0x2363128
	};

	vector<wstring> SelfInfoKey = {
		L"\"wxId\"",
		L"\"wxNumber\"",
		L"\"wxNickName\"",
		L"\"wxSignature\"",
		L"\"wxBigAvatar\"",
		L"\"wxSmallAvatar\"",
		L"\"wxNation\"",
		L"\"wxProvince\"",
		L"\"wxCity\"",
		L"\"PhoneNumber\""
	};
#ifdef _DEBUG
	wcout.imbue(locale("chs"));
#endif
	selfinfo = selfinfo + L"{";
	for (unsigned int i = 0; i < SelfInfoAddr.size(); i++) {
		selfinfo = selfinfo + SelfInfoKey[i] + L":";
		selfinfo = selfinfo + L"\"";
		char* temp = NULL;
		if (!SelfInfoKey[i].compare(L"\"wxNickName\"")) {
			if (*(DWORD*)(SelfInfoAddr[i] + 0x14) == 0xF) {
				temp = (*((DWORD*)SelfInfoAddr[i]) != 0) ? (char*)SelfInfoAddr[i] : (char*)"null";
			}
			else {
				temp = (*((DWORD*)SelfInfoAddr[i]) != 0) ? (char*)(*(DWORD*)SelfInfoAddr[i]) : (char*)"null";
			}
		}
		else if (!SelfInfoKey[i].compare(L"\"wxId\"")) {
			char wxidbuffer[0x100] = { 0 };
			sprintf_s(wxidbuffer, "%s", (char*)SelfInfoAddr[i]);
			if (strlen(wxidbuffer) < 0x6 || strlen(wxidbuffer) > 0x14)
			{
				//新的微信号 微信ID用地址保存
				temp = (char*)(*(DWORD*)SelfInfoAddr[i]);
			}
			else
			{
				temp = (char*)SelfInfoAddr[i];
			}
		}
		else {
			temp = (char*)SelfInfoAddr[i];
			if (temp == NULL || strlen(temp) == 0)
				temp = (char*)"null";
		}
		wchar_t* wtemp = new wchar_t[strlen(temp) + 1];
		ZeroMemory(wtemp, (strlen(temp) + 1) * 2);
		MultiByteToWideChar(CP_UTF8, 0, temp, -1, wtemp, strlen(temp) + 1);
		wstring wrtemp = wreplace(wtemp,L'\"',L"\\\"");
		selfinfo = selfinfo + wrtemp;
		selfinfo = selfinfo + L"\"";
		if(i!= SelfInfoAddr.size() - 1)
			selfinfo = selfinfo + L",";
		delete[] wtemp;
		wtemp = NULL;
	}
	selfinfo = selfinfo + L"}";
	ret.message = (DWORD)selfinfo.c_str();
	ret.length = selfinfo.length();
#ifdef _DEBUG
	wcout << selfinfo << endl;
#endif
	return (DWORD)&ret;
}

/*
* 删除个人信息缓存
* return：void
*/
VOID DeleteSelfInfoCacheRemote() {
	if (ret.length) {
		ZeroMemory((wchar_t*)ret.message, ret.length*2 + 2);
		ret.length = 0;
		selfinfo = L"";
	}
}