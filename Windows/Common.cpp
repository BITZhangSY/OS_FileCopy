#include"Common.h"

//判断是否是一个目录
static BOOL IsChildDir(WIN32_FIND_DATA* lpFindData)		//static func can only be called by func in the same file
{
	return ((lpFindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) &&		//is a directory
			(lstrcmp(lpFindData->cFileName, __TEXT(".")) != 0) &&					//not current dir
			(lstrcmp(lpFindData->cFileName, __TEXT("..")) != 0);					//not parent dir
}

//查找下一个子目录
static BOOL FindNextChildDir(HANDLE hFindFile, WIN32_FIND_DATA* lpFindData)
{
	BOOL fFound = FALSE;

	do
	{
		fFound = FindNextFile(hFindFile, lpFindData);
	} while (fFound && !IsChildDir(lpFindData));

	return fFound;
}

//找到第一个子目录
static HANDLE FindFirstChildDir(LPTSTR szPath, WIN32_FIND_DATA* lpFindData)
{
	BOOL fFound;
	HANDLE hFindFile = FindFirstFile(szPath, lpFindData);						//system API

	if (hFindFile != INVALID_HANDLE_VALUE)
	{
		fFound = IsChildDir(lpFindData);

		if (!fFound)
		{
			fFound = FindNextChildDir(hFindFile, lpFindData);					//system API
		}

		if (!fFound)
		{
			FindClose(hFindFile);												//system API
			hFindFile = INVALID_HANDLE_VALUE;
		}
	}
	return hFindFile;
}

static void DirWalkRecurse(LPDIRWALKDATA pDW, LPCTSTR pszRootPath, LPCTSTR pszDesPath)	//pointer to DIRWALKDATA as parameter, so the param in the struct can be constantly updated such as nDepth
{
	HANDLE hFind;						//FindFirstFile()返回的句柄  作为FindNextFile()的参数，多次查找的关联

	LPTSTR src = (LPTSTR)malloc(100 * sizeof(LPTSTR));
	LPTSTR des = (LPTSTR)malloc(100 * sizeof(LPTSTR));

	pDW->nDepth++;						//recursive depth + 1
	pDW->nIndent = 3 * pDW->nDepth;		//控制缓冲区写入位置
	_stprintf_s(pDW->szBuf, _TEXT("%*s"), pDW->nIndent, _TEXT(""));

	GetCurrentDirectory(chDIMOF(pDW->szBuf) - pDW->nIndent, &pDW->szBuf[pDW->nIndent]);		//将当前目录存入缓冲区

	//printf("%s\n", pDW->szBuf);

	hFind = FindFirstFile(_TEXT("*.*"), &pDW->FindData);						//system API
	pDW->fOk = (hFind != INVALID_HANDLE_VALUE);

	while (pDW->fOk)		//当前目录搜索完毕则跳出
	{
		pDW->fIsDir = pDW->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;	//是否是目录
		if (!pDW->fIsDir || (!pDW->fRecurse && IsChildDir(&pDW->FindData)))			//为文件非目录 || (非递归 && 为一个目录文件)
		{
			_stprintf_s(pDW->szBuf,
				pDW->fIsDir ? _TEXT("%*s[%s]") : _TEXT("%*s%s"),
				pDW->nIndent,
				_TEXT(""),
				pDW->FindData.cFileName);

			//封装路径
			lstrcpy(src, pszRootPath);
			lstrcat(src, "\\");
			lstrcat(src, pDW->FindData.cFileName);

			lstrcpy(des, pszDesPath);
			lstrcat(des, "\\");
			lstrcat(des, pDW->FindData.cFileName);

			//创建/打开文件
			HANDLE desFile = CreateFile(
				des,
				GENERIC_READ | GENERIC_WRITE,
				0,
				NULL,
				OPEN_ALWAYS,
				pDW->FindData.dwFileAttributes,
				NULL);
			HANDLE srcFile = CreateFile(
				src,
				GENERIC_READ | GENERIC_WRITE,
				0,
				NULL,
				OPEN_EXISTING,
				pDW->FindData.dwFileAttributes,
				NULL);

			DWORD filesize = GetFileSize(srcFile, NULL);
			char* buffer = new char[filesize + 1];
			DWORD readsize;

			ReadFile(srcFile, buffer, filesize, &readsize, NULL);
			WriteFile(desFile, buffer, filesize, &readsize, NULL);
			buffer[filesize] = 0;
			CloseHandle(srcFile);
			CloseHandle(desFile);

			//printf("%s\n", pDW->szBuf);
		}
		else {
			SECURITY_ATTRIBUTES attribute;
			attribute.nLength = sizeof(attribute);
			attribute.lpSecurityDescriptor = NULL;
			attribute.bInheritHandle = FALSE;
			SetCurrentDirectory(pszDesPath);
			CreateDirectory(pDW->FindData.cFileName, &attribute);
			SetCurrentDirectory(pszRootPath);
		}

		pDW->fOk = FindNextFile(hFind, &pDW->FindData);							//system API
	}

	if (hFind != INVALID_HANDLE_VALUE)
	{
		FindClose(hFind);	
	}

	if (pDW->fRecurse)
	{
		//Get the first child directory
		hFind = FindFirstChildDir(_TEXT("*.*"), &pDW->FindData);				//找到第一个子目录 FindFirstFile
		pDW->fOk = (hFind != INVALID_HANDLE_VALUE);
		
		while (pDW->fOk)
		{
			//change into the child directory
			if (SetCurrentDirectory(pDW->FindData.cFileName))
			{
				//封装路径
				lstrcpy(src, pszRootPath);
				lstrcat(src, "\\");
				lstrcat(src, pDW->FindData.cFileName);

				lstrcpy(des, pszDesPath);
				lstrcat(des, "\\");
				lstrcat(des, pDW->FindData.cFileName);
				DirWalkRecurse(pDW, src, des);

				//change back to the child's parent dir
				SetCurrentDirectory(_TEXT(".."));
			}

			pDW->fOk = FindNextChildDir(hFind, &pDW->FindData);					//将hFind句柄传入 FindNextFile
		}

		if (hFind != INVALID_HANDLE_VALUE)
		{
			FindClose(hFind);
		}
	}

	pDW->nDepth--;
}

void DirWalk(LPCTSTR pszRootPath, LPCTSTR pszDesPATH, BOOL fRecurse)
{
	TCHAR szCurrDir[_MAX_DIR];
	DIRWALKDATA DW;

	//save the current dir so that it can be restored later
	GetCurrentDirectory(chDIMOF(szCurrDir), szCurrDir);

	//set the current dir to where we want to start walking
	SetCurrentDirectory(pszRootPath);

	//nDpeth is used to control indenting(缩进). the value -1 will cause the first level to display flush left
	DW.nDepth = -1;

	DW.fRecurse = fRecurse;

	//call DirWalkRecurse() to do the real work
	DirWalkRecurse(&DW, pszRootPath, pszDesPATH);

	//restore the current directory to what it was before DirWalkRecurse() was called
	SetCurrentDirectory(szCurrDir);
}
