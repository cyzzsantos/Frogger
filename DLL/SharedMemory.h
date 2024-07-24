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

#define MSG_SIZE 50

DWORD WINAPI ReadThread(LPVOID param);

DWORD WINAPI WriteThread(LPVOID param);

DWORD WINAPI SharedMemoryMain(LPVOID param);

VOID SetupSharedMemory(Map* map, MapManager* mapManager, HANDLE hEventoMap, HANDLE hEventEvolve);
