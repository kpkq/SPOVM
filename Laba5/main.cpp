#include <iostream>
#include <dlfcn.h>
using namespace std;

struct fileArray 
{
    char** fileNameArray;
    int num;
};

int main()
{
    void* libHandle = dlopen("/home/pipa/SPOVM/Test/dl.so", RTLD_LAZY);
    fileArray arr;
    char file[] = "file.txt";
    char file1[] = "file1.txt";
    char resultFile[] = "resultFile.txt";
    char **array = new char*[2];
    array[0] = file;
    array[1] = file1;
    arr.fileNameArray = array;
    arr.num = 2;
    void (*processingFilesFunction)(fileArray, char*);
    *(void**)(&processingFilesFunction) = dlsym(libHandle, "processingFiles");
    
    (*processingFilesFunction)(arr, resultFile);
    dlclose(libHandle);
}