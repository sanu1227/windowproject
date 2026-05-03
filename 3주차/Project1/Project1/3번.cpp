#include <windows.h>
#include <tchar.h>
#include <time.h>
#include <stdlib.h>

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"My Window Class";
LPCTSTR lpszWindowName = L"Window Programming Lab";

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

int g_r;
int g_mode;   // 홀짝 판별용 랜덤 수

COLORREF colors[6] = {
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
    TCHAR temp[40];
    RECT rect;

    int width = 800;
    int height = 600;
    int halfHeight = height / 2;
    int colWidth;
    int colCount;
    int dan, n, col;
    int textHeight = 20;   // 한 줄 높이
    int totalHeight;       // 9줄 전체 높이
    int startY;            // 세로 가운데 시작 위치
    int textWidth;

    switch (uMsg) {

    case WM_CREATE:
        g_r = rand() % 15 + 2;   // 2~16
        g_mode = rand() % 100 + 1;   // 홀짝 판별용 랜덤 숫자
        return 0;

    case WM_PAINT:
    {
        hDC = BeginPaint(hWnd, &ps);

        SetBkMode(hDC, TRANSPARENT);

        colCount = g_r - 1;         // 2단~g_r단
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

            totalHeight = 9 * textHeight;
            startY = (halfHeight - totalHeight) / 2;

            for (n = 1; n <= 9; n++) {

                // 홀수: 단별 같은 색
                if (g_mode % 2 == 1) {
                    SetTextColor(hDC, colors[(dan - 2) % 6]);
                }
                // 짝수: 줄별 다른 색
                else {
                    SetTextColor(hDC, colors[(n - 1) % 6]);
                }

                wsprintf(temp, L"%d*%d=%d", dan, n, dan * n);

                textWidth = lstrlen(temp) * 8;  // 대충 문자폭 계산
                rect.left = col * colWidth;
                rect.right = rect.left + colWidth;
                rect.top = startY + (n - 1) * textHeight;
                rect.bottom = rect.top + textHeight;

                DrawText(hDC, temp, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
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

            totalHeight = 9 * textHeight;
            startY = halfHeight + (halfHeight - totalHeight) / 2;

            for (n = 1; n <= 9; n++) {

                // 홀수: 단별 같은 색
                if (g_mode % 2 == 1) {
                    SetTextColor(hDC, colors[(dan - 2) % 6]);
                }
                // 짝수: 줄별 다른 색
                else {
                    SetTextColor(hDC, colors[(n - 1) % 6]);
                }

                wsprintf(temp, L"%d*%d=%d", dan, n, dan * n);

                rect.left = col * colWidth;
                rect.right = rect.left + colWidth;
                rect.top = startY + (n - 1) * textHeight;
                rect.bottom = rect.top + textHeight;

                DrawText(hDC, temp, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
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