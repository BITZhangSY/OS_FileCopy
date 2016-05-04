#pragma once

#include<Windows.h>
#include<windowsx.h>

#include<tchar.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>

#define chDIMOF(Array)	(sizeof(Array) / sizeof(Array[0]))


typedef struct {
	int nDepth;					//Nesting depth
	BOOL fRecurse;				//Set to TRUE to list subdirectories
	TCHAR szBuf[1000];			//Output formatting buffer
	int nIndent;				//Indentation character count
	BOOL fOk;					//Loop control flag
	BOOL fIsDir;				//Loop control flag
	WIN32_FIND_DATA FindData;	//Information about the file that is found by the FindFirstFile()...
}DIRWALKDATA, *LPDIRWALKDATA;	//define struct and pointer

static BOOL IsChildDir(WIN32_FIND_DATA*);
static BOOL FindNextChildDir(HANDLE, WIN32_FIND_DATA*);
static HANDLE FindFirstChildDir(LPTSTR, WIN32_FIND_DATA*);
static void DirWalkRecurse(LPDIRWALKDATA, LPCTSTR, LPCTSTR);
void DirWalk(LPCTSTR, LPCTSTR, BOOL);



/*
	FindFirstFile()
	Searches a directory for a file or subdirectory with a name that matches a specific name (or partial name if wildcards are used).

	FindNextFile()
	Continues a file search from a previous call to the FindFirstFile, FindFirstFileEx, or FindFirstFileTransacted functions.

	FindClose();	
	Closes a file search handle opened by the FindFirstFile...

	WIN32_FIND_DATA structure
	Contains information about the file that is found by the FindFirstFile, FindFirstFileEx, or FindNextFile function.
	
	typedef struct _WIN32_FIND_DATA {
	DWORD    dwFileAttributes;
	FILETIME ftCreationTime;
	FILETIME ftLastAccessTime;
	FILETIME ftLastWriteTime;
	DWORD    nFileSizeHigh;
	DWORD    nFileSizeLow;
	DWORD    dwReserved0;
	DWORD    dwReserved1;
	TCHAR    cFileName[MAX_PATH];
	TCHAR    cAlternateFileName[14];
	} WIN32_FIND_DATA, *PWIN32_FIND_DATA, *LPWIN32_FIND_DATA;



*/
