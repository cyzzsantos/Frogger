#include "SharedMemory.h"

typedef struct {
    TCHAR* fileViewMap;
    HANDLE hEvento;     
    HANDLE hMutex;
    int terminar;
    MapManager* mapManager;
} ThreadDadosRead;

typedef struct {
    TCHAR* fileViewMap; 
    HANDLE hEvento;     
    HANDLE hEventMap;
    HANDLE hEventEvolve;
    HANDLE hMutex;      
    int terminar;       
    Map* map;
    MapManager* mapManager;
} ThreadDadosWrite;

typedef struct {
    TCHAR* msg;
    MapManager* mapManager;
} ThreadDadosCommand;   

DWORD WINAPI ThreadComando(LPVOID param) {

    ThreadDadosCommand* data = (ThreadDadosCommand*)param;
    int values[2] = { 0, 0 };
    std::wstring command;
    std::wstringstream ss(data->msg);

    ss >> command >> values[0] >> values[1];

    switch (command[0]) {
    case 's':                     // stop [seconds]
        _tprintf(TEXT("\nRequest received: Freezing all cars for %d seconds"), values[0]);
        data->mapManager->freezeAll();
        Sleep(values[0] * 1000);
        data->mapManager->freezeAll();
        _tprintf(TEXT("\nUnfreezing all cars"));
        break;
    case 'b':                     // block [line] [column]
        _tprintf(TEXT("\nRequest received: Adding a barrier on line %d, column %d"), values[0], values[1]);
        data->mapManager->addElement(new Barrier(values[0], values[1]));
        break;
    case 'r':                     // revert [line]
        _tprintf(TEXT("\nReversing the movement of all cars on line %d"), values[0]);
        data->mapManager->reverseLine(values[0]);
        break;
    default:                      // error
        _tprintf(TEXT("\nReceived an invalid command"));
        return 0;
    }

    return 1;
}

DWORD WINAPI ThreadLer(LPVOID param) {
    ThreadDadosRead* dados = (ThreadDadosRead*)param;
    ThreadDadosCommand* dataCommand = new ThreadDadosCommand;
    HANDLE hThread;
    HANDLE hSemO = CreateSemaphore(NULL, 0, TAM, NOME_SEMOCUPADOS);
    HANDLE hSemL = CreateSemaphore(NULL, TAM, TAM, NOME_SEMLIVRES);
    dataCommand->mapManager = dados->mapManager;

    while (!(dados->terminar)) {
        WaitForSingleObject(hSemO, INFINITE);
        WaitForSingleObject(dados->hMutex, INFINITE);

        dataCommand->msg = dados->fileViewMap;

        ReleaseSemaphore(hSemL, 1, NULL);
        ReleaseMutex(dados->hMutex);

        hThread = CreateThread(NULL, 0, &ThreadComando, dataCommand, 0, NULL);
    }
    return 0;
}

DWORD WINAPI ThreadEscrever(LPVOID param) {
    ThreadDadosWrite* dados = (ThreadDadosWrite*)param;
    TCHAR msg[MAX_SIZE + MAX_WIDTH + 1]{};
    
    int i = 0;

    while (!(dados->terminar)) {
        memset(msg, '\0', sizeof(msg));
        i = 0;

        WaitForSingleObject(dados->hEventMap, INFINITE);

        for (int y = 0; y < dados->mapManager->getMap()->getMap().size(); y++) {
            for (int x = 0; x < dados->mapManager->getMap()->getMap()[0].size(); x++) {
                msg[i] += dados->mapManager->getMap()->getMap()[y][x]->getSymbol();
                i++;
            }
            msg[i] += '\n';
            i++;
        }

        WaitForSingleObject(dados->hMutex, INFINITE);

        ZeroMemory(dados->fileViewMap, MAX_SIZE * 2);
        CopyMemory(dados->fileViewMap, msg, (MAX_SIZE + MAX_HEIGHT + 1) * sizeof(TCHAR));

        ReleaseMutex(dados->hMutex);

        SetEvent(dados->hEvento);
        ResetEvent(dados->hEventMap);
    }
    return 0;
}

