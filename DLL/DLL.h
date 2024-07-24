#ifdef SHAREDMEMORY_EXPORTS
#define SHAREDMEMORY_API __declspec(dllexport)
#else
#define SHAREDMEMORY_API __declspec(dllimport)
#endif

#pragma once
#include <iostream>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#include <sstream>
#include "MapManager.h"
#include "Map.h"
#include "Elements.h"
#include "Constants.h"
#include "rng.h"

#define NUM_CHARS 50
#define MSG_SIZE 50
#define TAM 100
#define NOME_SHM         _T("memória")
#define NOME_MUTEX       _T("mutex")
#define NOME_SEMOCUPADOS _T("ocupados")
#define NOME_SEMLIVRES   _T("livres")

std::atomic<bool> inputMutexLocked(false); // temporary solution to have commands working 

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

typedef struct {
    TCHAR* fileViewMap;
    HANDLE hEvento;
    HANDLE hEventMap;
    HANDLE hEventEvolve;
    HANDLE hMutex;
    HANDLE hInputMutex;
    int terminar;
} ThreadDadosReadOperador;

typedef struct {
    TCHAR* fileViewMap;
    HANDLE hEvento;
    HANDLE hMutex;
    HANDLE hInputMutex;
    HANDLE SEM_Mutex_P;
    // HANDLE SEM_Mutex_C; 
    HANDLE SEM_Empty;
    HANDLE SEM_Items;
    int terminar;
} ThreadDadosWriteOperador;

extern "C" SHAREDMEMORY_API DWORD WINAPI ThreadComando(LPVOID param);

extern "C" SHAREDMEMORY_API DWORD WINAPI ReadThread(LPVOID param);

extern "C" SHAREDMEMORY_API DWORD WINAPI WriteThread(LPVOID param);

extern "C" SHAREDMEMORY_API DWORD WINAPI SharedMemoryMain(LPVOID param);

extern "C" SHAREDMEMORY_API bool verifyCommand(TCHAR msg[NUM_CHARS]);

extern "C" SHAREDMEMORY_API DWORD WINAPI ThreadLer(LPVOID param);

extern "C" SHAREDMEMORY_API DWORD WINAPI ThreadEscrever(LPVOID param);

extern "C" SHAREDMEMORY_API void SetupSharedMemory(Map * map, MapManager * mapManager, HANDLE hEventMap, HANDLE hEventEvolve);
