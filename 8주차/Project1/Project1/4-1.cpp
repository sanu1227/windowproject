#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"My Window Class";
LPCTSTR lpszWindowName = L"Window Programming Lab";

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

#define PI 3.141592
#define TIMER_ID 1
#define TIMER_INTERVAL 30

#define ORBIT_CIRCLE   0
#define ORBIT_RECT     1
#define ORBIT_TRIANGLE 2

#define AREA1 1
#define AREA2 2
#define AREA3 3
#define AREA4 4

#define ID_GAME_START   1001
#define ID_GAME_STOP    1002
#define ID_GAME_QUIT    1003

#define ID_SELECT_1     2001
#define ID_SELECT_2     2002
#define ID_SELECT_3     2003
#define ID_SELECT_4     2004

#define ID_SPEED_FAST   3001
#define ID_SPEED_MEDIUM 3002
#define ID_SPEED_SLOW   3003

#define ID_COLOR_CYAN    4001
#define ID_COLOR_MAGENTA 4002
#define ID_COLOR_YELLOW  4003

#define ID_SHAPE_CIRCLE   5001
#define ID_SHAPE_RECT     5002
#define ID_SHAPE_TRIANGLE 5003

double g_angle[5] = { 0.0, 0.0, 0.0, 0.0, 0.0 };
double g_speed[5] = { 0.0, 0.08, 0.08, 0.08, 0.08 };

int g_orbitShape[5] = {
    0,
    ORBIT_CIRCLE,
    ORBIT_CIRCLE,
    ORBIT_CIRCLE,
    ORBIT_CIRCLE
};

COLORREF g_moveColor[5];
COLORREF g_centerColor;

int g_selectedArea = AREA1;
int g_isRunning = 0;

HMENU MakeMenuBar()
{
    HMENU hMenuBar;
    HMENU hGameMenu;
    HMENU hSelectionMenu;
    HMENU hOptionMenu;
    HMENU hSpeedMenu;
    HMENU hColorMenu;
    HMENU hShapeMenu;

    hMenuBar = CreateMenu();
    hGameMenu = CreateMenu();
    hSelectionMenu = CreateMenu();
    hOptionMenu = CreateMenu();
    hSpeedMenu = CreateMenu();
    hColorMenu = CreateMenu();
    hShapeMenu = CreateMenu();

    AppendMenu(hGameMenu, MF_STRING, ID_GAME_START, L"Start");
    AppendMenu(hGameMenu, MF_STRING, ID_GAME_STOP, L"Stop");
    AppendMenu(hGameMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hGameMenu, MF_STRING, ID_GAME_QUIT, L"Quit");

    AppendMenu(hSelectionMenu, MF_STRING, ID_SELECT_1, L"1");
    AppendMenu(hSelectionMenu, MF_STRING, ID_SELECT_2, L"2");
    AppendMenu(hSelectionMenu, MF_STRING, ID_SELECT_3, L"3");
    AppendMenu(hSelectionMenu, MF_STRING, ID_SELECT_4, L"4");

    AppendMenu(hSpeedMenu, MF_STRING, ID_SPEED_FAST, L"Fast");
    AppendMenu(hSpeedMenu, MF_STRING, ID_SPEED_MEDIUM, L"Medium");
    AppendMenu(hSpeedMenu, MF_STRING, ID_SPEED_SLOW, L"Slow");

    AppendMenu(hColorMenu, MF_STRING, ID_COLOR_CYAN, L"Cyan");
    AppendMenu(hColorMenu, MF_STRING, ID_COLOR_MAGENTA, L"Magenta");
    AppendMenu(hColorMenu, MF_STRING, ID_COLOR_YELLOW, L"Yellow");

    AppendMenu(hShapeMenu, MF_STRING, ID_SHAPE_CIRCLE, L"Circle");
    AppendMenu(hShapeMenu, MF_STRING, ID_SHAPE_RECT, L"Rectangle");
    AppendMenu(hShapeMenu, MF_STRING, ID_SHAPE_TRIANGLE, L"Triangle");

    AppendMenu(hOptionMenu, MF_POPUP, (UINT_PTR)hSpeedMenu, L"Speed");
    AppendMenu(hOptionMenu, MF_POPUP, (UINT_PTR)hColorMenu, L"Color");
    AppendMenu(hOptionMenu, MF_POPUP, (UINT_PTR)hShapeMenu, L"Shape");

    AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hGameMenu, L"Game");
    AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hSelectionMenu, L"Selection");
    AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hOptionMenu, L"Option");

    return hMenuBar;
}