DWORD WINAPI SharedMemoryMain(LPVOID param)
{
    ThreadDadosWrite* dataWrite = (ThreadDadosWrite*)param;
    ThreadDadosRead* dataRead = new ThreadDadosRead;
    HANDLE hFileMapWrite, hFileMapRead;

    hFileMapWrite = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, dataWrite->map->getMap().size() * dataWrite->map->getMap()[0].size() * sizeof(TCHAR), TEXT("MAP-MEMORY"));
    hFileMapRead = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, MSG_SIZE * sizeof(TCHAR), TEXT("READ-MEMORY"));
    if (hFileMapWrite == NULL || hFileMapRead == NULL) {
        _tprintf(TEXT("ERROR - Failed to create the File Mapping (hFileMapWrite or hFileMapRead)\n"));
        return -1;
    }


    dataWrite->fileViewMap = (TCHAR*)MapViewOfFile(hFileMapWrite, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    dataRead->fileViewMap = (TCHAR*)MapViewOfFile(hFileMapRead, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (dataWrite->fileViewMap == NULL || dataRead->fileViewMap == NULL) {
        _tprintf(TEXT("ERROR - Failed to define the Map View (fileViewMap)\n"));
        return -1;
    }

    dataWrite->hEvento = CreateEvent(NULL, TRUE, FALSE, TEXT("MAP-HEVENTO"));
    dataRead->hEvento = CreateEvent(NULL, TRUE, FALSE, TEXT("READ-HEVENTO"));
    if (dataWrite->hEvento == NULL || dataRead->hEvento == NULL) {
        _tprintf(TEXT("ERROR - Failed to create an Event (hEvento)\n"));
        UnmapViewOfFile(dataWrite->fileViewMap);
        UnmapViewOfFile(dataRead->fileViewMap);
        return -1;
    }

    dataWrite->hMutex = CreateMutex(NULL, FALSE, TEXT("MAP-MUTEX"));
    dataRead->hMutex = CreateMutex(NULL, FALSE, TEXT("READ-MUTEX"));
    if (dataWrite->hMutex == NULL || dataRead->hMutex == NULL) {
        _tprintf(TEXT("ERROR - Failed to create a Mutex (hMutex)\n"));
        UnmapViewOfFile(dataWrite->fileViewMap);
        UnmapViewOfFile(dataRead->fileViewMap);
        return -1;
    }

    dataWrite->terminar = 0;
    dataRead->terminar = 0;

    dataRead->mapManager = dataWrite->mapManager;

    HANDLE hThreads[2];
    hThreads[0] = CreateThread(NULL, 0, &ThreadLer, dataRead, 0, NULL);
    hThreads[1] = CreateThread(NULL, 0, &ThreadEscrever, dataWrite, 0, NULL);
    if (hThreads[0] != NULL && hThreads[1] != NULL) {
        WaitForMultipleObjects(2, hThreads, TRUE, INFINITE);
    }

    UnmapViewOfFile(dataWrite->fileViewMap);
    UnmapViewOfFile(dataRead->fileViewMap);
    CloseHandle(hFileMapWrite);
    CloseHandle(hFileMapRead);
    return 0;
}

void SetupSharedMemory(Map* map, MapManager* mapManager, HANDLE hEventMap, HANDLE hEventEvolve) {
    HANDLE threadSM;
    ThreadDadosWrite* data = new ThreadDadosWrite;

#ifdef UNICODE
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
#endif

    data->map = map;
    data->mapManager = mapManager;
    data->hEventMap = hEventMap;
    data->hEventEvolve = hEventEvolve;

    threadSM = CreateThread(NULL, 0, &SharedMemoryMain, data, 0, NULL);
}
