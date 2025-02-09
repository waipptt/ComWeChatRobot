#include "pch.h"

// 联系人相关库偏移
#define SqlHandleMicroMsgOffset 0x2363934
// 公众号相关库偏移
#define SqlHandlePublicMsgOffset 0x239B3C8

// 保存数据库信息的容器
vector<DbInfoStruct> dbs;

/*
* 根据数据库名从`dbs`中检索数据库句柄
* dbname：数据库名
* return：DWORD，如果检索成功，返回数据库句柄，否则返回`0`
*/
DWORD GetDbHandleByDbName(wchar_t* dbname) {
	if (dbs.size() == 0)
		GetDbHandles();
	for (unsigned int i = 0; i < dbs.size() - 1; i++) {
		if (!lstrcmpW(dbs[i].dbname, dbname))
			return dbs[i].handle;
	}
	return 0;
}

/*
* 供外部调用的获取数据库信息接口
* return：DWORD，`dbs`首个成员地址
*/
DWORD GetDbHandlesRemote() {
	if (dbs.size() == 0)
		GetDbHandles();
	return (DWORD)dbs.data() ;
}

/*
* 获取数据库信息的具体实现
* return：void
*/
void GetDbHandles() {
	dbs.clear();
	DWORD WeChatWinBase = GetWeChatWinBase();
	DWORD SqlHandleBaseAddr = WeChatWinBase + SqlHandleMicroMsgOffset;
	DWORD SqlHandleBeginAddr = 0x0;
	DWORD SqlHandleEndAddr = 0x0;
	DWORD SqlHandlePublicMsgAddr = *(DWORD*)(WeChatWinBase + SqlHandlePublicMsgOffset);
	__asm {
		mov eax, [SqlHandleBaseAddr];
		mov ecx, [eax];
		add ecx, 0x1428;
		mov eax, [ecx];
		mov SqlHandleBeginAddr, eax;
		mov eax, [ecx + 0x4];
		mov SqlHandleEndAddr, eax;
	}
	DWORD dwHandle = 0x0;
	wstring dbnames = L"";
	while (SqlHandleBeginAddr < SqlHandleEndAddr) {
		dwHandle = *(DWORD*)SqlHandleBeginAddr;
		SqlHandleBeginAddr += 0x4;
		if (SqlHandleBeginAddr == SqlHandleEndAddr)
			break;
		if(dbnames.find((wchar_t*)(*(DWORD*)(dwHandle + 0x50)),0) != wstring::npos)
			continue;
		DbInfoStruct db = { 0 };
		dbnames += (wchar_t*)(*(DWORD*)(dwHandle + 0x50));
		db.dbname = (wchar_t*)(*(DWORD*)(dwHandle + 0x50));
		db.l_dbname = wcslen(db.dbname);
		db.handle = *(DWORD*)(dwHandle + 0x3C);
		ExecuteSQL(*(DWORD*)(dwHandle + 0x3C), "select * from sqlite_master where type=\"table\";",(DWORD)GetDbInfo,&db);
		dbs.push_back(db);
	}
	for (int i = 1; i < 4; i++) {
		dwHandle = *((DWORD*)(SqlHandlePublicMsgAddr + i * 0x4));
		if (dbnames.find((wchar_t*)(*(DWORD*)(dwHandle + 0x50)), 0) != wstring::npos)
			continue;
		DbInfoStruct db = { 0 };
		dbnames += (wchar_t*)(*(DWORD*)(dwHandle + 0x50));
		db.dbname = (wchar_t*)(*(DWORD*)(dwHandle + 0x50));
		db.l_dbname = wcslen(db.dbname);
		db.handle = *(DWORD*)(dwHandle + 0x3C);
		ExecuteSQL(*(DWORD*)(dwHandle + 0x3C), "select * from sqlite_master where type=\"table\";", (DWORD)GetDbInfo, &db);
		dbs.push_back(db);
	}
	// 添加一个空结构体，作为读取结束标志
	DbInfoStruct db_end = { 0 };
	dbs.push_back(db_end);
#ifdef _DEBUG
	for (unsigned int i = 0; i < dbs.size() - 1; i++) {
		printf("dbname = %ws,handle = 0x%08X,table_count:%d\n",dbs[i].dbname,dbs[i].handle,dbs[i].tables.size());
		for (unsigned int j = 0; j < dbs[i].tables.size();j++) {
			cout << "name     = " << dbs[i].tables[j].name << endl;
			cout << "tbl_name = " << dbs[i].tables[j].tbl_name << endl;
			cout << "rootpage = " << dbs[i].tables[j].rootpage << endl;
			cout << "sql      = " << dbs[i].tables[j].sql << endl;
			cout << endl;
		}
		cout << endl;
	}
#endif
}