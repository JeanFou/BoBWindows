// FileEncoding.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"
#include <Windows.h>
#include <stdint.h>
#include <strsafe.h>
#include <crtdbg.h>

#define MAX_LINE 1024

bool read_file_using_memory_map();
bool create_file(HANDLE file_handle);
bool copy_file(LPCWSTR input);
bool read_and_print();
bool read_file();

int _tmain(int argc, _TCHAR* argv[])
{
	read_file_using_memory_map();
	return 0;
}


bool read_file_using_memory_map()
{
	// current directory �� ���Ѵ�.
	wchar_t *buf = NULL;
	uint32_t buflen = 0;
	buflen = GetCurrentDirectoryW(buflen, buf);
	if (0 == buflen)
	{
		printf("err, GetCurrentDirectoryW() failed. gle = 0x%08x", GetLastError());
		return false;
	}

	buf = (PWSTR)malloc(sizeof(WCHAR) * buflen);
	if (0 == GetCurrentDirectoryW(buflen, buf))
	{
		printf("err, GetCurrentDirectoryW() failed. gle = 0x%08x", GetLastError());
		free(buf);
		return false;
	}

	// current dir \\ test.txt ���ϸ� ����
	wchar_t file_name[260];
	if (!SUCCEEDED(StringCbPrintfW(
		file_name,
		sizeof(file_name),
		L"%ws\\bob.txt",
		buf)))
	{
		printf("err, can not create file name");
		free(buf);
		return false;
	}
	free(buf); buf = NULL;

	/*if (true == is_file_existsW(file_name))
	{
	::DeleteFileW(file_name);
	}*/




	HANDLE file_handle = CreateFileW(
		(LPCWSTR)file_name,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
		);
	if (INVALID_HANDLE_VALUE == file_handle)
	{
		printf("err, CreateFile(%ws) failed, gle = %u", file_name, GetLastError());
		return false;
	}

	if (TRUE != create_file(file_handle))
	{
		return false;
	}

	if (TRUE != copy_file(file_name))
	{
		CloseHandle(file_handle);
		return false;
	}

	read_file();

	// check file size 
	// 
	LARGE_INTEGER fileSize;
	if (TRUE != GetFileSizeEx(file_handle, &fileSize))
	{
		printf("err, GetFileSizeEx(%ws) failed, gle = %u", file_name, GetLastError());
		CloseHandle(file_handle);
		return false;
	}

	// [ WARN ]
	// 
	// 4Gb �̻��� ������ ��� MapViewOfFile()���� ������ ���ų� 
	// ���� ������ �̵��� ������ ��
	// FilIoHelperClass ����� �̿��ؾ� ��
	// 
	_ASSERTE(fileSize.HighPart == 0);
	if (fileSize.HighPart > 0)
	{
		printf("file size = %I64d (over 4GB) can not handle. use FileIoHelperClass",
			fileSize.QuadPart);
		CloseHandle(file_handle);
		return false;
	}

	DWORD file_size = (DWORD)fileSize.QuadPart;
	HANDLE file_map = CreateFileMapping(
		file_handle,
		NULL,
		PAGE_READONLY,
		0,
		0,
		NULL
		);
	if (NULL == file_map)
	{
		printf("err, CreateFileMapping(%ws) failed, gle = %u", file_name, GetLastError());
		CloseHandle(file_handle);
		return false;
	}

	PCHAR file_view = (PCHAR)MapViewOfFile(
		file_map,
		FILE_MAP_READ,
		0,
		0,
		0
		);
	if (file_view == NULL)
	{
		printf("err, MapViewOfFile(%ws) failed, gle = %u", file_name, GetLastError());

		CloseHandle(file_map);
		CloseHandle(file_handle);
		return false;
	}

	
	// do some io
	char a = file_view[0];  // 0x d9
	char b = file_view[1];  // 0xb3



	// close all
	UnmapViewOfFile(file_view);
	CloseHandle(file_map);
	CloseHandle(file_handle);
	return true;

}

