#include "pch.h"

// 发送文本消息的CALL偏移
#define SendTextCallOffset 0x67391D30 - 0x66E70000
// 清空缓存的CALL偏移
#define DeleteTextCacheCallOffset 0x54327720 - 0x54270000

/*
* 外部调用时传递的参数结构
* wxid：wxid保存地址
* wxmsg：发送的内容保存地址
*/
struct SendTextStruct
{
    DWORD wxid;
    DWORD wxmsg;
};

/*
* 供外部调用的发送文本消息接口
* lpParameter：SendTextStruct类型结构体指针
* return：void
*/
void SendTextRemote(LPVOID lpParameter) {
    SendTextStruct* rp = (SendTextStruct*)lpParameter;
    wchar_t* wsWxId = (WCHAR*)rp->wxid;
    wchar_t* wsTextMsg = (WCHAR*)rp->wxmsg;
    SendText(wsWxId, wsTextMsg);
}

/*
* 发送文本消息的具体实现
* wsWxId：接收人wxid
* wsTextMsg：发送的消息内容
* return：void
*/
void __stdcall SendText(wchar_t* wsWxId, wchar_t* wsTextMsg) {
    WxBaseStruct wxWxid(wsWxId);
    WxBaseStruct wxTextMsg(wsTextMsg);
    wchar_t** pWxmsg = &wxTextMsg.buffer;
    char buffer[0x3B0] = { 0 };

    WxString wxNull = { 0 };
    DWORD dllBaseAddress = GetWeChatWinBase();
    DWORD callAddress = dllBaseAddress + SendTextCallOffset;
    DWORD DeleteTextCacheCall = dllBaseAddress + DeleteTextCacheCallOffset;

    __asm {
        pushad;
        lea eax, wxNull;
        push 0x1;
        push eax;
        mov edi, pWxmsg;
        push edi;
        lea edx, wxWxid;
        lea ecx, buffer;
        call callAddress;
        add esp, 0xC;
        lea ecx, buffer;
        call DeleteTextCacheCall;
        popad;
    }
}