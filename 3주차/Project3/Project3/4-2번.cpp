#include <windows.h>
#include <tchar.h>
#include <time.h>
#include <stdlib.h>

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"My Window Class";
LPCTSTR lpszWindowName = L"Window Programming Lab";

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

COLORREF colors[7] = {
    RGB(0, 0, 0),       // 검정
    RGB(255, 0, 0),     // 빨강
    RGB(0, 0, 255),     // 파랑
    RGB(0, 180, 0),     // 초록
    RGB(180, 0, 180),   // 보라
    RGB(255, 128, 0),   // 주황
    RGB(128, 64, 0)     // 갈색
};

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
        lpszClass, lpszWindowName, WS_OVERLAPPEDWINDOW,
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
    TCHAR temp[40];
    RECT rect;

    RECT client;
    GetClientRect(hWnd, &client);

    int winW = client.right;
    int winH = client.bottom;

    int halfW = winW / 2;
    int halfH = winH / 2;

    int minW = 101;
    int maxW = halfW - 1;
    if (maxW < minW) maxW = minW;
    int w = rand() % (maxW - minW + 1) + minW;

    int minH = 101;
    int maxH = halfH - 1;
    if (maxH < minH) maxH = minH;
    int h = rand() % (maxH - minH + 1) + minH;

    int minX = 1;
    int maxX = halfW - 1;
    if (maxX > winW - w - 20)
        maxX = winW - w - 20;
    if (maxX < minX) maxX = minX;
    int x = rand() % (maxX - minX + 1) + minX;

    int minY = 1;
    int maxY = halfH - 1;
    if (maxY > winH - h - 50)
        maxY = winH - h - 50;
    if (maxY < minY) maxY = minY;
    int y = rand() % (maxY - minY + 1) + minY;

    int topColor = rand() % 7;
    int bottomColor = rand() % 7;
    int leftColor = rand() % 7;
    int rightColor = rand() % 7;

    switch (uMsg) {

    case WM_CREATE:
        return 0;

    case WM_PAINT:
        hDC = BeginPaint(hWnd, &ps);

        rect.left = x;
        rect.top = y;
        rect.right = x + w;
        rect.bottom = y + h;

        for (int i = x; i < x + w; i += 20) {
            int num = rand() % 26 + 65;
            wsprintf(temp, L"%c", num);

            rect.left = i;
            rect.top = y;
            rect.right = i + 20;
            rect.bottom = y + 30;

            SetTextColor(hDC, colors[topColor]);
            DrawText(hDC, temp, -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
        }

        for (int i = x; i < x + w; i += 20) {
            int num = rand() % 26 + 65;
            wsprintf(temp, L"%c", num);

            rect.left = i;
            rect.top = y + h + 30;
            rect.right = i + 20;
            rect.bottom = y + h + 50;

            SetTextColor(hDC, colors[bottomColor]);
            DrawText(hDC, temp, -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
        }

        for (int i = y + 30; i < y + h; i += 30) {
            int num = rand() % 26 + 65;
            wsprintf(temp, L"%c", num);

            rect.left = x;
            rect.top = i;
            rect.right = x + 20;
            rect.bottom = i + 20;

            SetTextColor(hDC, colors[leftColor]);
            DrawText(hDC, temp, -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
        }

        for (int i = y + 30; i < y + h; i += 30) {
            int num = rand() % 26 + 65;
            wsprintf(temp, L"%c", num);

            rect.left = x + w;
            rect.top = i;
            rect.right = x + w + 20;
            rect.bottom = i + 20;

            SetTextColor(hDC, colors[rightColor]);
            DrawText(hDC, temp, -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
        }

        EndPaint(hWnd, &ps);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}