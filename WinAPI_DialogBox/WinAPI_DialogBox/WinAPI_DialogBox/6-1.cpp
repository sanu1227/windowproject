#include <windows.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>
#include "resource.h"

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

enum ShapeType
{
    SHAPE_RECT,
    SHAPE_CIRCLE
};

enum WorkMode
{
    MODE_NONE,
    MODE_DRAW,
    MODE_MOVE
};

const int PANEL_WIDTH = 180;
const int TIMER_ID = 1;
const int TIMER_INTERVAL = 16;

HINSTANCE g_hInst;

ShapeType g_shape = SHAPE_RECT;
WorkMode g_mode = MODE_NONE;

int g_halfSize = 45;
bool g_gridOn = false;
COLORREF g_fillColor = RGB(255, 0, 0);

POINT g_center = { 350, 250 };
bool g_hasShape = false;

std::vector<POINT> g_path;
bool g_isDrawing = false;

bool g_isMoving = false;
int g_targetIndex = 1;
double g_curX = 0;
double g_curY = 0;
double g_speed = 5.0;

int ClampInt(int value, int minValue, int maxValue)
{
    if (value < minValue) return minValue;
    if (value > maxValue) return maxValue;
    return value;
}

RECT GetCanvasRect(HWND hDlg)
{
    RECT rc;
    GetClientRect(hDlg, &rc);
    rc.left = PANEL_WIDTH;
    return rc;
}

void UpdateSpeedText(HWND hDlg)
{
    wchar_t buffer[64];
    swprintf_s(buffer, L"Speed: %.0f", g_speed);
    SetDlgItemText(hDlg, IDC_STATIC_SPEED, buffer);
}

void ReadOptions(HWND hDlg)
{
    if (IsDlgButtonChecked(hDlg, IDC_RAD_CIRCLE) == BST_CHECKED)
        g_shape = SHAPE_CIRCLE;
    else
        g_shape = SHAPE_RECT;

    if (IsDlgButtonChecked(hDlg, IDC_RAD_SMALL) == BST_CHECKED)
        g_halfSize = 25;
    else if (IsDlgButtonChecked(hDlg, IDC_RAD_LARGE) == BST_CHECKED)
        g_halfSize = 70;
    else
        g_halfSize = 45;

    g_gridOn = IsDlgButtonChecked(hDlg, IDC_RAD_GRID_ON) == BST_CHECKED;

    int r = IsDlgButtonChecked(hDlg, IDC_CHK_RED) == BST_CHECKED ? 255 : 0;
    int g = IsDlgButtonChecked(hDlg, IDC_CHK_GREEN) == BST_CHECKED ? 255 : 0;
    int b = IsDlgButtonChecked(hDlg, IDC_CHK_BLUE) == BST_CHECKED ? 255 : 0;

    if (r == 0 && g == 0 && b == 0)
    {
        g_fillColor = RGB(255, 255, 255);
    }
    else
    {
        g_fillColor = RGB(r, g, b);
    }
}

void RandomizeShapePosition(HWND hDlg)
{
    RECT rc = GetCanvasRect(hDlg);

    int margin = g_halfSize + 15;

    int minX = rc.left + margin;
    int maxX = rc.right - margin;
    int minY = rc.top + margin;
    int maxY = rc.bottom - margin;

    if (maxX <= minX) maxX = minX + 1;
    if (maxY <= minY) maxY = minY + 1;

    g_center.x = minX + rand() % (maxX - minX + 1);
    g_center.y = minY + rand() % (maxY - minY + 1);

    g_curX = g_center.x;
    g_curY = g_center.y;

    g_hasShape = true;
    g_path.clear();

    g_isMoving = false;
    g_isDrawing = false;
    g_mode = MODE_NONE;

    KillTimer(hDlg, TIMER_ID);
}

void DrawGrid(HDC hdc, RECT rc)
{
    HPEN hPen = CreatePen(PS_DOT, 1, RGB(210, 210, 210));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

    const int gap = 30;

    for (int x = rc.left; x <= rc.right; x += gap)
    {
        MoveToEx(hdc, x, rc.top, NULL);
        LineTo(hdc, x, rc.bottom);
    }

    for (int y = rc.top; y <= rc.bottom; y += gap)
    {
        MoveToEx(hdc, rc.left, y, NULL);
        LineTo(hdc, rc.right, y);
    }

    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
}

