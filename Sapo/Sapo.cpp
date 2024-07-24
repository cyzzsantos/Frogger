#include "framework.h"
#include "Project1.h"
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#include <time.h>
#include <iostream>

#define MAX_LOADSTRING 100
#define BUFSIZE 4096
#define SINGLEPLAYER 1
#define MULTIPLAYER 2
#define PIPE_NAME TEXT("\\\\.\\pipe\\teste")

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

int delay = 500;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

typedef struct {
    OVERLAPPED overlapped;
    HANDLE hPipe;
    HANDLE hMutex;
    HANDLE hEventConnect;
    HANDLE hEventDisconnect;
    HANDLE hMoveUpEvent;
    HANDLE hMoveDownEvent;
    HANDLE hMoveLeftEvent;
    HANDLE hMoveRightEvent;
} ThreadData;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PROJECT1, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PROJECT1));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

DWORD WINAPI Disconnect(LPVOID param) {
    ThreadData* dadosPipe = (ThreadData*)param;
    TCHAR buf[BUFSIZE];
    DWORD n;

    HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("UI-DISCONNECT"));
    if (hEvent == NULL)
    {
        _tprintf(TEXT("ERROR - Failed to create the event (UI-DISCONNECT)\n"));
        return -1;
    }

    wcscpy_s(buf, L"dc");
    buf[_tcslen(buf)] = '\0';
    
    WaitForSingleObject(hEvent, INFINITE);
    WaitForSingleObject(dadosPipe->hMutex, INFINITE);
    if (!WriteFile(dadosPipe->hPipe, buf, _tcslen(buf) * sizeof(TCHAR), &n, NULL)) {
        _tprintf(TEXT("[ERRO] Escrever no pipe! (WriteFile)\n"));
        return -1;
    }
    ReleaseMutex(dadosPipe->hMutex);
    ResetEvent(hEvent);
}

