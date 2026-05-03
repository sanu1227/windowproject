#include <windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <time.h>

// 프로그램 인스턴스 핸들
HINSTANCE g_hInst;

// 윈도우 클래스 이름
LPCTSTR lpszClass = L"My Window Class";

// 창 제목
LPCTSTR lpszWindowName = L"Window Programming Lab";

// 윈도우 메시지 처리 함수 선언
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);



// ---------------------------
// 도형 종류 상수
// ---------------------------
#define SHAPE_CIRCLE     0
#define SHAPE_HOURGLASS  1
#define SHAPE_PENTAGON   2
#define SHAPE_PIE        3



// ---------------------------
// 전역 변수
// ---------------------------

// 현재 선택된 도형
int selectedShape;

// 각 도형의 원래 색
COLORREF baseColor[4];

// 현재 화면에 그릴 색
COLORREF currentColor[4];



// ---------------------------
// 랜덤 색상 함수
// ---------------------------
COLORREF GetRandomColor()
{
    return RGB(rand() % 256, rand() % 256, rand() % 256);
}

// 모든 도형 색을 원래 색으로 복구
void RestoreColors()
{
    int i;
    for (i = 0; i < 4; i++)
    {
        currentColor[i] = baseColor[i];
    }
}



// ---------------------------
// 바깥 도형 그리기 함수
// ---------------------------

// 왼쪽 원
void DrawCircle(HDC hDC, int left, int top, int right, int bottom, COLORREF color)
{
    HBRUSH hBrush = CreateSolidBrush(color);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hDC, hBrush);

    Ellipse(hDC, left, top, right, bottom);

    SelectObject(hDC, oldBrush);
    DeleteObject(hBrush);
}

// 위쪽 모래시계
void DrawHourglass(HDC hDC, int cx, int top, int width, int height, COLORREF color)
{
    POINT pt[6];
    HBRUSH hBrush, oldBrush;

    pt[0].x = cx - width / 2;  pt[0].y = top;
    pt[1].x = cx + width / 2;  pt[1].y = top;
    pt[2].x = cx + width / 6;  pt[2].y = top + height / 2;
    pt[3].x = cx + width / 2;  pt[3].y = top + height;
    pt[4].x = cx - width / 2;  pt[4].y = top + height;
    pt[5].x = cx - width / 6;  pt[5].y = top + height / 2;

    hBrush = CreateSolidBrush(color);
    oldBrush = (HBRUSH)SelectObject(hDC, hBrush);

    Polygon(hDC, pt, 6);

    SelectObject(hDC, oldBrush);
    DeleteObject(hBrush);
}

// 오른쪽 오각형
void DrawPentagon(HDC hDC, int cx, int cy, int size, COLORREF color)
{
    POINT pt[5];
    HBRUSH hBrush, oldBrush;

    pt[0].x = cx;              pt[0].y = cy - size;
    pt[1].x = cx + size;       pt[1].y = cy - size / 4;
    pt[2].x = cx + size / 2;   pt[2].y = cy + size;
    pt[3].x = cx - size / 2;   pt[3].y = cy + size;
    pt[4].x = cx - size;       pt[4].y = cy - size / 4;

    hBrush = CreateSolidBrush(color);
    oldBrush = (HBRUSH)SelectObject(hDC, hBrush);

    Polygon(hDC, pt, 5);

    SelectObject(hDC, oldBrush);
    DeleteObject(hBrush);
}

// 아래 파이 모양
void DrawPieShape(HDC hDC, int left, int top, int right, int bottom, COLORREF color)
{
    HBRUSH hBrush = CreateSolidBrush(color);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hDC, hBrush);

    // 오른쪽 위 1/4 조각이 빠진 듯한 느낌
    Pie(hDC, left, top, right, bottom,
        (left + right) / 2, top,
        right, (top + bottom) / 2);

    SelectObject(hDC, oldBrush);
    DeleteObject(hBrush);
}



// ---------------------------
// 중앙 변형 도형 그리기 함수
// ---------------------------

