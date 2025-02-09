#include "pch.h"

// 发送艾特消息CALL偏移
#define SendAtTextCallOffset 0x67391D30 - 0x66E70000
// 清空缓存CALL偏移
#define DeleteAtTextCacheCallOffset 0x54327720 - 0x54270000

/*
* 外部调用时传递的参数结构
* chatroomid：群聊ID的保存地址
* wxidlist：艾特列表的保存地址，真实类型应当是`wchar_t**`
* wxmsg：发送的内容保存地址
* length：艾特的人数量，用于指示wxidlist长度
*/
struct SendAtTextStruct
{
    DWORD chatroomid;
    DWORD wxidlist;
    DWORD wxmsg;
    DWORD length;
};

/*
* 内存中使用的参数结构
* 构造与Release版本vector动态数组相仿
* 成员类型：`WxString`
* AtUser：类似`vector`的`data`方法，保存数组首个成员的地址
* addr_end1：数组尾地址
* addr_end2：数组尾地址
*/
struct AtStruct {
    DWORD AtUser;
    DWORD addr_end1;
    DWORD addr_end2;
};

/*
* 供外部调用的发送艾特消息接口
* lpParameter：SendAtTextStruct类型结构体指针
* return：void
*/
void SendAtTextRemote(LPVOID lpParameter) {
    SendAtTextStruct* rp = (SendAtTextStruct*)lpParameter;
    wchar_t* wsChatRoomId = (WCHAR*)rp->chatroomid;
    wchar_t* wsTextMsg = (WCHAR*)rp->wxmsg;
    if (rp->length == 0)
        return;
    else if(rp->length == 1)
        SendAtText(wsChatRoomId, (DWORD*)&rp->wxidlist, wsTextMsg,rp->length);
    else
        SendAtText(wsChatRoomId, (DWORD*)rp->wxidlist, wsTextMsg, rp->length);
}

/*
* 发送艾特消息的具体实现
* wsChatRoomId：群聊ID
* wsWxId：艾特的人列表
* wsTextMsg：发送的消息内容
* length：艾特的人数量
* return：void
*/
void __stdcall SendAtText(wchar_t* wsChatRoomId, DWORD wsWxId[], wchar_t* wsTextMsg,int length) {
    // +1的作用是补充一个空结构体，将`AtStruct`尾地址设定为空结构的首地址即可
    WxString* AtUsers = new WxString[length + 1];
    wstring AtMessage = L"";
    int querySuccess = 0;
    for (int i = 0; i < length; i++) {
        wchar_t* nickname = NULL;
        if (!lstrcmpW((wchar_t*)wsWxId[i], (wchar_t*)L"notify@all")) {
            nickname = (wchar_t*)L"所有人";
        }
        else
            nickname = GetUserNickNameByWxId((wchar_t*)wsWxId[i]);
        if (!nickname)
            continue;
        WxString temp = { 0 };
        temp.buffer = (wchar_t*)wsWxId[i];
        temp.length = wcslen((wchar_t*)wsWxId[i]);
        temp.maxLength = wcslen((wchar_t*)wsWxId[i]) * 2;
        memcpy(&AtUsers[querySuccess],&temp,sizeof(WxString));
        AtMessage = AtMessage + L"@" + nickname + L" ";
        querySuccess++;
    }
    AtMessage += wsTextMsg;
    if (!querySuccess)
        return;
    WxBaseStruct wxChatRoomId(wsChatRoomId);
    WxBaseStruct wxTextMsg((wchar_t*)AtMessage.c_str());
    AtStruct at = { 0 };
    at.AtUser = (DWORD)AtUsers;
    at.addr_end1 = (DWORD)&AtUsers[querySuccess];
    at.addr_end2 = (DWORD)&AtUsers[querySuccess];

    wchar_t** pWxmsg = &wxTextMsg.buffer;
    char buffer[0x3B0] = { 0 };

    DWORD dllBaseAddress = GetWeChatWinBase();
    DWORD callAddress = dllBaseAddress + SendAtTextCallOffset;
    DWORD DeleteTextCacheCall = dllBaseAddress + DeleteAtTextCacheCallOffset;

    __asm {
        lea eax, at;
        push 0x1;
        push eax;
        mov edi, pWxmsg;
        push edi;
        lea edx, wxChatRoomId;
        lea ecx, buffer;
        call callAddress;
        add esp, 0xC;
        lea ecx, buffer;
        call DeleteTextCacheCall;
    }
    delete[] AtUsers;
    AtUsers = NULL;
}