DWORD WINAPI Movimento(LPVOID param) {
    HANDLE hMoveUpEvent, hMoveDownEvent, hMoveLeftEvent, hMoveRightEvent;
    ThreadData* movimentoData = (ThreadData*)param;
    DWORD n;
    TCHAR buf[256];

    SetLastError(0);
    hMoveUpEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("UI-HMOVEUPEVENT-2"));
    if (GetLastError() != ERROR_ALREADY_EXISTS) {
        CloseHandle(hMoveUpEvent);
        hMoveUpEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("UI-HMOVEUPEVENT"));
    }
    if (hMoveUpEvent == NULL) {
        _tprintf(TEXT("ERROR - Failed to create an event (hEventMap or hEventEvolve)\n"));
        return -1;
    }

    hMoveDownEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("UI-HMOVEDOWNEVENT-2"));
    if (GetLastError() != ERROR_ALREADY_EXISTS)
        hMoveDownEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("UI-HMOVEDOWNEVENT"));
    if (hMoveDownEvent == NULL) {
        _tprintf(TEXT("ERROR - Failed to create an event (hEventMap or hEventEvolve)\n"));
        return -1;
    }

    hMoveLeftEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("UI-HMOVELEFTEVENT-2"));
    if (GetLastError() != ERROR_ALREADY_EXISTS)
        hMoveLeftEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("UI-HMOVELEFTEVENT"));
    if (hMoveLeftEvent == NULL) {
        _tprintf(TEXT("ERROR - Failed to create an event (hEventMap or hEventEvolve)\n"));
        return -1;
    }

    hMoveRightEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("UI-HMOVERIGHTEVENT-2"));
    if (GetLastError() != ERROR_ALREADY_EXISTS)
        hMoveRightEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("UI-HMOVERIGHTEVENT"));
    if (hMoveRightEvent == NULL) {
        _tprintf(TEXT("ERROR - Failed to create an event (hEventMap or hEventEvolve)\n"));
        return -1;
    }

    HANDLE handles[4] = { hMoveUpEvent, hMoveDownEvent,
                          hMoveLeftEvent, hMoveRightEvent};
    
    while (1)
    {
        int returnValue = WaitForMultipleObjects(4, handles, FALSE, INFINITE) - WAIT_OBJECT_0;

        switch (returnValue)
        {
        case 0:

            wcscpy_s(buf, L"up");
            buf[_tcslen(buf)] = '\0';

            WaitForSingleObject(movimentoData->hMutex, INFINITE);
            if (!WriteFile(movimentoData->hPipe, buf, _tcslen(buf) * sizeof(TCHAR), &n, NULL)) {
                _tprintf(TEXT("[ERRO] Escrever no pipe! (WriteFile)\n"));
            }
            ReleaseMutex(movimentoData->hMutex);
                       
            ResetEvent(handles[0]);
            break;
        case 1:

            wcscpy_s(buf, L"down");
            buf[_tcslen(buf)] = '\0';

            WaitForSingleObject(movimentoData->hMutex, INFINITE);
            if (!WriteFile(movimentoData->hPipe, buf, _tcslen(buf) * sizeof(TCHAR), &n, NULL)) {
                _tprintf(TEXT("[ERRO] Escrever no pipe! (WriteFile)\n"));
            }
            ReleaseMutex(movimentoData->hMutex);

            ResetEvent(handles[1]);
            break;
        case 2:

            wcscpy_s(buf, L"left");
            buf[_tcslen(buf)] = '\0';

            WaitForSingleObject(movimentoData->hMutex, INFINITE);
            if (!WriteFile(movimentoData->hPipe, buf, _tcslen(buf) * sizeof(TCHAR), &n, NULL)) {
                _tprintf(TEXT("[ERRO] Escrever no pipe! (WriteFile)\n"));
            }
            ReleaseMutex(movimentoData->hMutex);

            ResetEvent(handles[2]);
            break;
        case 3:

            wcscpy_s(buf, L"right");
            buf[_tcslen(buf)] = '\0';

            WaitForSingleObject(movimentoData->hMutex, INFINITE);
            if (!WriteFile(movimentoData->hPipe, buf, _tcslen(buf) * sizeof(TCHAR), &n, NULL)) {
                _tprintf(TEXT("[ERRO] Escrever no pipe! (WriteFile)\n"));
            }
            ReleaseMutex(movimentoData->hMutex);

            ResetEvent(handles[3]);
            break;
        }
    }
}

