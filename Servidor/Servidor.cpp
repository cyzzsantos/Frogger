//#include "DLL.h"
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#include <iostream>
#include <string.h>
#include <queue>
#include <strsafe.h>
#include "SharedMemory.h"
#include "MapManager.h"
#include "rng.h"
#include "Map.h"
#include "Elements.h"
#include "Constants.h"

const LPCSTR REGISTRY_PATH = "SOFTWARE\\TP_SO2";

typedef struct {
	OVERLAPPED ov;
	HANDLE hInstance;
	TCHAR chMessage[BUFSIZE];
	TCHAR chReply[BUFSIZE];
	DWORD cbRead;
	DWORD cbWrite;
	DWORD dwState;
	BOOL fPendingIO;
} PipeData, *LPPipeData;
typedef struct {
	LPPipeData pipe[2];
} PipePair, *LPPipePair;
typedef struct {
	MapManager* mapManager;
	HANDLE MutexUpdates;
	HANDLE hEventMap;
} GameData;

BOOL NewClient(HANDLE hPipe, LPOVERLAPPED lpo)
{
	BOOL fConnected, fPendingIO = FALSE;

	fConnected = ConnectNamedPipe(hPipe, lpo);
	if (fConnected)
	{
		_tprintf(TEXT("ERROR - ConnectNamedPipe failed with %d\n"), GetLastError());
		return false;
	}

	switch (GetLastError()) {
	case ERROR_IO_PENDING:
		fPendingIO = TRUE;
		break;

	case ERROR_PIPE_CONNECTED:
		if (SetEvent(lpo->hEvent))
			break;

	default:
	{
		_tprintf(TEXT("ERROR - ConnectNamedPipe failed with %d\n"), GetLastError());
		return false;
	}
	}

	return fPendingIO;
}
VOID Reconnect(LPPipeData pipe) {
	if (!DisconnectNamedPipe(pipe->hInstance))
		_tprintf(TEXT("Error - DisconnectNamedPipe failed with %d\n"), GetLastError());

	pipe->fPendingIO = NewClient(pipe->hInstance, &pipe->ov);
	pipe->dwState = pipe->fPendingIO ? CONNECTING_STATE : READING_STATE;
}
VOID GetAnswer(LPPipePair pipePair, GameData* gameData, int i) {

	int n = 0;
	int i2 = 0;
	if (i == i2)
		i2 = 1;

	TCHAR msg[MAX_SIZE + MAX_WIDTH + 1]{};

	if (_tcscmp(pipePair->pipe[i]->chMessage, L"")) {
		_tprintf(TEXT("[User %d] - %s\n"), pipePair->pipe[i]->hInstance, pipePair->pipe[i]->chMessage);
	}

	if (!_tcscmp(pipePair->pipe[i]->chMessage, L"up")) {
		gameData->mapManager->moveFrog(i, MOVE_UP);
		WaitForSingleObject(gameData->MutexUpdates, INFINITE);
		SetEvent(gameData->hEventMap);
		ReleaseMutex(gameData->MutexUpdates);
	}

	else if (!_tcscmp(pipePair->pipe[i]->chMessage, L"down")) {
		gameData->mapManager->moveFrog(i, MOVE_DOWN);
		WaitForSingleObject(gameData->MutexUpdates, INFINITE);
		SetEvent(gameData->hEventMap);
		ReleaseMutex(gameData->MutexUpdates);
	}

	else if (!_tcscmp(pipePair->pipe[i]->chMessage, L"left")) {
		gameData->mapManager->moveFrog(i, MOVE_LEFT);
		WaitForSingleObject(gameData->MutexUpdates, INFINITE);
		SetEvent(gameData->hEventMap);
		ReleaseMutex(gameData->MutexUpdates);
	}

	else if (!_tcscmp(pipePair->pipe[i]->chMessage, L"right")) {
		gameData->mapManager->moveFrog(i, MOVE_RIGHT);
		WaitForSingleObject(gameData->MutexUpdates, INFINITE);
		SetEvent(gameData->hEventMap);
		ReleaseMutex(gameData->MutexUpdates);
	}

	gameData->mapManager->evolve(FROGS);
	memset(pipePair->pipe[i]->chMessage, 0, sizeof(pipePair->pipe[i]->chMessage));
	memset(pipePair->pipe[i2]->chMessage, 0, sizeof(pipePair->pipe[i2]->chMessage));

	for (int y = 0; y < gameData->mapManager->getMap()->getMap().size(); y++) {
		for (int x = 0; x < gameData->mapManager->getMap()->getMap()[0].size(); x++) {
			msg[n] += gameData->mapManager->getMap()->getMap()[y][x]->getSymbol();
			n++;
		}
		msg[n] += '\n';
		n++;
	}

	StringCchCopy(pipePair->pipe[i]->chReply, BUFSIZE, msg);
	StringCchCopy(pipePair->pipe[i2]->chReply, BUFSIZE, msg);
	pipePair->pipe[i]->cbWrite = _tcslen(pipePair->pipe[i2]->chReply) * sizeof(TCHAR);
	pipePair->pipe[i2]->cbWrite = _tcslen(pipePair->pipe[i2]->chReply) * sizeof(TCHAR);
}

