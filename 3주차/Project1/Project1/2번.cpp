#include <windows.h>
#include <tchar.h>
#include <time.h>
#include <stdlib.h>

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"My Window Class";
LPCTSTR lpszWindowName = L"Window Programming Lab";

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

int g_r;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
    srand((unsigned)time(NULL));

    HWND hWnd;
    MSG Message;
    WNDCLASSEX WndClass;

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
        NULL, NULL, hInstance, NULL
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
    TCHAR lpOut[300];
    TCHAR temp[40];
    RECT rect;

    int width = 800;
    int height = 600;
    int halfHeight = height / 2;
    int colWidth;
    int colCount;
    int dan, n, col;

    switch (uMsg) {

    case WM_CREATE:
        g_r = rand() % 15 + 2;   // 2~16
        return 0;

    case WM_PAINT:
    {
        hDC = BeginPaint(hWnd, &ps);

        colCount = g_r - 1;         // 2단~g_r단 이므로 총 칸 수
        colWidth = width / colCount;

        // ---------------------------
        // 윗줄 : 2단 ~ g_r단
        // ---------------------------
        for (dan = 2; dan <= g_r; dan++) {

            col = dan - 2;

            rect.left = col * colWidth;
            rect.top = 0;
            rect.right = rect.left + colWidth;
            rect.bottom = halfHeight;

            lpOut[0] = L'\0';

            for (n = 1; n <= 9; n++) {
                wsprintf(temp, L"%d*%d=%d\r\n", dan, n, dan * n);
                lstrcat(lpOut, temp);
            }

            DrawText(hDC, lpOut, -1, &rect,
                DT_CENTER | DT_VCENTER);
        }

        // ---------------------------
        // 아랫줄 : g_r단 ~ 2단
        // ---------------------------
        for (dan = g_r; dan >= 2; dan--) {

            col = g_r - dan;

            rect.left = col * colWidth;
            rect.top = halfHeight;
            rect.right = rect.left + colWidth;
            rect.bottom = height;

            lpOut[0] = L'\0';

            for (n = 1; n <= 9; n++) {
                wsprintf(temp, L"%d*%d=%d\r\n", dan, n, dan * n);
                lstrcat(lpOut, temp);
            }

            DrawText(hDC, lpOut, -1, &rect,
                DT_CENTER | DT_VCENTER);
        }

        EndPaint(hWnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}