void DrawPath(HDC hdc)
{
    if (g_path.size() < 2)
        return;

    HPEN hPen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

    MoveToEx(hdc, g_path[0].x, g_path[0].y, NULL);

    for (size_t i = 1; i < g_path.size(); ++i)
    {
        LineTo(hdc, g_path[i].x, g_path[i].y);
    }

    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
}

void DrawShape(HDC hdc)
{
    if (!g_hasShape)
        return;

    HBRUSH hBrush = CreateSolidBrush(g_fillColor);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);

    HPEN hPen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

    int left = g_center.x - g_halfSize;
    int top = g_center.y - g_halfSize;
    int right = g_center.x + g_halfSize;
    int bottom = g_center.y + g_halfSize;

    if (g_shape == SHAPE_RECT)
    {
        Rectangle(hdc, left, top, right, bottom);
    }
    else
    {
        Ellipse(hdc, left, top, right, bottom);
    }

    SelectObject(hdc, hOldPen);
    SelectObject(hdc, hOldBrush);

    DeleteObject(hPen);
    DeleteObject(hBrush);
}

bool IsNearShapeCenter(int x, int y)
{
    int dx = x - g_center.x;
    int dy = y - g_center.y;

    double dist = sqrt((double)(dx * dx + dy * dy));

    return dist <= 20;
}

void AddPathPoint(HWND hDlg, int x, int y)
{
    RECT rc = GetCanvasRect(hDlg);

    x = ClampInt(x, rc.left, rc.right);
    y = ClampInt(y, rc.top, rc.bottom);

    if (!g_path.empty())
    {
        POINT last = g_path.back();
        int dx = x - last.x;
        int dy = y - last.y;

        if (dx * dx + dy * dy < 16)
            return;
    }

    POINT p = { x, y };
    g_path.push_back(p);
}

void StartMove(HWND hDlg)
{
    if (g_path.size() < 2)
        return;

    g_mode = MODE_MOVE;
    g_isMoving = true;
    g_isDrawing = false;

    g_targetIndex = 1;

    g_curX = g_path[0].x;
    g_curY = g_path[0].y;

    g_center.x = g_path[0].x;
    g_center.y = g_path[0].y;

    SetTimer(hDlg, TIMER_ID, TIMER_INTERVAL, NULL);
}

void UpdateMove(HWND hDlg)
{
    if (!g_isMoving)
        return;

    if (g_targetIndex >= (int)g_path.size())
    {
        g_isMoving = false;
        g_mode = MODE_NONE;
        KillTimer(hDlg, TIMER_ID);
        return;
    }

    POINT target = g_path[g_targetIndex];

    double dx = target.x - g_curX;
    double dy = target.y - g_curY;
    double dist = sqrt(dx * dx + dy * dy);

    if (dist <= g_speed)
    {
        g_curX = target.x;
        g_curY = target.y;
        g_targetIndex++;

        if (g_targetIndex >= (int)g_path.size())
        {
            g_isMoving = false;
            g_mode = MODE_NONE;
            KillTimer(hDlg, TIMER_ID);
        }
    }
    else
    {
        g_curX += dx / dist * g_speed;
        g_curY += dy / dist * g_speed;
    }

    g_center.x = (int)(g_curX + 0.5);
    g_center.y = (int)(g_curY + 0.5);

    InvalidateRect(hDlg, NULL, FALSE);
}

