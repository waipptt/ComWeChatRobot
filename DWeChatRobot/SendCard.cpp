#include "pch.h"

// 发送名片的CALL偏移
#define SendCardCallOffset 0x54791D30 - 0x54270000
// 清空缓存的CALL偏移
#define DeleteCardCacheCallOffset 0x54327720 - 0x54270000

/*
* 外部调用时提供的参数结构
* receiver：名片消息接收人wxid保存地址
* sharedwxid：被推荐人的wxid保存地址
* nickname：名片显示的昵称保存地址
*/
struct SendCardStruct {
	DWORD receiver;
	DWORD sharedwxid;
	DWORD nickname;
};

/*
* 供外部调用的发送名片接口
* lparameter：SendCardStruct类型结构体指针
* return：void
*/
VOID SendCardRemote(LPVOID lparameter) {
	SendCardStruct* scs = (SendCardStruct*)lparameter;
	wchar_t* receiver = (WCHAR*)scs->receiver;
	wchar_t* sharedwxid = (WCHAR*)scs->sharedwxid;
	wchar_t* nickname = (WCHAR*)scs->nickname;
	SendCard(receiver,sharedwxid,nickname);
}

/*
* 发送名片消息的具体实现
* receiver：消息接收人wxid
* sharedwxid：被推荐人wxid
* nickname：名片显示的昵称
* return：BOOL，发送成功返回`0`，发送失败返回`1`
*/
BOOL __stdcall SendCard(wchar_t* receiver, wchar_t* sharedwxid, wchar_t* nickname) {
	DWORD WeChatWinBase = GetWeChatWinBase();
	DWORD SendCardCall = WeChatWinBase + SendCardCallOffset;
	DWORD DeleteCardCacheCall = WeChatWinBase + DeleteCardCacheCallOffset;
	wchar_t* xml = new wchar_t[0x2000];
	ZeroMemory(xml, 0x2000 * 2);
	swprintf_s(xml, 0x2000,L"<?xml version=\"1.0\"?><msg bigheadimgurl=\"\" smallheadimgurl=\"\" username=\"%ws\" nickname=\"%ws\" fullpy=\"?\" shortpy=\"\" alias=\"%ws\" imagestatus=\"3\" scene=\"17\" province=\"北京\" city=\"中国\" sign=\"\" sex=\"2\" certflag=\"0\" certinfo=\"\" brandIconUrl=\"\" brandHomeUrl=\"\" brandSubscriptConfigUrl= \"\" brandFlags=\"0\" regionCode=\"CN_BeiJing_BeiJing\" />", 
		sharedwxid, nickname, sharedwxid);
	WxBaseStruct pReceiver(receiver);
	WxBaseStruct pXml(xml);
	char buffer[0x2D0] = { 0 };
	DWORD isSuccess = 0x1;

	__asm {
		pushad;
		push 0x2A;
		lea eax, pXml;
		lea edx, pReceiver;
		push 0x0;
		push eax;
		lea ecx, buffer;
		call SendCardCall;
		add esp, 0xC;
		lea ecx, buffer;
		call DeleteCardCacheCall;
		mov isSuccess, eax;
		popad;
	}
	delete[] xml;
	xml = NULL;
	return isSuccess;
}