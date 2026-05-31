#include <windows.h>
#include <tchar.h>
#include <vector>
#include <cmath>
#include "6-2.h"

#pragma comment(lib, "msimg32.lib")

HINSTANCE g_hInst;
HWND g_hWnd = NULL;
HWND g_hDlg = NULL;

LPCTSTR lpszClass = L"My Window Class";
LPCTSTR lpszWindowName = L"Practice 6-2";

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DialogProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);

const double PI = 3.14159265358979323846;

enum PathType
{
    PATH_SINE,
    PATH_SEMI,
    PATH_SPRING,
    PATH_STAIRS
};

enum MoveMode
{
    MOVE_STOP,
    MOVE_X,
    MOVE_Y
};

PathType g_pathType = PATH_SINE;
MoveMode g_moveMode = MOVE_STOP;

bool g_red = false;
bool g_green = false;
bool g_blue = false;
bool g_invert = false;

bool g_circleMoving = false;

int g_offsetX = 0;
int g_offsetY = 0;
int g_dirX = 1;
int g_dirY = 1;

int g_circleIndex = 0;

void AddLinePoints(std::vector<POINT>& pts, POINT a, POINT b)
{
    int dx = b.x - a.x;
    int dy = b.y - a.y;

    int steps = max(abs(dx), abs(dy)) / 4;
    if (steps < 1) steps = 1;

    for (int i = 0; i <= steps; i++)
    {
        double t = (double)i / steps;
        POINT p;
        p.x = (int)(a.x + dx * t);
        p.y = (int)(a.y + dy * t);
        pts.push_back(p);
    }
}

COLORREF GetLineColor()
{
    bool anyColor = g_red || g_green || g_blue;

    if (!anyColor)
        return RGB(0, 0, 0);

    int r = g_red ? 255 : 0;
    int g = g_green ? 255 : 0;
    int b = g_blue ? 255 : 0;

    if (g_invert)
    {
        r = 255 - r;
        g = 255 - g;
        b = 255 - b;
    }

    return RGB(r, g, b);
}

std::vector<POINT> MakePathPoints(RECT rt)
{
    std::vector<POINT> pts;

    int width = rt.right - rt.left;
    int height = rt.bottom - rt.top;

    int margin = 80;
    int baseY = height / 2 + g_offsetY;
    int startX = margin + g_offsetX;
    int pathW = width - margin * 2;

    if (pathW < 100)
        pathW = 100;

    if (g_pathType == PATH_SINE)
    {
        int count = 900;
        int amp = 45;
        int cycle = 4;

        for (int i = 0; i <= count; i++)
        {
            double t = (double)i / count;
            POINT p;
            p.x = startX + (int)(pathW * t);
            p.y = baseY - (int)(amp * sin(2 * PI * cycle * t));
            pts.push_back(p);
        }
    }
    else if (g_pathType == PATH_SEMI)
    {
        int r = 45;
        int circleCount = pathW / (2 * r);

        if (circleCount < 1)
            circleCount = 1;

        // 1단계: 위쪽 반원들을 왼쪽에서 오른쪽으로 먼저 이동
        for (int k = 0; k < circleCount; k++)
        {
            int cx = startX + r + k * 2 * r;

            for (int i = 0; i <= 80; i++)
            {
                double t = (double)i / 80.0;

                // 왼쪽 -> 오른쪽, 위쪽 반원
                double angle = PI - PI * t;

                POINT p;
                p.x = cx + (int)(r * cos(angle));
                p.y = baseY - (int)(r * sin(angle));

                pts.push_back(p);
            }
        }

        // 2단계: 아래쪽 반원들을 오른쪽에서 왼쪽으로 이동
        for (int k = circleCount - 1; k >= 0; k--)
        {
            int cx = startX + r + k * 2 * r;

            for (int i = 0; i <= 80; i++)
            {
                double t = (double)i / 80.0;

                // 오른쪽 -> 왼쪽, 아래쪽 반원
                double angle = PI * t;

                POINT p;
                p.x = cx + (int)(r * cos(angle));
                p.y = baseY + (int)(r * sin(angle));

                pts.push_back(p);
            }
        }
    }
    else if (g_pathType == PATH_SPRING)
    {
        int r = 35;
        int turnSpace = 48;
        int turns = pathW / turnSpace;
        if (turns < 4) turns = 4;

        int count = turns * 120;

        for (int i = 0; i <= count; i++)
        {
            double t = (double)i / count;
            double theta = 2 * PI * turns * t;

            POINT p;
            p.x = startX + (int)(turnSpace * turns * t + r * cos(theta));
            p.y = baseY + (int)(r * sin(theta));

            pts.push_back(p);
        }
    }
    else if (g_pathType == PATH_STAIRS)
    {
        int stepW = 65;
        int stepH = 30;
        int steps = 8;

        POINT cur;
        cur.x = startX;
        cur.y = baseY + 120;

        pts.push_back(cur);

        for (int i = 0; i < steps; i++)
        {
            POINT next1 = cur;
            next1.x += stepW;
            AddLinePoints(pts, cur, next1);

            POINT next2 = next1;
            next2.y -= stepH;
            AddLinePoints(pts, next1, next2);

            cur = next2;
        }
    }

    return pts;
}

