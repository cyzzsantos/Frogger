#pragma once
#include <iostream>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#include <sstream>

#ifndef SM_H
#define SM_H
#include "Map.h"
#include "MapManager.h"
#endif SM_H

#define NOME_SHM         _T("memória")
#define NOME_MUTEX       _T("mutex")
#define NOME_SEMOCUPADOS _T("ocupados")
#define NOME_SEMLIVRES   _T("livres")
#define TAM 100
#define MSG_SIZE 50

DWORD WINAPI ThreadComando(LPVOID param);

DWORD WINAPI ThreadLer(LPVOID param);

DWORD WINAPI ThreadEscrever(LPVOID param);

DWORD WINAPI SharedMemoryMain(LPVOID param);

VOID SetupSharedMemory(Map* map, MapManager* mapManager, HANDLE hEventoMap, HANDLE hEventEvolve);