bool create_file(HANDLE file_handle)
{
	wchar_t str[260];// "���ȯ����� ����~ I can give my word." ���ڿ� ����
	if (!SUCCEEDED(StringCbPrintfW( 
		str,
		sizeof(str),
		L"���ȯ����� ����~ I can give my word.")))
	{
		printf("err, can not create file name");
		return false;
	}

	//���ڿ��� UTF-8�� ���ڵ�
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, str, wcslen(str), NULL, 0, NULL, NULL);
	char* multibyteBuffer;
	multibyteBuffer = (char*)malloc(sizeof(char)*size_needed);
	
	WideCharToMultiByte(CP_UTF8, 0, str, size_needed, multibyteBuffer, size_needed, NULL, NULL);
	
	unsigned char mark[3]; //UTF-8�� �����
	mark[0] = 0xEF;
	mark[1] = 0xBB;
	mark[2] = 0xBF;
	DWORD numberOfByteWritten;

	if (TRUE != ::WriteFile(file_handle, &mark, sizeof(mark), &numberOfByteWritten, NULL)) //UTF-8 ����� write
	{
		printf("err, WriteFile(%s) failed, gle = %u", str, GetLastError());
		CloseHandle(file_handle);
		return false;
	}
	if (TRUE != ::WriteFile(file_handle, multibyteBuffer, size_needed, &numberOfByteWritten, NULL)) // "���ȯ����� ����~ I can give my word." ���ڿ� write
	{
		printf("err, WriteFile(%s) failed, gle = %u", str, GetLastError());
		CloseHandle(file_handle);
		return false;
	}

	free(multibyteBuffer); //malloc ����
	return true;
}

bool copy_file(LPCWSTR input)
{
	// current directory �� ���Ѵ�.
	wchar_t *buf = NULL;
	uint32_t buflen = 0;
	buflen = GetCurrentDirectoryW(buflen, buf);
	if (0 == buflen)
	{
		printf("err, GetCurrentDirectoryW() failed. gle = 0x%08x", GetLastError());
		return false;
	}

	buf = (PWSTR)malloc(sizeof(WCHAR) * buflen);
	if (0 == GetCurrentDirectoryW(buflen, buf))
	{
		printf("err, GetCurrentDirectoryW() failed. gle = 0x%08x", GetLastError());
		free(buf);
		return false;
	}

	// current dir \\ bob2.txt ���ϸ� ����
	wchar_t file_name[260];
	if (!SUCCEEDED(StringCbPrintfW(
		file_name,
		sizeof(file_name),
		L"%ws\\bob2.txt",
		buf)))
	{
		printf("err, can not create file name");
		free(buf);
		return false;
	}
	free(buf);
	buf = NULL;

	if (TRUE != CopyFile(input, (LPCWSTR)file_name, false)) //���� copy
	{
		printf("err, CopyFile() failed. gle: %u", GetLastError());
		return false;
	}

	return true;
}

bool read_file()
{
	// current directory �� ���Ѵ�.
	wchar_t *buf = NULL;
	uint32_t buflen = 0;
	buflen = GetCurrentDirectoryW(buflen, buf);
	if (0 == buflen)
	{
		printf("err, GetCurrentDirectoryW() failed. gle = 0x%08x", GetLastError());
		return false;
	}

	buf = (PWSTR)malloc(sizeof(WCHAR) * buflen);
	if (0 == GetCurrentDirectoryW(buflen, buf))
	{
		printf("err, GetCurrentDirectoryW() failed. gle = 0x%08x", GetLastError());
		free(buf);
		return false;
	}

	// current dir \\ test.txt ���ϸ� ����
	wchar_t file_name[260];
	if (!SUCCEEDED(StringCbPrintfW(
		file_name,
		sizeof(file_name),
		L"%ws\\bob2.txt",
		buf)))
	{
		printf("err, can not create file name");
		free(buf);
		return false;
	}
	free(buf); buf = NULL;

	HANDLE file_handle = CreateFileW(
		(LPCWSTR)file_name,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
		);
	if (INVALID_HANDLE_VALUE == file_handle)
	{
		printf("err, CreateFile(%ws) failed, gle = %u", file_name, GetLastError());
		return false;
	}
	wchar_t* bstrstr = NULL;
	char multibyteBuffer[MAX_LINE];
	char* str = NULL;
	DWORD numberOfByteRead;

	memset(multibyteBuffer, 0, sizeof(multibyteBuffer));

	if (TRUE != ReadFile(file_handle, multibyteBuffer, MAX_LINE-1, &numberOfByteRead, NULL))
	{
		printf("err, ReadFile() failed. gle: %u", GetLastError());
		CloseHandle(file_handle);
		return false;
	}
	int Len1 = ::MultiByteToWideChar(CP_UTF8, 0, multibyteBuffer, -1, bstrstr, 0);
	bstrstr = (wchar_t*)malloc(sizeof(wchar_t)*(Len1+1));
	memset(bstrstr, 0, sizeof(bstrstr));
	::MultiByteToWideChar(CP_UTF8, 0, multibyteBuffer, -1, bstrstr, Len1);

	int Len2 = WideCharToMultiByte(CP_ACP, 0, bstrstr, -1, str, 0, NULL, NULL);
	str = (char*)malloc(Len2 + 1);
	memset(str, 0, sizeof(str));
	WideCharToMultiByte(CP_ACP, 0, bstrstr, -1, str, Len2, NULL, NULL);
	

	printf("%s", str+1); //??? �� 1 ���ؾ��ұ�
	
	free(bstrstr);
	free(str);
	return true;
}