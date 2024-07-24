#include "Conectar.h"

#define PIPE_NAME TEXT("\\\\.\\pipe\\teste")



typedef struct {
	HANDLE hEventMove;
} ThreadData;

DWORD WINAPI Thread(LPVOID param) {

	while (1)
	{
	
	}
	return 0;
}

int Conectar() {
	AllocConsole();
	freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);

	HANDLE hPipe, hThread, hEventTemp;
	ThreadDados dados;

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	int i = 0;
	DWORD nBytes = 0;
	dados.terminar = 0;
	dados.hMutex = CreateMutex(NULL, FALSE, NULL); //Criação do mutex
	if (dados.hMutex == NULL) {
		_tprintf(TEXT("[Erro] ao criar mutex!\n"));
		return -1;
	}

	_tprintf(TEXT("[ESCRITOR] Criar uma cópia do pipe '%s' ... (CreateNamedPipe)\n"), PIPE_NAME);
	hEventTemp = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (hEventTemp == NULL) {
		_tprintf(TEXT("[ERRO] Criar Evento! (CreateEvent)"));
		exit(-1);
	}

	hPipe = CreateNamedPipe(PIPE_NAME, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE, 1, 256 * sizeof(TCHAR), 256 * sizeof(TCHAR), 1000, NULL);
	ZeroMemory(&dados.hPipe.ov, sizeof(dados.hPipe.ov));
	dados.hPipe.hInstance = hPipe;
	dados.hPipe.ov.hEvent = hEventTemp;
	dados.hPipe.bool_ = FALSE;
	dados.hEvents = hEventTemp;

	if (ConnectNamedPipe(hPipe, &dados.hPipe.ov)) {
		_tprintf(TEXT("[ERRO] Ligacao ao leitor! (ConnectNamedPipe)\n"));
		exit(-1);
	}

	//criacao da thread
	hThread = CreateThread(NULL, 0, &Thread, &dados, 0, NULL);
	if (hThread == NULL) {
		_tprintf(TEXT("[Erro] ao criar thread!\n"));
		return -1;
	}

	_tprintf(TEXT("[ESCRITOR] Esperar ligacao de um leitor...\n"));
	int offset = WaitForSingleObject(dados.hEvents, INFINITE);
	i = offset - WAIT_OBJECT_0;
	_tprintf(TEXT("[ESCRITOR] Leitor %d chegou...\n"), i);

	while (!dados.terminar) {

		if (GetOverlappedResult(dados.hPipe.hInstance, &dados.hPipe.ov, &nBytes, FALSE))
		{
			ResetEvent(dados.hEvents);
			WaitForSingleObject(dados.hMutex, INFINITE);
			dados.hPipe.bool_ = TRUE;
			ReleaseMutex(dados.hMutex);
		}
	}

	WaitForSingleObject(hThread, INFINITE);
	if (!DisconnectNamedPipe(dados.hPipe.hInstance)) {
		_tprintf(TEXT("[ERRO] Desligar o pipe! (DisconnectNamedPipe)"));
		CloseHandle(dados.hPipe.hInstance);
	}
	CloseHandle(hThread);
	exit(0);
	_tprintf(L"exiting");
	FreeConsole();
}