POINT GetMovePos(int shape, int cx, int cy, int size, double angle)
{
    POINT p;
    double t;
    double ratio;
    POINT pt[3];

    while (angle < 0.0) {
        angle += PI * 2.0;
    }

    while (angle >= PI * 2.0) {
        angle -= PI * 2.0;
    }

    if (shape == ORBIT_CIRCLE) {
        p.x = cx + (int)(size * cos(angle));
        p.y = cy + (int)(size * sin(angle));
    }
    else if (shape == ORBIT_RECT) {
        ratio = angle / (PI * 2.0);

        if (ratio < 0.25) {
            t = ratio / 0.25;
            p.x = cx - size + (int)(size * 2 * t);
            p.y = cy - size;
        }
        else if (ratio < 0.5) {
            t = (ratio - 0.25) / 0.25;
            p.x = cx + size;
            p.y = cy - size + (int)(size * 2 * t);
        }
        else if (ratio < 0.75) {
            t = (ratio - 0.5) / 0.25;
            p.x = cx + size - (int)(size * 2 * t);
            p.y = cy + size;
        }
        else {
            t = (ratio - 0.75) / 0.25;
            p.x = cx - size;
            p.y = cy + size - (int)(size * 2 * t);
        }
    }
    else {
        pt[0].x = cx;
        pt[0].y = cy - size;

        pt[1].x = cx - size;
        pt[1].y = cy + size;

        pt[2].x = cx + size;
        pt[2].y = cy + size;

        ratio = angle / (PI * 2.0);

        if (ratio < 1.0 / 3.0) {
            t = ratio / (1.0 / 3.0);
            p.x = pt[0].x + (int)((pt[1].x - pt[0].x) * t);
            p.y = pt[0].y + (int)((pt[1].y - pt[0].y) * t);
        }
        else if (ratio < 2.0 / 3.0) {
            t = (ratio - 1.0 / 3.0) / (1.0 / 3.0);
            p.x = pt[1].x + (int)((pt[2].x - pt[1].x) * t);
            p.y = pt[1].y + (int)((pt[2].y - pt[1].y) * t);
        }
        else {
            t = (ratio - 2.0 / 3.0) / (1.0 / 3.0);
            p.x = pt[2].x + (int)((pt[0].x - pt[2].x) * t);
            p.y = pt[2].y + (int)((pt[0].y - pt[2].y) * t);
        }
    }

    return p;
}

void DrawSelectedBorder(HDC hDC, RECT rect, int area)
{
    HPEN hPen;
    HPEN oldPen;
    HBRUSH oldBrush;
    RECT border;
    int midx;
    int midy;

    midx = (rect.left + rect.right) / 2;
    midy = (rect.top + rect.bottom) / 2;

    if (area == AREA1) {
        border.left = rect.left;
        border.top = rect.top;
        border.right = midx;
        border.bottom = midy;
    }
    else if (area == AREA2) {
        border.left = midx;
        border.top = rect.top;
        border.right = rect.right;
        border.bottom = midy;
    }
    else if (area == AREA3) {
        border.left = rect.left;
        border.top = midy;
        border.right = midx;
        border.bottom = rect.bottom;
    }
    else {
        border.left = midx;
        border.top = midy;
        border.right = rect.right;
        border.bottom = rect.bottom;
    }

    hPen = CreatePen(PS_SOLID, 4, RGB(0, 0, 255));
    oldPen = (HPEN)SelectObject(hDC, hPen);
    oldBrush = (HBRUSH)SelectObject(hDC, GetStockObject(HOLLOW_BRUSH));

    Rectangle(hDC, border.left, border.top, border.right, border.bottom);

    SelectObject(hDC, oldBrush);
    SelectObject(hDC, oldPen);
    DeleteObject(hPen);
}

void DrawOneArea(HDC hDC, int area, int cx, int cy)
{
    int orbitSize = 100;
    int centerSize = 10;
    int moveSize = 14;

    POINT movePos;
    POINT tri[3];

    HBRUSH hBrush;
    HBRUSH oldBrush;

    if (g_orbitShape[area] == ORBIT_CIRCLE) {
        Ellipse(hDC, cx - orbitSize, cy - orbitSize, cx + orbitSize, cy + orbitSize);
    }
    else if (g_orbitShape[area] == ORBIT_RECT) {
        Rectangle(hDC, cx - orbitSize, cy - orbitSize, cx + orbitSize, cy + orbitSize);
    }
    else {
        tri[0].x = cx;
        tri[0].y = cy - orbitSize;

        tri[1].x = cx - orbitSize;
        tri[1].y = cy + orbitSize;

        tri[2].x = cx + orbitSize;
        tri[2].y = cy + orbitSize;

        Polygon(hDC, tri, 3);
    }

    hBrush = CreateSolidBrush(g_centerColor);
    oldBrush = (HBRUSH)SelectObject(hDC, hBrush);

    Ellipse(hDC,
        cx - centerSize,
        cy - centerSize,
        cx + centerSize,
        cy + centerSize
    );

    SelectObject(hDC, oldBrush);
    DeleteObject(hBrush);

    movePos = GetMovePos(
        g_orbitShape[area],
        cx,
        cy,
        orbitSize,
        g_angle[area]
    );

    hBrush = CreateSolidBrush(g_moveColor[area]);
    oldBrush = (HBRUSH)SelectObject(hDC, hBrush);

    Ellipse(hDC,
        movePos.x - moveSize,
        movePos.y - moveSize,
        movePos.x + moveSize,
        movePos.y + moveSize
    );

    SelectObject(hDC, oldBrush);
    DeleteObject(hBrush);
}

