#include"Common.h"

//�ж��Ƿ���һ��Ŀ¼
static BOOL IsChildDir(WIN32_FIND_DATA* lpFindData)		//static func can only be called by func in the same file
{
	return ((lpFindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) &&		//is a directory
			(lstrcmp(lpFindData->cFileName, __TEXT(".")) != 0) &&					//not current dir
			(lstrcmp(lpFindData->cFileName, __TEXT("..")) != 0);					//not parent dir
}

//������һ����Ŀ¼
static BOOL FindNextChildDir(HANDLE hFindFile, WIN32_FIND_DATA* lpFindData)
{
	BOOL fFound = FALSE;

	do
	{
		fFound = FindNextFile(hFindFile, lpFindData);
	} while (fFound && !IsChildDir(lpFindData));

	return fFound;
}

//�ҵ���һ����Ŀ¼
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
	HANDLE hFind;						//FindFirstFile()���صľ��  ��ΪFindNextFile()�Ĳ�������β��ҵĹ���

	LPTSTR src = (LPTSTR)malloc(100 * sizeof(LPTSTR));
	LPTSTR des = (LPTSTR)malloc(100 * sizeof(LPTSTR));

	pDW->nDepth++;						//recursive depth + 1
	pDW->nIndent = 3 * pDW->nDepth;		//���ƻ�����д��λ��
	_stprintf_s(pDW->szBuf, _TEXT("%*s"), pDW->nIndent, _TEXT(""));

	GetCurrentDirectory(chDIMOF(pDW->szBuf) - pDW->nIndent, &pDW->szBuf[pDW->nIndent]);		//����ǰĿ¼���뻺����

	//printf("%s\n", pDW->szBuf);

	hFind = FindFirstFile(_TEXT("*.*"), &pDW->FindData);						//system API
	pDW->fOk = (hFind != INVALID_HANDLE_VALUE);

	while (pDW->fOk)		//��ǰĿ¼�������������
	{
		pDW->fIsDir = pDW->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;	//�Ƿ���Ŀ¼
		if (!pDW->fIsDir || (!pDW->fRecurse && IsChildDir(&pDW->FindData)))			//Ϊ�ļ���Ŀ¼ || (�ǵݹ� && Ϊһ��Ŀ¼�ļ�)
		{
			_stprintf_s(pDW->szBuf,
				pDW->fIsDir ? _TEXT("%*s[%s]") : _TEXT("%*s%s"),
				pDW->nIndent,
				_TEXT(""),
				pDW->FindData.cFileName);

			//��װ·��
			lstrcpy(src, pszRootPath);
			lstrcat(src, "\\");
			lstrcat(src, pDW->FindData.cFileName);

			lstrcpy(des, pszDesPath);
			lstrcat(des, "\\");
			lstrcat(des, pDW->FindData.cFileName);

			//����/���ļ�
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
		hFind = FindFirstChildDir(_TEXT("*.*"), &pDW->FindData);				//�ҵ���һ����Ŀ¼ FindFirstFile
		pDW->fOk = (hFind != INVALID_HANDLE_VALUE);
		
		while (pDW->fOk)
		{
			//change into the child directory
			if (SetCurrentDirectory(pDW->FindData.cFileName))
			{
				//��װ·��
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

			pDW->fOk = FindNextChildDir(hFind, &pDW->FindData);					//��hFind������� FindNextFile
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

	//nDpeth is used to control indenting(����). the value -1 will cause the first level to display flush left
	DW.nDepth = -1;

	DW.fRecurse = fRecurse;

	//call DirWalkRecurse() to do the real work
	DirWalkRecurse(&DW, pszRootPath, pszDesPATH);

	//restore the current directory to what it was before DirWalkRecurse() was called
	SetCurrentDirectory(szCurrDir);
}