// c 선택 시 -> 타원
void DrawCenterEllipse(HDC hDC, RECT rc, COLORREF color)
{
    HBRUSH hBrush = CreateSolidBrush(color);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hDC, hBrush);

    Ellipse(hDC, rc.left + 35, rc.top + 55, rc.right - 35, rc.bottom - 55);

    SelectObject(hDC, oldBrush);
    DeleteObject(hBrush);
}

// s 선택 시 -> 나비 모양
void DrawCenterButterfly(HDC hDC, RECT rc, COLORREF color)
{
    POINT leftWing[3];
    POINT rightWing[3];
    int cx = (rc.left + rc.right) / 2;
    int cy = (rc.top + rc.bottom) / 2;
    HBRUSH hBrush, oldBrush;

    leftWing[0].x = rc.left + 35;   leftWing[0].y = rc.top + 20;
    leftWing[1].x = cx;             leftWing[1].y = cy;
    leftWing[2].x = rc.left + 35;   leftWing[2].y = rc.bottom - 20;

    rightWing[0].x = rc.right - 35; rightWing[0].y = rc.top + 20;
    rightWing[1].x = cx;            rightWing[1].y = cy;
    rightWing[2].x = rc.right - 35; rightWing[2].y = rc.bottom - 20;

    hBrush = CreateSolidBrush(color);
    oldBrush = (HBRUSH)SelectObject(hDC, hBrush);

    Polygon(hDC, leftWing, 3);
    Polygon(hDC, rightWing, 3);

    SelectObject(hDC, oldBrush);
    DeleteObject(hBrush);
}

// p 선택 시 -> 위아래 뒤집힌 오각형
void DrawCenterInvertedPentagon(HDC hDC, RECT rc, COLORREF color)
{
    POINT pt[5];
    int cx = (rc.left + rc.right) / 2;
    int cy = (rc.top + rc.bottom) / 2;
    int size = 55;
    HBRUSH hBrush, oldBrush;

    pt[0].x = cx;              pt[0].y = cy + size;
    pt[1].x = cx + size;       pt[1].y = cy + size / 4;
    pt[2].x = cx + size / 2;   pt[2].y = cy - size;
    pt[3].x = cx - size / 2;   pt[3].y = cy - size;
    pt[4].x = cx - size;       pt[4].y = cy + size / 4;

    hBrush = CreateSolidBrush(color);
    oldBrush = (HBRUSH)SelectObject(hDC, hBrush);

    Polygon(hDC, pt, 5);

    SelectObject(hDC, oldBrush);
    DeleteObject(hBrush);
}

// e 선택 시 -> 파이의 남은 부분
void DrawCenterPieRemain(HDC hDC, RECT rc, COLORREF color)
{
    HRGN hOuter;
    HRGN hCut;
    HRGN hResult;
    HBRUSH hBrush;
    POINT cutPt[3];
    int left, top, right, bottom;
    int cx, cy;

    left = rc.left + 35;
    top = rc.top + 25;
    right = rc.right - 35;
    bottom = rc.bottom - 25;
    cx = (left + right) / 2;
    cy = (top + bottom) / 2;

    // 전체 원
    hOuter = CreateEllipticRgn(left, top, right, bottom);

    // 잘라낼 삼각형 조각
    cutPt[0].x = cx;    cutPt[0].y = cy;
    cutPt[1].x = cx;    cutPt[1].y = top;
    cutPt[2].x = right; cutPt[2].y = cy;

    hCut = CreatePolygonRgn(cutPt, 3, WINDING);

    // 전체 원 - 잘라낼 조각
    hResult = CreateRectRgn(0, 0, 0, 0);
    CombineRgn(hResult, hOuter, hCut, RGN_DIFF);

    hBrush = CreateSolidBrush(color);
    FillRgn(hDC, hResult, hBrush);
    FrameRgn(hDC, hResult, (HBRUSH)GetStockObject(BLACK_BRUSH), 1, 1);

    DeleteObject(hBrush);
    DeleteObject(hOuter);
    DeleteObject(hCut);
    DeleteObject(hResult);
}