void DrawAll(HDC hDC, HWND hWnd)
{
    RECT rect;
    int midx;
    int midy;

    GetClientRect(hWnd, &rect);

    midx = (rect.left + rect.right) / 2;
    midy = (rect.top + rect.bottom) / 2;

    MoveToEx(hDC, midx, rect.top, NULL);
    LineTo(hDC, midx, rect.bottom);

    MoveToEx(hDC, rect.left, midy, NULL);
    LineTo(hDC, rect.right, midy);

    DrawSelectedBorder(hDC, rect, g_selectedArea);

    DrawOneArea(hDC, AREA1, midx / 2, midy / 2);
    DrawOneArea(hDC, AREA2, midx + midx / 2, midy / 2);
    DrawOneArea(hDC, AREA3, midx / 2, midy + midy / 2);
    DrawOneArea(hDC, AREA4, midx + midx / 2, midy + midy / 2);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpszCmdParam, int nCmdShow)
{
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
        lpszClass,
        lpszWindowName,
        WS_OVERLAPPEDWINDOW,
        0,
        0,
        800,
        600,
        NULL,
        MakeMenuBar(),
        hInstance,
        NULL
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

    switch (uMsg) {
    case WM_CREATE:
        srand((unsigned int)time(NULL));

        g_centerColor = RGB(rand() % 256, rand() % 256, rand() % 256);

        g_moveColor[AREA1] = RGB(0, 255, 255);
        g_moveColor[AREA2] = RGB(255, 0, 255);
        g_moveColor[AREA3] = RGB(255, 255, 0);
        g_moveColor[AREA4] = RGB(0, 255, 255);

        g_isRunning = 0;

        InvalidateRect(hWnd, NULL, TRUE);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_GAME_START:
            if (g_isRunning == 0) {
                g_isRunning = 1;
                SetTimer(hWnd, TIMER_ID, TIMER_INTERVAL, NULL);
            }
            break;

        case ID_GAME_STOP:
            g_isRunning = 0;
            KillTimer(hWnd, TIMER_ID);
            break;

        case ID_GAME_QUIT:
            DestroyWindow(hWnd);
            break;

        case ID_SELECT_1:
            g_selectedArea = AREA1;
            break;

        case ID_SELECT_2:
            g_selectedArea = AREA2;
            break;

        case ID_SELECT_3:
            g_selectedArea = AREA3;
            break;

        case ID_SELECT_4:
            g_selectedArea = AREA4;
            break;

        case ID_SPEED_FAST:
            g_speed[g_selectedArea] = 0.16;
            break;

        case ID_SPEED_MEDIUM:
            g_speed[g_selectedArea] = 0.08;
            break;

        case ID_SPEED_SLOW:
            g_speed[g_selectedArea] = 0.035;
            break;

        case ID_COLOR_CYAN:
            g_moveColor[g_selectedArea] = RGB(0, 255, 255);
            break;

        case ID_COLOR_MAGENTA:
            g_moveColor[g_selectedArea] = RGB(255, 0, 255);
            break;

        case ID_COLOR_YELLOW:
            g_moveColor[g_selectedArea] = RGB(255, 255, 0);
            break;

        case ID_SHAPE_CIRCLE:
            g_orbitShape[g_selectedArea] = ORBIT_CIRCLE;
            break;

        case ID_SHAPE_RECT:
            g_orbitShape[g_selectedArea] = ORBIT_RECT;
            break;

        case ID_SHAPE_TRIANGLE:
            g_orbitShape[g_selectedArea] = ORBIT_TRIANGLE;
            break;
        }

        InvalidateRect(hWnd, NULL, TRUE);
        break;

    case WM_KEYDOWN:
        if (wParam == '1') {
            g_selectedArea = AREA1;
        }
        else if (wParam == '2') {
            g_selectedArea = AREA2;
        }
        else if (wParam == '3') {
            g_selectedArea = AREA3;
        }
        else if (wParam == '4') {
            g_selectedArea = AREA4;
        }

        InvalidateRect(hWnd, NULL, TRUE);
        break;

    case WM_TIMER:
        g_centerColor = RGB(rand() % 256, rand() % 256, rand() % 256);

        g_angle[AREA1] += g_speed[AREA1];
        g_angle[AREA2] += g_speed[AREA2];
        g_angle[AREA3] += g_speed[AREA3];
        g_angle[AREA4] += g_speed[AREA4];

        if (g_angle[AREA1] >= PI * 2.0) {
            g_angle[AREA1] -= PI * 2.0;
        }

        if (g_angle[AREA2] >= PI * 2.0) {
            g_angle[AREA2] -= PI * 2.0;
        }

        if (g_angle[AREA3] >= PI * 2.0) {
            g_angle[AREA3] -= PI * 2.0;
        }

        if (g_angle[AREA4] >= PI * 2.0) {
            g_angle[AREA4] -= PI * 2.0;
        }

        InvalidateRect(hWnd, NULL, TRUE);
        break;

    case WM_PAINT:
        hDC = BeginPaint(hWnd, &ps);
        DrawAll(hDC, hWnd);
        EndPaint(hWnd, &ps);
        break;

    case WM_DESTROY:
        KillTimer(hWnd, TIMER_ID);
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}