DWORD WINAPI Conectar(LPVOID param) {
    ThreadData* threadData = (ThreadData*)param;
    ThreadData movimentoData;
    HANDLE hThreadDisconnect, hThreadMovimento, hPipe, hEvent, hEventConnected;
    OVERLAPPED overlapped;
    TCHAR buf[BUFSIZE];
    DWORD n;
    int i = 0;

#ifdef UNICODE
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
#endif
    _tprintf(TEXT("[LEITOR] Esperar pelo pipe '%s' (WaitNamedPipe)\n"), PIPE_NAME);
    if (!WaitNamedPipe(PIPE_NAME, NMPWAIT_WAIT_FOREVER)) {
        _tprintf(TEXT("[ERRO] Ligar ao pipe '%s'! (WaitNamedPipe)\n"), PIPE_NAME);
        exit(0);
    }

    hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (hEvent == NULL)
    {
        _tprintf(L"Failed to create event object");
        return -1;
    }
    ZeroMemory(&overlapped, sizeof(overlapped));
    overlapped.hEvent = hEvent;

    hEventConnected = CreateEvent(NULL, TRUE, FALSE, TEXT("UI-HEVENTCONNECTED-2"));
    if (GetLastError() != ERROR_ALREADY_EXISTS) {
        hEventConnected = CreateEvent(NULL, TRUE, FALSE, TEXT("UI-HEVENTCONNECTED"));
    }
    if (hEventConnected == NULL) {
        _tprintf(TEXT("ERROR - Failed to create an event (hEventMap or hEventEvolve)\n"));
        return -1;
    }

    _tprintf(TEXT("[LEITOR] Ligação ao pipe do escritor... (CreateFile)\n"));
    hPipe = CreateFile(PIPE_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
    if (hPipe == INVALID_HANDLE_VALUE)
    {
        _tprintf(TEXT("[ERRO] Ligar ao pipe '%s'! (CreateFile)\n"), PIPE_NAME);
        CloseHandle(hEvent);
        return 1;
    }

    movimentoData.hPipe = hPipe;
    movimentoData.hMutex = threadData->hMutex;
    movimentoData.overlapped = overlapped;

    SetEvent(hEventConnected);
    
    _tprintf(TEXT("[LEITOR] Liguei-me...\n"));

    hThreadMovimento = CreateThread(NULL, 0, Movimento, &movimentoData, 0, NULL);
    if (hThreadMovimento == NULL)
    {
        _tprintf(L"Failed to create movement thread");
        CloseHandle(hPipe);
        CloseHandle(hEvent);
        return 1;
    }

    hThreadDisconnect = CreateThread(NULL, 0, Disconnect, &movimentoData, 0, NULL);
    if (hThreadDisconnect == NULL)
    {
        _tprintf(L"Failed to create disconnect thread");
        CloseHandle(hPipe);
        CloseHandle(hEvent);
        CloseHandle(hThreadMovimento);
        return 1;
    }

    while (1) {
        if (!ReadFile(hPipe, buf, BUFSIZE * sizeof(TCHAR), &n, &overlapped))
        {
            if (GetLastError() != ERROR_IO_PENDING)
            {
                _tprintf(L"Failed to read from named pipe");
                break;
            }
        }

        if (WaitForSingleObject(overlapped.hEvent, INFINITE) == WAIT_OBJECT_0)
        {
            #ifdef _WIN32			
                        system("cls");      // Windows
            #else					
                        system("clear");    // macOS or Linux
            #endif
            _tprintf(L"%s", buf);
            memset(buf, 0, sizeof(buf));
        }

        else
        {
            _tprintf(L"Failed to wait for read operation completion");
            break;
        }
    }

    CloseHandle(overlapped.hEvent);
    CloseHandle(hPipe);
    Sleep(200);

    return 0;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PROJECT1));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_PROJECT1);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HANDLE hThread = nullptr, hEventConnected = nullptr, hEventDisconnect = nullptr, hMutexUser = nullptr,
           hMoveUpEvent, hMoveDownEvent, hMoveLeftEvent, hMoveRightEvent;

    HBRUSH hBrushBackground = CreateSolidBrush(RGB(255, 255, 255));
    const long ID_TITLE = 1000;
    const long ID_USERNAME_LABEL = 1001;
    const long ID_USERNAME_EDIT = 1002;
    const long ID_CHECKBOX_1 = 1003;
    const long ID_CHECKBOX_2 = 1004;
    const long ID_BUTTON = 1005;

    switch (message) {

    case WM_CREATE:
        {
            AllocConsole(); // TEMP
            freopen_s((FILE**)stdout, "CONOUT$", "w", stdout); // TEMP

            ThreadData* threadData = new ThreadData();

            hMoveUpEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("UI-HMOVEUPEVENT"));
            if (GetLastError() == ERROR_ALREADY_EXISTS)
            {
                SetLastError(0);
                hMoveUpEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("UI-HMOVEUPEVENT-2"));
                if (GetLastError() == ERROR_ALREADY_EXISTS)
                    exit(0);
            }
            if (hMoveUpEvent == NULL) {
                _tprintf(TEXT("ERROR - Failed to create an event (hEventMap or hEventEvolve)\n"));
                return -1;
            }
            threadData->hMoveUpEvent = hMoveUpEvent;

            hMoveDownEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("UI-HMOVEDOWNEVENT"));
            if (GetLastError() == ERROR_ALREADY_EXISTS)
            {
                SetLastError(0);
                hMoveDownEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("UI-HMOVEDOWNEVENT-2"));
            }
            if (hMoveDownEvent == NULL) {
                _tprintf(TEXT("ERROR - Failed to create an event (hEventMap or hEventEvolve)\n"));
                return -1;
            }
            threadData->hMoveDownEvent = hMoveDownEvent;

            hMoveLeftEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("UI-HMOVELEFTEVENT"));
            if (GetLastError() == ERROR_ALREADY_EXISTS)
            {
                SetLastError(0);
                hMoveLeftEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("UI-HMOVELEFTEVENT-2"));
            }
            if (hMoveLeftEvent == NULL) {
                _tprintf(TEXT("ERROR - Failed to create an event (hEventMap or hEventEvolve)\n"));
                return -1;
            }
            threadData->hMoveLeftEvent = hMoveLeftEvent;

            hMoveRightEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("UI-HMOVERIGHTEVENT"));
            if (GetLastError() == ERROR_ALREADY_EXISTS)
            {
                SetLastError(0);
                hMoveRightEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("UI-HMOVERIGHTEVENT-2"));
            }
            if (hMoveRightEvent == NULL) {
                _tprintf(TEXT("ERROR - Failed to create an event (hEventMap or hEventEvolve)\n"));
                return -1;
            }
            threadData->hMoveRightEvent = hMoveRightEvent;

            hEventConnected = CreateEvent(NULL, TRUE, FALSE, TEXT("UI-HEVENTCONNECTED"));
            if (GetLastError() == ERROR_ALREADY_EXISTS)
            {
                SetLastError(0);
                hEventConnected = CreateEvent(NULL, TRUE, FALSE, TEXT("UI-HEVENTCONNECTED-2"));
            }
            if (hEventConnected == NULL) {
                _tprintf(TEXT("ERROR - Failed to create an event (hEventMap or hEventEvolve)\n"));
                return -1;
            }
            threadData->hEventConnect = hEventConnected;

            hMutexUser = CreateMutex(NULL, FALSE, TEXT("USER-MUTEX"));
            if (hMutexUser == NULL) {
                _tprintf(TEXT("ERROR - Failed to create the mutex (USER-MUTEX)\n"));
                return -1;
            }
            threadData->hMutex = hMutexUser;

            hEventDisconnect = CreateEvent(NULL, TRUE, FALSE, TEXT("UI-DISCONNECT"));
            if (hEventDisconnect == NULL) {
                _tprintf(TEXT("ERROR - Failed to create an event (hEventMap or hEventEvolve)\n"));
                return -1;
            }
            threadData->hEventDisconnect = hEventDisconnect;

            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(threadData));

            hThread = CreateThread(NULL, 0, Conectar, &threadData, 0, NULL);

            HFONT hFont = CreateFont(36, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Microsoft Sans Serif");

            HFONT hFont2 = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Microsoft Sans Serif");

            HWND hTitle = CreateWindowEx(0, L"STATIC", L"Frogger",
                WS_CHILD | WS_VISIBLE | SS_CENTER, 
                CW_USEDEFAULT, CW_USEDEFAULT, 200, 50,
                hWnd, (HMENU)ID_TITLE, hInst, NULL);

            HWND hUsernameLabel = CreateWindowEx(0, L"STATIC", L"Username:",
                WS_CHILD | WS_VISIBLE | SS_CENTER, 
                CW_USEDEFAULT, CW_USEDEFAULT, 200, 50,
                hWnd, (HMENU)ID_USERNAME_LABEL, hInst, NULL);

            HWND hEdit = CreateWindowEx(0, L"EDIT", L"", 
                WS_CHILD | WS_VISIBLE | WS_BORDER, 
                CW_USEDEFAULT, CW_USEDEFAULT, 200, 50,
                hWnd, (HMENU)ID_USERNAME_EDIT, hInst, NULL);

            HWND hCheckboxMP = CreateWindowEx(0, L"BUTTON", L"Multiplayer",
                WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                CW_USEDEFAULT, CW_USEDEFAULT, 200, 50,
                hWnd, (HMENU)ID_CHECKBOX_1, hInst, NULL);

            HWND hCheckboxSP = CreateWindowEx(0, L"BUTTON", L"Singleplayer",
                WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                CW_USEDEFAULT, CW_USEDEFAULT, 200, 50,
                hWnd, (HMENU)ID_CHECKBOX_2, hInst, NULL);

            HWND hButton = CreateWindowEx(0, L"BUTTON", L"Confirmar",
                WS_CHILD | WS_VISIBLE ,
                CW_USEDEFAULT, CW_USEDEFAULT, 100, 50,
                hWnd, (HMENU)ID_BUTTON, hInst, NULL);

            SendMessage(hTitle, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hUsernameLabel, WM_SETFONT, (WPARAM)hFont2, TRUE);
    }
        break;

    case WM_CTLCOLORSTATIC:
        {
            HDC hdcStatic = (HDC)wParam;
            SetBkMode(hdcStatic, TRANSPARENT);
            return (INT_PTR)hBrushBackground;
        }
        break;

    case WM_SIZE:
        {
            RECT clientRect;
            GetClientRect(hWnd, &clientRect);

            HWND hTitle = GetDlgItem(hWnd, ID_TITLE);
            HWND hUsernameLabel = GetDlgItem(hWnd, ID_USERNAME_LABEL);
            HWND hEdit = GetDlgItem(hWnd, ID_USERNAME_EDIT);
            HWND hCheckboxMP = GetDlgItem(hWnd, ID_CHECKBOX_1);
            HWND hCheckboxSP = GetDlgItem(hWnd, ID_CHECKBOX_2);
            HWND hButton = GetDlgItem(hWnd, ID_BUTTON);

            SetWindowPos(hTitle, NULL,
                (clientRect.right - clientRect.left) / 2 - 100,  // Center horizontally
                (clientRect.bottom - clientRect.top) / 2 - 150,  // Center vertically
                200, 50,
                SWP_NOZORDER);

            SetWindowPos(hUsernameLabel, NULL,
                (clientRect.right - clientRect.left) / 2 - 100, // Center horizontally
                (clientRect.bottom - clientRect.top) / 2 - 100,  // Center vertically
                200, 50,
                SWP_NOZORDER);

            SetWindowPos(hEdit, NULL,
                (clientRect.right - clientRect.left) / 2 - 100, // Center horizontally
                (clientRect.bottom - clientRect.top) / 2 - 70,  // Center vertically
                200, 20,
                SWP_NOZORDER);

            SetWindowPos(hCheckboxMP, NULL,
                (clientRect.right - clientRect.left) / 2 - 100, // Center horizontally
                (clientRect.bottom - clientRect.top) / 2 - 40,  // Center vertically
                100, 20,
                SWP_NOZORDER);

            SetWindowPos(hCheckboxSP, NULL,
                (clientRect.right - clientRect.left) / 2,       // Center horizontally
                (clientRect.bottom - clientRect.top) / 2 - 40,  // Center vertically
                120, 20,
                SWP_NOZORDER);

            SetWindowPos(hButton, NULL,
                (clientRect.right - clientRect.left) / 2 - 50 , // Center horizontally
                (clientRect.bottom - clientRect.top) / 2,       // Center vertically
                100, 20,
                SWP_NOZORDER);
        }
        break;

    case WM_COMMAND:
        {
            ThreadData* threadData = (ThreadData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
            int wmId = LOWORD(wParam);
            int wmEvent = HIWORD(wParam);

            HWND hTitle = GetDlgItem(hWnd, ID_TITLE);
            HWND hUsernameLabel = GetDlgItem(hWnd, ID_USERNAME_LABEL);
            HWND hEdit = GetDlgItem(hWnd, ID_USERNAME_EDIT);
            HWND hCheckboxMP = GetDlgItem(hWnd, ID_CHECKBOX_1);
            HWND hCheckboxSP = GetDlgItem(hWnd, ID_CHECKBOX_2);
            HWND hButton = GetDlgItem(hWnd, ID_BUTTON);

            // Parse the menu selections:
            switch (wmId)
            {

            case ID_CHECKBOX_1:
            {
                if (wmEvent == BN_CLICKED)
                {
                    // Checkbox 1 clicked
                    if (SendMessage(hCheckboxMP, BM_GETCHECK, 0, 0) == BST_CHECKED)
                    {
                        // uncheck Checkbox 2
                        SendMessage(hCheckboxSP, BM_SETCHECK, BST_UNCHECKED, 0);
                    }
                }
            }
            break;

            case ID_CHECKBOX_2:
            {
                if (wmEvent == BN_CLICKED)
                {
                    // Checkbox 2 clicked
                    if (SendMessage(hCheckboxMP, BM_GETCHECK, 0, 0) == BST_CHECKED)
                    {
                        // uncheck Checkbox 1
                        SendMessage(hCheckboxMP, BM_SETCHECK, BST_UNCHECKED, 0);
                    }
                }
            }
            break;

            case ID_BUTTON:
            {
                if (wmEvent == BN_CLICKED && 
                    (SendMessage(hCheckboxMP, BM_GETCHECK, 0, 0) || SendMessage(hCheckboxSP, BM_GETCHECK, 0, 0)) &&
                    GetWindowTextLength(hEdit) > 0)
                {
                    DestroyWindow(hButton);
                    DestroyWindow(hUsernameLabel);
                    DestroyWindow(hEdit);
                    DestroyWindow(hCheckboxSP);
                    DestroyWindow(hCheckboxMP);

                    SetWindowText(hTitle, L"Conectando...");
                    
                    WaitForSingleObject(threadData->hEventConnect, INFINITE);
                    DestroyWindow(hTitle);
                    ResetEvent(threadData->hEventConnect);

                }
            }
            break;

            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;

            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;

            default:
                return DefWindowProc(hWnd, message, wParam, lParam);

            }
        }
        break;

    case WM_KEYDOWN:
        {
            ThreadData* threadData = (ThreadData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
            int keyCode = wParam;
            switch (keyCode)
            {
                case VK_UP:
                    SetEvent(threadData->hMoveUpEvent);
                    Sleep(100);
                    break;

                case VK_DOWN:
                    SetEvent(threadData->hMoveDownEvent);
                    Sleep(100);
                    break;

                case VK_LEFT:
                    SetEvent(threadData->hMoveLeftEvent);
                    Sleep(100);
                    break;

                case VK_RIGHT:
                    SetEvent(threadData->hMoveRightEvent);
                    Sleep(100);
                    break;
            }
        }   
        break;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
        }
        break;

    case WM_GETMINMAXINFO:
        {
            MINMAXINFO* pMinMaxInfo = (MINMAXINFO*)lParam;
            pMinMaxInfo->ptMinTrackSize.x = pMinMaxInfo->ptMaxTrackSize.x = 620;
            pMinMaxInfo->ptMinTrackSize.y = pMinMaxInfo->ptMaxTrackSize.y = 440;
        }
        break;

    case WM_CLOSE: 
        {
            ThreadData* threadData = (ThreadData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
            SetEvent(threadData->hEventDisconnect);
            FreeConsole(); // TEMP
            DestroyWindow(hWnd);
        }
        break;

    case WM_DESTROY:
        {
            PostQuitMessage(0);
        }
        break;

    default: 
        {
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
        break;
    }
    return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