DWORD WINAPI ServerMain(LPVOID param) {
	GameData* gameData = (GameData*)param;
	DWORD i, offset, bytes;
	BOOL fSuccess;
	int connectedClients = 0;

	PipeData pipe[MAX_CLI];
	HANDLE hEvents[MAX_CLI + 1];

	for (int i = 0; i < MAX_CLI; i++)
	{
		hEvents[i] = CreateEvent(NULL, TRUE, TRUE, NULL);
		if (hEvents[i] == NULL)
		{
			_tprintf(TEXT("ERROR - Failed to initialize hEvents[%d]\n"), i);
			return -1;
		}

		ZeroMemory(&pipe[i].ov, sizeof(pipe[i].ov));
		memset(pipe[i].chMessage, 0, sizeof(pipe[i].chMessage));
		pipe[i].ov.hEvent = hEvents[i];
		pipe[i].ov.Offset = 0;
		pipe[i].ov.OffsetHigh = 0;

		pipe[i].hInstance = CreateNamedPipe(
			PIPE_NAME,
			PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
			PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE,
			MAX_CLI,
			BUFSIZE * sizeof(TCHAR),
			BUFSIZE * sizeof(TCHAR),
			NMPWAIT_USE_DEFAULT_WAIT,
			NULL);
		if (pipe[i].hInstance == INVALID_HANDLE_VALUE)
		{
			_tprintf(TEXT("ERROR - CreateNamedPipe of pipe[%d] failed with %d\n"), i, GetLastError());
			return -1;
		}

		pipe[i].fPendingIO = NewClient(pipe[i].hInstance, &pipe[i].ov);
		pipe[i].dwState = pipe[i].fPendingIO ? CONNECTING_STATE : READING_STATE;
	}

	HANDLE hStartGame = CreateEvent(NULL, TRUE, FALSE, TEXT("STARTGAME"));
	if (hStartGame == NULL) {
		_tprintf(TEXT("ERROR - Failed to create an event (hStartGame)\n"));
		return -1;
	}
	hEvents[2] = gameData->hEventMap;

	while (1)
	{
		offset = WaitForMultipleObjects(MAX_CLI + 1, hEvents, FALSE, INFINITE);;
		i = offset - WAIT_OBJECT_0;
		if (i < 0 || i > MAX_CLI)
		{
			_tprintf(TEXT("ERROR - Something went wrong (WaitForMultipleObjects)\n"));
			return -1;
		}

		if (i == 2)
		{
			i = 0;
			pipe[0].fPendingIO = false;
			pipe[0].dwState = WRITING_STATE;
		}

		if (pipe[i].fPendingIO)
		{
			fSuccess = GetOverlappedResult(
				pipe[i].hInstance,
				&pipe[i].ov,
				&bytes,
				FALSE);

			switch (pipe[i].dwState)
			{
				case CONNECTING_STATE:
				{
					if (!fSuccess) {
						_tprintf(TEXT("ERROR - %d (GetOverlappedResult)\n"), GetLastError());
						return -1;
					}
					pipe[i].dwState = READING_STATE;
					connectedClients++;
					break;
				}

				case READING_STATE:
				{
					if (!fSuccess || bytes == 0)
					{
						Reconnect(&pipe[i]);
						continue;
					}
					pipe[i].cbRead = bytes;
					pipe[i].dwState = WRITING_STATE;
					break;
				}

				case WRITING_STATE:
				{
					if (!fSuccess || bytes != pipe[i].cbWrite)
					{
						Reconnect(&pipe[i]);
						continue;
					}
					pipe[i].dwState = READING_STATE;
					break;
				}

				default: 
				{
					_tprintf(TEXT("ERROR - Invalid pipe state\n"));
					return -1;
				}
			}
		}

		if (connectedClients >= 2)
			SetEvent(hStartGame);

		switch (pipe[i].dwState)
		{
			case READING_STATE:
			{
				fSuccess = ReadFile(
					pipe[i].hInstance,
					pipe[i].chMessage,
					BUFSIZE * sizeof(TCHAR),
					&pipe[i].cbRead,
					&pipe[i].ov);

				if (fSuccess && pipe[i].cbRead != 0)
				{
					pipe[i].fPendingIO = FALSE;
					pipe[i].dwState = WRITING_STATE;
					continue;
				}

				if (!fSuccess && (GetLastError() == ERROR_IO_PENDING))
				{
					pipe[i].fPendingIO = TRUE;
					continue;
				}

				Reconnect(&pipe[i]);
				break;
			}

			case WRITING_STATE:
			{
				PipePair pipePair;
				pipePair.pipe[0] = &pipe[0];
				pipePair.pipe[1] = &pipe[1];
				GetAnswer(&pipePair, gameData, i);

				for (int i = 0; i < MAX_CLI; i++) 
				{
					fSuccess = WriteFile(
						pipePair.pipe[i]->hInstance,
						pipePair.pipe[i]->chReply,
						pipePair.pipe[i]->cbWrite,
						&bytes,
						&pipePair.pipe[i]->ov);

					if (fSuccess && bytes == pipePair.pipe[i]->cbWrite) {
						pipePair.pipe[i]->fPendingIO = FALSE;
						pipePair.pipe[i]->dwState = READING_STATE;
						continue;
					}

					if (!fSuccess && (GetLastError() == ERROR_IO_PENDING))
					{
						pipePair.pipe[i]->fPendingIO = TRUE;
						continue;
					}

					Reconnect(pipePair.pipe[i]);
				}
				memset(pipe[0].chReply, 0, sizeof(pipe[0].chReply));
				memset(pipe[1].chReply, 0, sizeof(pipe[1].chReply));
				break;
			}

			default:
			{
				_tprintf(TEXT("ERROR - Invalid pipe state\n"));
				return -1;
			}
		}
	}
	return 0;
}