INT_PTR CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
    {
        srand((unsigned int)time(NULL));

        CheckDlgButton(hDlg, IDC_RAD_RECT, BST_CHECKED);
        CheckDlgButton(hDlg, IDC_RAD_MEDIUM, BST_CHECKED);
        CheckDlgButton(hDlg, IDC_RAD_GRID_OFF, BST_CHECKED);

        CheckDlgButton(hDlg, IDC_CHK_RED, BST_CHECKED);
        CheckDlgButton(hDlg, IDC_CHK_GREEN, BST_UNCHECKED);
        CheckDlgButton(hDlg, IDC_CHK_BLUE, BST_UNCHECKED);

        ReadOptions(hDlg);
        RandomizeShapePosition(hDlg);
        UpdateSpeedText(hDlg);

        return TRUE;
    }

    case WM_COMMAND:
    {
        int id = LOWORD(wParam);

        switch (id)
        {
        case IDC_RAD_RECT:
        case IDC_RAD_CIRCLE:
        case IDC_RAD_SMALL:
        case IDC_RAD_MEDIUM:
        case IDC_RAD_LARGE:
            ReadOptions(hDlg);
            RandomizeShapePosition(hDlg);
            InvalidateRect(hDlg, NULL, TRUE);
            return TRUE;

        case IDC_RAD_GRID_ON:
        case IDC_RAD_GRID_OFF:
        case IDC_CHK_RED:
        case IDC_CHK_GREEN:
        case IDC_CHK_BLUE:
            ReadOptions(hDlg);
            InvalidateRect(hDlg, NULL, TRUE);
            return TRUE;

        case IDC_BTN_DRAW:
            g_mode = MODE_DRAW;
            g_isDrawing = false;
            g_isMoving = false;
            g_path.clear();
            KillTimer(hDlg, TIMER_ID);
            InvalidateRect(hDlg, NULL, TRUE);
            return TRUE;

        case IDC_BTN_MOVE:
            StartMove(hDlg);
            return TRUE;

        case IDC_BTN_PLUS:
            g_speed += 2.0;
            if (g_speed > 30.0)
                g_speed = 30.0;
            UpdateSpeedText(hDlg);
            return TRUE;

        case IDC_BTN_MINUS:
            g_speed -= 2.0;
            if (g_speed < 1.0)
                g_speed = 1.0;
            UpdateSpeedText(hDlg);
            return TRUE;

        case IDC_BTN_QUIT:
            EndDialog(hDlg, 0);
            return TRUE;
        }

        return FALSE;
    }

    case WM_LBUTTONDOWN:
    {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);

        if (g_mode == MODE_DRAW && g_hasShape && IsNearShapeCenter(x, y))
        {
            SetCapture(hDlg);

            g_isDrawing = true;
            g_path.clear();

            POINT start = g_center;
            g_path.push_back(start);

            AddPathPoint(hDlg, x, y);

            InvalidateRect(hDlg, NULL, FALSE);
        }

        return TRUE;
    }

    case WM_MOUSEMOVE:
    {
        if (g_isDrawing && (wParam & MK_LBUTTON))
        {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);

            AddPathPoint(hDlg, x, y);
            InvalidateRect(hDlg, NULL, FALSE);
        }

        return TRUE;
    }

    case WM_LBUTTONUP:
    {
        if (g_isDrawing)
        {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);

            AddPathPoint(hDlg, x, y);

            g_isDrawing = false;
            ReleaseCapture();

            InvalidateRect(hDlg, NULL, FALSE);
        }

        return TRUE;
    }

    case WM_TIMER:
    {
        if (wParam == TIMER_ID)
        {
            UpdateMove(hDlg);
            return TRUE;
        }

        return FALSE;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hDlg, &ps);

        RECT canvas = GetCanvasRect(hDlg);

        HBRUSH hWhiteBrush = CreateSolidBrush(RGB(255, 255, 255));
        FillRect(hdc, &canvas, hWhiteBrush);
        DeleteObject(hWhiteBrush);

        FrameRect(hdc, &canvas, (HBRUSH)GetStockObject(BLACK_BRUSH));

        if (g_gridOn)
        {
            DrawGrid(hdc, canvas);
        }

        DrawPath(hdc);
        DrawShape(hdc);

        EndPaint(hDlg, &ps);
        return TRUE;
    }

    case WM_CLOSE:
        EndDialog(hDlg, 0);
        return TRUE;
    }

    return FALSE;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow)
{
    g_hInst = hInstance;

    DialogBox(
        hInstance,
        MAKEINTRESOURCE(IDD_MAIN_DIALOG),
        NULL,
        DialogProc
    );

    return 0;
}