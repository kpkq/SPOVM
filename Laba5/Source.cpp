#include<iostream>
#include<Windows.h>
HANDLE readWriteEvent = CreateEvent(NULL, FALSE, FALSE, "readWriteEvent");
HANDLE readEnded = CreateEvent(NULL, FALSE, FALSE, "ended");
DWORD WINAPI readFunction(LPVOID);
struct setOfFiles
{
	TCHAR **fileNameArray;
	int fileNum;
};

int main()
{
	
	HMODULE libHndl = LoadLibrary("D:\\4sem\\LabiWIn\\Dll_laba5\\Debug\\Dll_laba5.dll");

	
	DWORD(*pReadFunction)(LPVOID);
	(FARPROC &)pReadFunction = GetProcAddress(libHndl, "readFunction");
	DWORD(*pWriteFunction)(LPVOID);
	(FARPROC &)pWriteFunction = GetProcAddress(libHndl, "writeFunction");
	TCHAR **fileNameArray;
	fileNameArray = new TCHAR*[3];
	TCHAR file1[] = TEXT("D:\\4sem\\LabiWIn\\Laba5\\Laba5\\files\\file1.txt");
	TCHAR file2[] = TEXT("D:\\4sem\\LabiWIn\\Laba5\\Laba5\\files\\file2.txt");
	TCHAR file3[] = TEXT("D:\\4sem\\LabiWIn\\Laba5\\Laba5\\files\\file3.txt");
	TCHAR outFile[] = TEXT("D:\\4sem\\LabiWIn\\Laba5\\Laba5\\files\\resultFile.txt");
	fileNameArray[0] = file1;
	fileNameArray[1] = file2;
	fileNameArray[2] = file3;
	setOfFiles set;
	set.fileNameArray = fileNameArray;
	set.fileNum = 3;


	DWORD id;
	HANDLE hWriteEvent;
	HANDLE hReadThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)pReadFunction, (LPVOID*)&set, 0, &id);
	Sleep(100);
	hWriteEvent = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)pWriteFunction, (LPVOID)outFile, 0, &id);
	WaitForSingleObject(hWriteEvent, INFINITE);
	FreeLibrary(libHndl);
	system("pause");
}
