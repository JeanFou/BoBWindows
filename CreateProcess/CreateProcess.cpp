// CreateProcess.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"


int main()
{
	LPTSTR szNotepad = _tcsdup(TEXT("notepad"));
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(STARTUPINFO));

	CreateProcess(NULL, szNotepad, NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi);
 
	ShellExecute(NULL, _tcsdup(TEXT("open")), szNotepad, NULL, NULL, SW_SHOW);
	return 0;
}