// 현재 선택된 도형을 중앙에 그림
void DrawCenterShape(HDC hDC, RECT rc)
{
    if (selectedShape == SHAPE_CIRCLE)
    {
        DrawCenterEllipse(hDC, rc, currentColor[SHAPE_CIRCLE]);
    }
    else if (selectedShape == SHAPE_HOURGLASS)
    {
        DrawCenterButterfly(hDC, rc, currentColor[SHAPE_HOURGLASS]);
    }
    else if (selectedShape == SHAPE_PENTAGON)
    {
        DrawCenterInvertedPentagon(hDC, rc, currentColor[SHAPE_PENTAGON]);
    }
    else if (selectedShape == SHAPE_PIE)
    {
        DrawCenterPieRemain(hDC, rc, currentColor[SHAPE_PIE]);
    }
}



// ---------------------------
// 프로그램 시작점
// ---------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpszCmdParam, int nCmdShow)
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
        lpszClass, lpszWindowName, WS_OVERLAPPEDWINDOW,
        100, 100, 900, 700,
        NULL, NULL, hInstance, NULL
    );

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    while (GetMessage(&Message, 0, 0, 0))
    {
        TranslateMessage(&Message);
        DispatchMessage(&Message);
    }

    return (int)Message.wParam;
}



// ---------------------------
// 메시지 처리 함수
// ---------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hDC;
    RECT centerRect;

    centerRect.left = 300;
    centerRect.top = 200;
    centerRect.right = 600;
    centerRect.bottom = 420;

    switch (uMsg)
    {
    case WM_CREATE:
        srand((unsigned int)time(NULL));

        // 각 도형의 기본 색
        baseColor[SHAPE_CIRCLE] = RGB(80, 130, 200);      // 파랑
        baseColor[SHAPE_HOURGLASS] = RGB(220, 0, 0);      // 빨강
        baseColor[SHAPE_PENTAGON] = RGB(255, 200, 0);     // 노랑
        baseColor[SHAPE_PIE] = RGB(0, 200, 80);           // 초록

        RestoreColors();

        // 시작할 때 랜덤 선택
        selectedShape = rand() % 4;
        break;

    case WM_KEYDOWN:
        if (wParam == 'C')
        {
            RestoreColors();
            selectedShape = SHAPE_CIRCLE;
            currentColor[SHAPE_CIRCLE] = GetRandomColor();
            InvalidateRect(hWnd, NULL, TRUE);
        }
        else if (wParam == 'S')
        {
            RestoreColors();
            selectedShape = SHAPE_HOURGLASS;
            currentColor[SHAPE_HOURGLASS] = GetRandomColor();
            InvalidateRect(hWnd, NULL, TRUE);
        }
        else if (wParam == 'P')
        {
            RestoreColors();
            selectedShape = SHAPE_PENTAGON;
            currentColor[SHAPE_PENTAGON] = GetRandomColor();
            InvalidateRect(hWnd, NULL, TRUE);
        }
        else if (wParam == 'E')
        {
            RestoreColors();
            selectedShape = SHAPE_PIE;
            currentColor[SHAPE_PIE] = GetRandomColor();
            InvalidateRect(hWnd, NULL, TRUE);
        }
        break;

    case WM_KEYUP:
        if (wParam == 'C' || wParam == 'S' || wParam == 'P' || wParam == 'E')
        {
            RestoreColors();
            InvalidateRect(hWnd, NULL, TRUE);
        }
        break;

    case WM_PAINT:
        hDC = BeginPaint(hWnd, &ps);

        // 중앙 사각형
        Rectangle(hDC, centerRect.left, centerRect.top, centerRect.right, centerRect.bottom);

        // 좌우상하 도형
        DrawCircle(hDC, 120, 250, 200, 330, currentColor[SHAPE_CIRCLE]);           // 왼쪽 원
        DrawHourglass(hDC, 450, 80, 90, 70, currentColor[SHAPE_HOURGLASS]);         // 위쪽 모래시계
        DrawPentagon(hDC, 710, 290, 35, currentColor[SHAPE_PENTAGON]);              // 오른쪽 오각형
        DrawPieShape(hDC, 380, 450, 470, 540, currentColor[SHAPE_PIE]);             // 아래 파이

        // 중앙에 선택된 도형을 변형해서 그림
        DrawCenterShape(hDC, centerRect);

        EndPaint(hWnd, &ps);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}