int _tmain(int argc, TCHAR* argv[])
{
	HANDLE MutexServidor;
	HANDLE hEventMap;
	HANDLE hEventEvolve;
	HANDLE hPipeThread;
	HANDLE MutexUpdates;
	HANDLE MutexLevel;
	HANDLE hStartGame;
	HKEY hKey;
	DWORD dwDisposition;
	DWORD height, speed;
	MapManager* mapManager;
	GameData* gameData = new GameData;
	

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	MutexServidor = CreateMutex(NULL, FALSE, TEXT("SERVIDOR-MUTEX"));
	if (ERROR_ALREADY_EXISTS == GetLastError())
	{
		_tprintf(TEXT("ERROR - There's already an instance of the server running\n"));
		return -1;
	}

	MutexUpdates = CreateMutex(NULL, FALSE, TEXT("UPDATES-MUTEX"));
	if (MutexUpdates == NULL)
	{
		_tprintf(TEXT("ERROR - Couldn't create MutexUpdates\n"));
		return -1;
	}

	MutexLevel = CreateMutex(NULL, FALSE, TEXT("LEVEL-MUTEX"));
	if (MutexLevel == NULL)
	{
		_tprintf(TEXT("ERROR - Couldn't create MutexUpdates\n"));
		return -1;
	}

	// creates the registry
	if (RegCreateKeyExA(HKEY_CURRENT_USER, REGISTRY_PATH, 0, NULL, 0, KEY_ALL_ACCESS, 0, &hKey, &dwDisposition) != ERROR_SUCCESS) {
		_tprintf(TEXT("ERROR - Couldn't access the registry"));
		return -1;
	}

	// conditions: created the registry but received arguments or registry already existed but received arguments
	if (argc == 3) {
		height = _tstoi(argv[1]);
		speed = _tstoi(argv[2]);

		if (RegSetValueExA(hKey, "HEIGHT", 0, REG_DWORD, (const BYTE*)&height, sizeof(height)) != ERROR_SUCCESS) {
			_tprintf(TEXT("ERROR - Couldn't save the value of HEIGHT on the registry"));
			return -1;
		}

		if (RegSetValueExA(hKey, "SPEED", 0, REG_DWORD, (const BYTE*)&speed, sizeof(speed)) != ERROR_SUCCESS) {
			_tprintf(TEXT("ERROR - Couldn't save the value of WIDTH on the registry"));
			return -1;
		}

		if ((int)height < 5 || (int)speed < 2  || int(height) > 10 || (int)speed > 10) {
			_tprintf(TEXT("\nERROR - Minimum height: 5, Minimum speed: 1, Maximum height: 10, Maximum speed: 10"));
			return -1;
		}

		mapManager = new MapManager((int)height, MAX_WIDTH);
		mapManager->setCarSpeed(speed * 100);
	}

	// conditions: created the registry and didn't receive arguments (uses default values)
	else if (dwDisposition == REG_CREATED_NEW_KEY && argc < 3) {
		DWORD buf = MAX_HEIGHT;
		height = MAX_HEIGHT;

		if (RegSetValueExA(hKey, "HEIGHT", 0, REG_DWORD, (const BYTE*)&buf, sizeof(buf)) != ERROR_SUCCESS) {
			_tprintf(TEXT("ERROR - Couldn't save the value of HEIGHT on the registry"));
			return -1;
		}

		buf = 5;
		if (RegSetValueExA(hKey, "SPEED", 0, REG_DWORD, (const BYTE*)&buf, sizeof(buf)) != ERROR_SUCCESS) {
			_tprintf(TEXT("ERROR - Couldn't save the value of SPEED on the registry"));
			return -1;
		}

		mapManager = new MapManager(MAX_HEIGHT, MAX_WIDTH);
		mapManager->setCarSpeed(DEFAULT_SPEED);
	}

	// conditions: registry already existed and didn't received arguments
	else if (dwDisposition == REG_OPENED_EXISTING_KEY && argc < 3) {
		DWORD size = sizeof(DWORD);

		if (RegGetValueA(hKey, NULL, "HEIGHT", RRF_RT_ANY, NULL, &height, &size) != ERROR_SUCCESS) {
			_tprintf(TEXT("ERROR - Couldn't access the value of HEIGHT on the registry"));
			return -1;
		}
		if (RegGetValueA(hKey, NULL, "SPEED", RRF_RT_ANY, NULL, &speed, &size) != ERROR_SUCCESS) {
			_tprintf(TEXT("ERROR - Couldn't access the value of SPEED on the registry"));
			return -1;
		}
		
		if ((int)height < 5 || (int)speed < 2 || int(height) > 10 || (int)speed > 10) {
			_tprintf(TEXT("\nERROR - Minimum height: 5, Minimum speed: 1, Maximum height: 10, Maximum speed: 10"));
			return -1;
		}

		mapManager = new MapManager((int)height, MAX_WIDTH);
		mapManager->setCarSpeed(DEFAULT_SPEED);
	}

	else {
		// something went wrong, mapManager is being initialized with default values
		mapManager = new MapManager(MAX_HEIGHT, MAX_WIDTH);
		mapManager->setCarSpeed(DEFAULT_SPEED);
	}

	_tprintf(TEXT("starting up...\n"));

	hEventMap = CreateEvent(NULL, TRUE, FALSE, TEXT("MAP-HEVENTMAP"));
	hEventEvolve = CreateEvent(NULL, TRUE, FALSE, TEXT("MAP-HEVENTEVOLVE"));
	hStartGame = CreateEvent(NULL, TRUE, FALSE, TEXT("STARTGAME"));
	if (hEventMap == NULL  || hEventEvolve == NULL || hStartGame == NULL){
		_tprintf(TEXT("ERROR - Failed to create an event (hEventMap or hEventEvolve or hStartGame)\n"));
		return -1;
	}

	SetupSharedMemory(mapManager->getMap(), mapManager, hEventMap, hEventEvolve);

	gameData->mapManager = mapManager;
	gameData->hEventMap = hEventMap;
	gameData->MutexUpdates = MutexUpdates;
	hPipeThread = CreateThread(NULL, 0, &ServerMain, gameData, 0, NULL);

	while (1)
	{	
		WaitForSingleObject(hStartGame, INFINITE);
		WaitForSingleObject(MutexLevel, INFINITE);
		mapManager->evolve(CARS);
		ReleaseMutex(MutexLevel);

		WaitForSingleObject(MutexUpdates, INFINITE);
		SetEvent(hEventMap);
		ReleaseMutex(MutexUpdates);

		Sleep(mapManager->getCarSpeed());
	}

	ReleaseMutex(MutexServidor);
	_tprintf(TEXT("closing..."));
}

