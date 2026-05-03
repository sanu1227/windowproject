#include <windows.h>
#include <tchar.h>
#include <time.h>
#include <stdlib.h>

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"My Window Class";
LPCTSTR lpszWindowName = L"Window Programming Lab";

int arr[15];
int arr2[15];

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

int overlap(int x, int y, int count)
{
    int j;

    /* 0번 고정 좌표 (400, 300) 검사 */
    if ((x - 400 < 120 && x - 400 > -120) &&
        (y - 300 < 25 && y - 300 > -25))
        return 1;

    /* 기존 좌표들과 비교 */
    for (j = 0; j < count; j++) {
        if ((x - arr[j] < 120 && x - arr[j] > -120) &&
            (y - arr2[j] < 25 && y - arr2[j] > -25))
            return 1;
    }

    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
    HWND hWnd;
    MSG Message;
    WNDCLASSEX WndClass;

    srand((unsigned int)time(NULL));

    g_hInst = hInstance;
    WndClass.cbSize = sizeof(WndClass);
    WndClass.style = CS_HREDRAW | CS_VREDRAW;
    WndClass.lpfnWndProc = (WNDPROC)WndProc;
    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = 0;
    WndClass.hInstance = hInstance;
    WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    WndClass.lpszMenuName = NULL;
    WndClass.lpszClassName = lpszClass;
    WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    RegisterClassEx(&WndClass);

    hWnd = CreateWindow(
        lpszClass, lpszWindowName, WS_SYSMENU,
        0, 0, 800, 600,
        NULL, (HMENU)NULL, hInstance, NULL
    );

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    while (GetMessage(&Message, 0, 0, 0)) {
        TranslateMessage(&Message);
        DispatchMessage(&Message);
    }

    return (int)Message.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hDC;
    int num;
    TCHAR lpOut[100];

    switch (uMsg) {

    case WM_PAINT:
    {
        int i;

        hDC = BeginPaint(hWnd, &ps);

        TextOut(hDC, 400, 300, L"0: (400, 300)", lstrlen(L"0: (400, 300)"));

        for (i = 0; i < 15; i++) {
            int x, y;
            int retry = 0;

            do {
                x = rand() % 650;   /* 너무 오른쪽 끝에 붙지 않게 */
                y = rand() % 550;   /* 너무 아래쪽 끝에 붙지 않게 */
                retry++;
            } while (overlap(x, y, i) && retry < 1000);

            arr[i] = x;
            arr2[i] = y;

            num = i + 1;
            wsprintf(lpOut, L"%d: (%d, %d)", num, arr[i], arr2[i]);
            TextOut(hDC, arr[i], arr2[i], lpOut, lstrlen(lpOut));
        }

        EndPaint(hWnd, &ps);
    }
    break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}