void ResetState()
{
    g_pathType = PATH_SINE;
    g_moveMode = MOVE_STOP;

    g_red = false;
    g_green = false;

    g_blue = false;
    g_invert = false;

    g_circleMoving = false;

    g_offsetX = 0;
    g_offsetY = 0;
    g_dirX = 1;
    g_dirY = 1;
    g_circleIndex = 0;

    if (g_hDlg)
    {
        CheckRadioButton(g_hDlg, IDC_RADIO_SINE, IDC_RADIO_STAIRS, IDC_RADIO_SINE);

        CheckDlgButton(g_hDlg, IDC_CHECK_RED, BST_UNCHECKED);
        CheckDlgButton(g_hDlg, IDC_CHECK_GREEN, BST_UNCHECKED);
        CheckDlgButton(g_hDlg, IDC_CHECK_BLUE, BST_UNCHECKED);
        CheckDlgButton(g_hDlg, IDC_CHECK_INVERT, BST_UNCHECKED);
    }

    if (g_hWnd)
        InvalidateRect(g_hWnd, NULL, TRUE);
}

void DrawSemiCirclePath(HDC hDC, RECT rt)
{
    int width = rt.right - rt.left;
    int height = rt.bottom - rt.top;

    int margin = 80;
    int baseY = height / 2 + g_offsetY;
    int startX = margin + g_offsetX;
    int pathW = width - margin * 2;

    int r = 45;  // 원 반지름
    int circleCount = pathW / (2 * r);

    if (circleCount < 1)
        circleCount = 1;

    for (int k = 0; k < circleCount; k++)
    {
        int left = startX + k * 2 * r;
        int top = baseY - r;
        int right = left + 2 * r;
        int bottom = baseY + r;

        // 위 반원
        Arc(hDC,
            left, top, right, bottom,
            left, baseY, right, baseY);

        // 아래 반원
        Arc(hDC,
            left, top, right, bottom,
            right, baseY, left, baseY);
    }
}

