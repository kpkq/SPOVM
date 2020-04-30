// Dll_laba5.cpp : Определяет экспортированные функции для приложения DLL.
//
#include"stdafx.h"

int sum(int a, int b)
{
	return a + b;
}

struct setOfFiles
{
	TCHAR **fileNameArray;
	int fileNum;
};
DWORD WINAPI readFunction(LPVOID param)
{
	std::cout << "read begins" << std::endl;
	HANDLE hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, "readWriteEvent");
	HANDLE readEnded = OpenEvent(EVENT_ALL_ACCESS, FALSE, "ended");
	char *buffer;
	DWORD bytesWrittenInPipe = 0, bytesReadedFromFile = 0;
	LPCSTR pipeName = "\\\\.\\pipe\\pipeName";
	HANDLE hNamedPipe;
	hNamedPipe = CreateNamedPipe(pipeName,
		PIPE_ACCESS_DUPLEX,
		PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
		PIPE_UNLIMITED_INSTANCES,
		512, 512, 5000, NULL);
	ConnectNamedPipe(hNamedPipe, NULL);
	setOfFiles *set;
	set = (setOfFiles*)param;
	for (int i = 0; i < 3; i++)
	{
		OVERLAPPED stOverLapped = { 0 };
		HANDLE hFile = CreateFile(
			set->fileNameArray[i],
			GENERIC_READ,          // open for reading
			FILE_SHARE_READ,       // share for reading
			NULL,                  // default security
			OPEN_EXISTING,         // existing file only
			FILE_FLAG_OVERLAPPED, // normal file
			NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			printf("Error %x\n", GetLastError());
			return 1;
		}
		bool readAgain = true;
		do
		{
			buffer = new char[80]();
			ReadFile(hFile, buffer, 80, &bytesReadedFromFile, &stOverLapped);
			std::cout << "buffer: " << buffer << std::endl;
			if (stOverLapped.InternalHigh != 80)
			{
				readAgain = false;
			}
			stOverLapped.Offset += 80;
			WriteFile(hNamedPipe, buffer, stOverLapped.InternalHigh, &bytesWrittenInPipe, NULL);
			delete buffer;
			SetEvent(hEvent);
			WaitForSingleObject(hEvent, INFINITE);
		} while (readAgain);
		CloseHandle(hFile);
	}
	std::cout << "read ended" << std::endl;
	SetEvent(readEnded);
	return 0;
}

DWORD WINAPI writeFunction(LPVOID param)
{
	TCHAR *filename;
	OVERLAPPED stOverLapped = { 0 };
	filename = (TCHAR*)param;
	HANDLE hFile = CreateFile(
		filename,
		GENERIC_WRITE,          // open for reading
		FILE_SHARE_WRITE,       // share for reading
		NULL,                  // default security
		OPEN_EXISTING,         // existing file only
		FILE_FLAG_OVERLAPPED, // normal file
		NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		printf("Error %x\n", GetLastError());
		return 1;
	}
	HANDLE hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, "readWriteEvent");
	HANDLE readEnded = OpenEvent(EVENT_ALL_ACCESS, FALSE, "ended");
	ResetEvent(readEnded);
	LPCSTR pipeName = "\\\\.\\pipe\\pipeName";
	DWORD bytesReadedFromPipe = 0, bytesWritedInFile = 0;
	HANDLE hNamedPipe = CreateFile(
		pipeName, GENERIC_READ,
		0, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		printf("Error %x\n", GetLastError());
		return 1;
	}
	char *buffer = new char[80]();
	while (1)
	{
		WaitForSingleObject(hEvent, 1000);
		if (WaitForSingleObject(readEnded, 0) == WAIT_OBJECT_0) break;
		char *buffer = new char[80]();
		std::cout << "new iteration" << std::endl;
		ReadFile(hNamedPipe, buffer, 80, &bytesReadedFromPipe, NULL);
		std::cout << "bytes readed " << bytesReadedFromPipe << buffer << std::endl;
		WriteFile(hFile, buffer, bytesReadedFromPipe, &bytesWritedInFile, &stOverLapped);
		stOverLapped.Offset += bytesReadedFromPipe;
		std::cout << "new iteration\n";
		delete buffer;
		SetEvent(hEvent);
	}
	std::cout << "write ended" << std::endl;
	return 0;
}