#include <iostream>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <aio.h>
#include <string.h>
#include <sys/sem.h>
#include <unistd.h>
#include <signal.h>
using namespace std;
struct fileArray 
{
    char** fileNameArray;
    int num;
};
struct argsStruct
{
    fileArray fileArrayField;
    int filedes;
};
void* readFile(void* param);
void* writeFile(void* param);
extern "C" void processingFiles(fileArray arr, char* outFilename)
{
    
    argsStruct readerStruct, writerStruct;
    readerStruct.fileArrayField = arr;
    int filedes[2];
    pipe(filedes);
    readerStruct.filedes = filedes[1];
    writerStruct.fileArrayField.fileNameArray = new char*[1];
    writerStruct.fileArrayField.fileNameArray[0] = outFilename;
    writerStruct.fileArrayField.num = 1;
    writerStruct.filedes = filedes[0];
    pthread_t thread, thread1;
    pthread_create(&thread, NULL, readFile, &readerStruct);
    sleep(1);
    pthread_create(&thread1, NULL, writeFile, &writerStruct);
    pthread_join(thread, NULL);
    usleep(100);
    pthread_cancel(thread1);
};

void* readFile(void* param)
{
    argsStruct *args = (argsStruct*)param;
    int filedes = (*args).filedes;
    aiocb cb;
    fileArray array;
    array = (*args).fileArrayField;
    int sem = semget(ftok("./main.cpp", 1), 2, IPC_CREAT | 0666);              // 0 - семафор чтения, 1 - семафор записи 
    struct sembuf wait = {1, 0, SEM_UNDO};                                     //Ожидание семафора записи
    struct sembuf lock = {1, 1, SEM_UNDO};                                     //Блокировка семафора записи
    struct sembuf unlock = {0, -1, SEM_UNDO};                                   
    for(int i = 0; i < array.num; i++)
    {
        struct stat st;
        cout << "filename: " << array.fileNameArray[i];
        stat(array.fileNameArray[i], &st);
        cout << " filesize: " << st.st_size << endl;
        int file = open(array.fileNameArray[i], O_RDONLY, 0);
        char* buffer = new char[st.st_size];

        memset(&cb, 0, sizeof(aiocb));
        cb.aio_nbytes = st.st_size;
        cb.aio_fildes = file;
        cb.aio_offset = 0;
        cb.aio_buf = buffer;
        aio_read(&cb);
        while(aio_error(&cb) == EINPROGRESS);
        semop(sem, &wait, 1);  
        semop(sem, &lock, 1);
        sleep(2);
        int result = write(filedes, buffer, st.st_size);
        semop(sem, &unlock, 1);
        delete[] buffer;
        close(file);
    }
};

void* writeFile(void* param)
{
    int sem = semget(ftok("./main.cpp", 1), 2, IPC_CREAT | 0666);
    struct sembuf wait = {0, 0, SEM_UNDO};                                     //Ожидание семафора чтения
    struct sembuf lock = {0, 1, SEM_UNDO};                                     //Блокировка семафора чтения
    struct sembuf unlock = {1, -1, SEM_UNDO};   


    char* filename;
    aiocb writeStruct;
    memset(&writeStruct, 0, sizeof(aiocb));
    argsStruct *args = (argsStruct*)param;
    filename = (*args).fileArrayField.fileNameArray[0];
    int filedes = (*args).filedes;
    int file = open(filename, O_WRONLY, 0);
    char* buffer;
    writeStruct.aio_fildes = file;
    writeStruct.aio_offset = 0;
    int readedBytes = 80;
    while(1)
    {
        semop(sem, &wait, 1); 
        semop(sem, &lock, 1);

        while(1)
        {
            buffer = new char[80]();
            writeStruct.aio_buf = buffer;

            readedBytes = read(filedes, buffer, 80);
            cout << "write: " << readedBytes << endl;
            writeStruct.aio_nbytes = readedBytes;
            aio_write(&writeStruct);
            while(aio_error(&writeStruct) == EINPROGRESS);
            writeStruct.aio_offset += readedBytes;
            delete[] buffer;  
            if(readedBytes < 80) break;  
        }
        semop(sem, &unlock, 1); 
    }
    close(file);
};