void DrawScene(HDC hDC, RECT rt)
{
    int width = rt.right - rt.left;
    int height = rt.bottom - rt.top;

    int baseY = height / 2;
    int centerX = width / 2;

    // 축 그리기
    HPEN axisPen = CreatePen(PS_SOLID, 1, RGB(180, 180, 180));
    HPEN oldPen = (HPEN)SelectObject(hDC, axisPen);

    MoveToEx(hDC, 0, baseY, NULL);
    LineTo(hDC, width, baseY);

    MoveToEx(hDC, centerX, 0, NULL);
    LineTo(hDC, centerX, height);

    SelectObject(hDC, oldPen);
    DeleteObject(axisPen);

    // 원 이동을 위해 pts는 항상 먼저 만들어둔다.
    std::vector<POINT> pts = MakePathPoints(rt);

    COLORREF lineColor = GetLineColor();
    HPEN pathPen = CreatePen(PS_SOLID, 2, lineColor);
    oldPen = (HPEN)SelectObject(hDC, pathPen);

    if (g_pathType == PATH_SEMI)
    {
        DrawSemiCirclePath(hDC, rt);
    }
    else
    {
        if (pts.size() >= 2)
        {
            Polyline(hDC, pts.data(), (int)pts.size());
        }
    }

    SelectObject(hDC, oldPen);
    DeleteObject(pathPen);

    // 원 이동
    if (g_circleMoving && !pts.empty())
    {
        if (g_circleIndex >= (int)pts.size())
            g_circleIndex = 0;

        POINT p = pts[g_circleIndex];

        int r = 28;

        HBRUSH circleBrush = CreateSolidBrush(RGB(255, 255, 220));
        HBRUSH oldBrush = (HBRUSH)SelectObject(hDC, circleBrush);

        HPEN circlePen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
        oldPen = (HPEN)SelectObject(hDC, circlePen);

        Ellipse(hDC, p.x - r, p.y - r, p.x + r, p.y + r);

        SelectObject(hDC, oldPen);
        DeleteObject(circlePen);

        SelectObject(hDC, oldBrush);
        DeleteObject(circleBrush);

        SetBkMode(hDC, TRANSPARENT);

        const wchar_t* text = L"It's moving";
        SIZE size;
        GetTextExtentPoint32(hDC, text, lstrlen(text), &size);

        TextOut(hDC, p.x - size.cx / 2, p.y - size.cy / 2, text, lstrlen(text));
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
    HWND hWnd;
    MSG Message;
    WNDCLASSEX WndClass;

    g_hInst = hInstance;

    WndClass.cbSize = sizeof(WndClass);
    WndClass.style = CS_HREDRAW | CS_VREDRAW;
    WndClass.lpfnWndProc = WndProc;
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
        100,
        100,
        900,
        600,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    g_hWnd = hWnd;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    while (GetMessage(&Message, NULL, 0, 0))
    {
        if (g_hDlg == NULL || !IsDialogMessage(g_hDlg, &Message))
        {
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }
    }

    return (int)Message.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
    switch (iMessage)
    {
    case WM_CREATE:
        g_hDlg = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_CONTROL_DLG), hWnd, DialogProc);
        ShowWindow(g_hDlg, SW_SHOW);

        SetTimer(hWnd, 1, 16, NULL);
        return 0;

    case WM_TIMER:
    {
        bool needDraw = false;

        if (g_moveMode == MOVE_X)
        {
            g_offsetX += g_dirX * 4;

            if (g_offsetX > 130)
            {
                g_offsetX = 130;
                g_dirX = -1;
            }
            else if (g_offsetX < -130)
            {
                g_offsetX = -130;
                g_dirX = 1;
            }

            needDraw = true;
        }
        else if (g_moveMode == MOVE_Y)
        {
            g_offsetY += g_dirY * 4;

            if (g_offsetY > 100)
            {
                g_offsetY = 100;
                g_dirY = -1;
            }
            else if (g_offsetY < -100)
            {
                g_offsetY = -100;
                g_dirY = 1;
            }

            needDraw = true;
        }

        if (g_circleMoving)
        {
            RECT rt;
            GetClientRect(hWnd, &rt);

            std::vector<POINT> pts = MakePathPoints(rt);

            if (!pts.empty())
            {
                g_circleIndex += 5;

                if (g_circleIndex >= (int)pts.size())
                    g_circleIndex = 0;
            }

            needDraw = true;
        }

        if (needDraw)
            InvalidateRect(hWnd, NULL, FALSE);

        return 0;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hDC = BeginPaint(hWnd, &ps);

        RECT rt;
        GetClientRect(hWnd, &rt);

        int width = rt.right - rt.left;
        int height = rt.bottom - rt.top;

        HDC memDC = CreateCompatibleDC(hDC);
        HBITMAP memBitmap = CreateCompatibleBitmap(hDC, width, height);
        HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

        FillRect(memDC, &rt, (HBRUSH)GetStockObject(WHITE_BRUSH));

        DrawScene(memDC, rt);

        BitBlt(hDC, 0, 0, width, height, memDC, 0, 0, SRCCOPY);

        SelectObject(memDC, oldBitmap);
        DeleteObject(memBitmap);
        DeleteDC(memDC);

        EndPaint(hWnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        KillTimer(hWnd, 1);

        if (g_hDlg)
        {
            DestroyWindow(g_hDlg);
            g_hDlg = NULL;
        }

        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hWnd, iMessage, wParam, lParam);
}

INT_PTR CALLBACK DialogProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
    switch (iMessage)
    {
    case WM_INITDIALOG:
        CheckRadioButton(hDlg, IDC_RADIO_SINE, IDC_RADIO_STAIRS, IDC_RADIO_SINE);
        return TRUE;

    case WM_COMMAND:
    {
        int id = LOWORD(wParam);

        switch (id)
        {
        case IDC_RADIO_SINE:
            g_pathType = PATH_SINE;
            g_circleIndex = 0;
            InvalidateRect(g_hWnd, NULL, TRUE);
            return TRUE;

        case IDC_RADIO_SEMI:
            g_pathType = PATH_SEMI;
            g_circleIndex = 0;
            InvalidateRect(g_hWnd, NULL, TRUE);
            return TRUE;

        case IDC_RADIO_SPRING:
            g_pathType = PATH_SPRING;
            g_circleIndex = 0;
            InvalidateRect(g_hWnd, NULL, TRUE);
            return TRUE;

        case IDC_RADIO_STAIRS:
            g_pathType = PATH_STAIRS;
            g_circleIndex = 0;
            InvalidateRect(g_hWnd, NULL, TRUE);
            return TRUE;

        case IDC_CHECK_RED:
        case IDC_CHECK_GREEN:
        case IDC_CHECK_BLUE:
        case IDC_CHECK_INVERT:
            g_red = IsDlgButtonChecked(hDlg, IDC_CHECK_RED) == BST_CHECKED;
            g_green = IsDlgButtonChecked(hDlg, IDC_CHECK_GREEN) == BST_CHECKED;
            g_blue = IsDlgButtonChecked(hDlg, IDC_CHECK_BLUE) == BST_CHECKED;
            g_invert = IsDlgButtonChecked(hDlg, IDC_CHECK_INVERT) == BST_CHECKED;

            InvalidateRect(g_hWnd, NULL, TRUE);
            return TRUE;

        case IDC_BTN_MOVEX:
            g_moveMode = MOVE_X;
            return TRUE;

        case IDC_BTN_MOVEY:
            g_moveMode = MOVE_Y;
            return TRUE;

        case IDC_BTN_STOP:
            g_moveMode = MOVE_STOP;
            g_circleMoving = false;
            return TRUE;

        case IDC_BTN_RESET:
        case IDOK:
            ResetState();
            return TRUE;

        case IDC_BTN_CIRCLE:
            g_circleMoving = true;
            return TRUE;

        case IDCANCEL:
            DestroyWindow(hDlg);
            return TRUE;
        }

        break;
    }

    case WM_CLOSE:
        DestroyWindow(hDlg);
        return TRUE;

    case WM_DESTROY:
        if (g_hDlg == hDlg)
            g_hDlg = NULL;
        return TRUE;
    }

    return FALSE;
}
