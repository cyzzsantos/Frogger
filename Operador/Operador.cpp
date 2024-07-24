#include <iostream>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#include <sstream>

#define NUM_CHARS 50
#define TAM 100
#define NOME_SHM         _T("memória")
#define NOME_MUTEX       _T("mutex")
#define NOME_SEMOCUPADOS _T("ocupados")
#define NOME_SEMLIVRES   _T("livres")
std::atomic<bool> inputMutexLocked(false);

typedef struct {
    TCHAR* fileViewMap;
    HANDLE hEvento;     
    HANDLE hEventMap;
    HANDLE hEventEvolve;
    HANDLE hMutex;      
    HANDLE hInputMutex;
    int terminar;       
}ThreadDadosRead;

typedef struct {
    TCHAR* fileViewMap; 
    HANDLE hEvento;     
    HANDLE hMutex;
    HANDLE hInputMutex;
    int terminar;
} ThreadDadosWrite;

bool verifyCommand(TCHAR msg[NUM_CHARS]) 
{
    std::wstring command;
    std::wstring validCommands[3] = {TEXT("stop"), TEXT("block"), TEXT("revert")};
    std::wstringstream ss(msg);

    int values[2] = { 0, 0 };

    ss >> command >> values[0] >> values[1];

    // validates commands
    if (command != validCommands[0] && command != validCommands[1] && command != validCommands[2])
        return false;

    return true;
}

DWORD WINAPI ThreadLer(LPVOID param) {
    ThreadDadosRead* dados = (ThreadDadosRead*)param;
    TCHAR* msg = nullptr;

    while (!(dados->terminar)) {
        WaitForSingleObject(dados->hEvento, INFINITE);
        WaitForSingleObject(dados->hMutex, INFINITE);

        msg = dados->fileViewMap;

        ReleaseMutex(dados->hMutex);

        if (!inputMutexLocked) {
        #ifdef _WIN32			
                    system("cls");      // Windows
        #else					
                    system("clear");    // macOS or Linux
        #endif

        _tprintf(TEXT("%s\n\nPress ENTER to write a command"), msg);
        }

        ResetEvent(dados->hEvento);
    }
    return 0;
}

DWORD WINAPI ThreadEscrever(LPVOID param) {
    ThreadDadosWrite* dados = (ThreadDadosWrite*)param;
    TCHAR msg[NUM_CHARS];
    std::wstring startInput;
    HANDLE hSemO = CreateSemaphore(NULL, 0, TAM, NOME_SEMOCUPADOS);
    HANDLE hSemL = CreateSemaphore(NULL, TAM, TAM, NOME_SEMLIVRES);
    while (!(dados->terminar)) {
        std::getline(std::wcin, startInput);
        inputMutexLocked.exchange(true);

        _fgetts(msg, NUM_CHARS, stdin);
        msg[_tcslen(msg) - 1] = '\0';

        if (!verifyCommand(msg))
        {
            _tprintf(TEXT("Invalid command"));
            Sleep(500);
            inputMutexLocked.exchange(false);
            continue;
        }

        inputMutexLocked.exchange(false);

        WaitForSingleObject(hSemL, INFINITE);
        WaitForSingleObject(dados->hMutex, INFINITE);
        
        ZeroMemory(dados->fileViewMap, NUM_CHARS);
        CopyMemory(dados->fileViewMap, msg, _tcslen(msg) * sizeof(TCHAR));

        ReleaseSemaphore(hSemO, 1, NULL);
        ReleaseMutex(dados->hMutex);
    }
    return 0;
}

int _tmain(int argc, TCHAR* argv[]) {
    ThreadDadosRead* dadosRead;
    ThreadDadosWrite* dadosWrite;
    HANDLE hFileMapRead, hFileMapWrite; 

#ifdef UNICODE
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
#endif

    dadosRead = new ThreadDadosRead;
    dadosWrite = new ThreadDadosWrite;

    dadosRead->hEventMap = CreateEvent(NULL, TRUE, FALSE, TEXT("MAP-HEVENTMAP"));
    dadosRead->hEventEvolve = CreateEvent(NULL, TRUE, FALSE, TEXT("MAP-HEVENTEVOLVE"));

    hFileMapRead = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, TEXT("MAP-MEMORY"));
    hFileMapWrite = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, TEXT("READ-MEMORY"));
    if (hFileMapRead == NULL || hFileMapWrite == NULL) {
        _tprintf(TEXT("ERROR - Failed to create the File Mapping (hFileMapWrite or hFileMapRead)\n"));
        return -1;
    }

    dadosRead->fileViewMap = (TCHAR*)MapViewOfFile(hFileMapRead, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    dadosWrite->fileViewMap = (TCHAR*)MapViewOfFile(hFileMapWrite, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (dadosRead->fileViewMap == NULL || dadosWrite->fileViewMap == NULL) {
        _tprintf(TEXT("ERROR - Failed to define the Map View (fileViewMap)\n"));
        return -1;
    }

    dadosRead->hEvento = CreateEvent(NULL, TRUE, FALSE, TEXT("MAP-HEVENTO"));
    dadosWrite->hEvento = CreateEvent(NULL, TRUE, FALSE, TEXT("READ-HEVENTO"));
    if (dadosRead->hEvento == NULL || dadosWrite->hEvento == NULL) {
        _tprintf(TEXT("ERROR - Failed to create an Event (hEvento)\n"));
        UnmapViewOfFile(dadosRead->fileViewMap);
        UnmapViewOfFile(dadosWrite->fileViewMap);
        return -1;
    }

    dadosRead->hMutex = CreateMutex(NULL, FALSE, TEXT("MAP-MUTEX"));
    dadosWrite->hMutex = CreateMutex(NULL, FALSE, TEXT("READ-MUTEX"));
    if (dadosRead->hMutex == NULL || dadosWrite->hMutex == NULL) {
        _tprintf(TEXT("ERROR - Failed to create a Mutex (hMutex)\n"));
        UnmapViewOfFile(dadosRead->fileViewMap);
        UnmapViewOfFile(dadosWrite->fileViewMap);
        return -1;
    }

    dadosRead->hInputMutex = CreateMutex(NULL, FALSE, TEXT("INPUT-MUTEX"));
    dadosWrite->hInputMutex = dadosRead->hInputMutex;
    if (dadosWrite->hInputMutex == NULL || dadosRead->hInputMutex == NULL) {
        _tprintf(TEXT("ERROR - Failed to create a Mutex (hInputMutex)\n"));
        UnmapViewOfFile(dadosRead->fileViewMap);
        UnmapViewOfFile(dadosWrite->fileViewMap);
        return -1;
    }

    dadosRead->terminar = 0;
    dadosWrite->terminar = 0;

    HANDLE hThreads[2]; 
    hThreads[0] = CreateThread(NULL, 0, &ThreadLer, dadosRead, 0, NULL);
    hThreads[1] = CreateThread(NULL, 0, &ThreadEscrever, dadosWrite, 0, NULL);
    if (hThreads[0] != NULL && hThreads[1] != NULL) {
        WaitForMultipleObjects(2, hThreads, TRUE, INFINITE);
    }

    UnmapViewOfFile(dadosRead->fileViewMap);
    UnmapViewOfFile(dadosWrite->fileViewMap);
    CloseHandle(hFileMapRead);
    CloseHandle(hFileMapWrite);
    return 0;
}