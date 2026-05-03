#include <windows.h>   // 윈도우 API 사용
#include <tchar.h>     // TCHAR, LPCTSTR 사용
#include <stdlib.h>    // rand, srand
#include <time.h>      // time

// 프로그램 인스턴스 핸들
HINSTANCE g_hInst;

// 윈도우 클래스 이름
LPCTSTR lpszClass = L"My Window Class";

// 창 제목
LPCTSTR lpszWindowName = L"Window Programming Lab";

// 윈도우 메시지 처리 함수 선언
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);



// ---------------------------
// 도형 정보를 저장할 구조체
// ---------------------------

// 삼각형 1개의 정보
typedef struct
{
    POINT pt[3];      // 삼각형의 3개 꼭짓점
    COLORREF color;   // 삼각형 색상
} TRIANGLE;

// 사각형 1개의 정보
typedef struct
{
    int left;
    int top;
    int right;
    int bottom;
    COLORREF color;   // 사각형 색상
} RECT_SHAPE;

// 원 1개의 정보
typedef struct
{
    int left;
    int top;
    int right;
    int bottom;
    COLORREF color;   // 원 색상
} CIRCLE_SHAPE;



// ---------------------------
// 전역 변수
// ---------------------------

// 각 영역에 들어갈 도형 개수 (1~5개)
int triCount = 0;     // 삼각형 개수
int rectCount = 0;    // 사각형 개수
int circleCount = 0;  // 원 개수

// 각 도형 정보를 저장할 배열
TRIANGLE triangles[5];
RECT_SHAPE rects[5];
CIRCLE_SHAPE circles[5];



// ---------------------------
// 랜덤 관련 함수
// ---------------------------

// 랜덤 색상 생성 함수
COLORREF GetRandomColor()
{
    int r = rand() % 256;   // 0~255
    int g = rand() % 256;   // 0~255
    int b = rand() % 256;   // 0~255

    return RGB(r, g, b);
}



// ---------------------------
// 영역별 랜덤 생성 함수
// ---------------------------

// 1번 영역(왼쪽 위) 삼각형 생성
void MakeTriangles()
{
    int i;

    // 삼각형 개수는 1~5개
    triCount = rand() % 5 + 1;

    for (i = 0; i < triCount; i++)
    {
        int x = rand() % 260 + 20;   // 영역 안쪽에서 시작 x
        int y = rand() % 170 + 20;   // 영역 안쪽에서 시작 y
        int size = rand() % 40 + 30; // 삼각형 크기

        // 삼각형 꼭짓점 3개 설정
        triangles[i].pt[0].x = x;
        triangles[i].pt[0].y = y;

        triangles[i].pt[1].x = x - size / 2;
        triangles[i].pt[1].y = y + size;

        triangles[i].pt[2].x = x + size / 2;
        triangles[i].pt[2].y = y + size;

        // 색상 저장
        triangles[i].color = GetRandomColor();
    }
}

// 2번 영역(오른쪽 위) 사각형 생성
void MakeRects()
{
    int i;

    // 사각형 개수는 1~5개
    rectCount = rand() % 5 + 1;

    for (i = 0; i < rectCount; i++)
    {
        int left = rand() % 300 + 400;   // 오른쪽 위 영역 내부
        int top = rand() % 170 + 20;
        int width = rand() % 60 + 30;    // 가로 길이
        int height = rand() % 60 + 30;   // 세로 길이

        rects[i].left = left;
        rects[i].top = top;
        rects[i].right = left + width;
        rects[i].bottom = top + height;

        rects[i].color = GetRandomColor();
    }
}

// 3번 영역(아래) 원 생성
void MakeCircles()
{
    int i;

    // 원 개수는 1~5개
    circleCount = rand() % 5 + 1;

    for (i = 0; i < circleCount; i++)
    {
        int left = rand() % 650 + 30;   // 아래 영역 내부
        int top = rand() % 100 + 400;
        int size = rand() % 50 + 30;    // 원 지름

        circles[i].left = left;
        circles[i].top = top;
        circles[i].right = left + size;
        circles[i].bottom = top + size;

        circles[i].color = GetRandomColor();
    }
}

// 전체 영역 모두 다시 생성
void MakeAllShapes()
{
    MakeTriangles();
    MakeRects();
    MakeCircles();
}



// ---------------------------
// 프로그램 시작점
// ---------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpszCmdParam, int nCmdShow)
{
    HWND hWnd;             // 생성될 윈도우 핸들
    MSG Message;           // 메시지 저장 구조체
    WNDCLASSEX WndClass;   // 윈도우 클래스 정보 구조체

    g_hInst = hInstance;   // 인스턴스 저장

    // 윈도우 클래스 설정
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

    // 윈도우 생성
    hWnd = CreateWindow(
        lpszClass, lpszWindowName, WS_OVERLAPPEDWINDOW,
        0, 0, 800, 600,
        NULL, NULL, hInstance, NULL
    );

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // 메시지 루프
    while (GetMessage(&Message, 0, 0, 0)) {
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

    switch (uMsg) {

    case WM_CREATE:
        // 랜덤 시드 초기화
        srand((unsigned int)time(NULL));

        // 처음 실행될 때 전체 도형 생성
        MakeAllShapes();
        break;

    case WM_KEYDOWN:
        // Enter 키 -> 전체 다시 랜덤 생성
        if (wParam == VK_RETURN) {
            MakeAllShapes();
            InvalidateRect(hWnd, NULL, TRUE);
        }
        break;

    case WM_CHAR:
        // '1' -> 삼각형 영역만 다시 생성
        if (wParam == '1') {
            MakeTriangles();
            InvalidateRect(hWnd, NULL, TRUE);
        }
        // '2' -> 사각형 영역만 다시 생성
        else if (wParam == '2') {
            MakeRects();
            InvalidateRect(hWnd, NULL, TRUE);
        }
        // '3' -> 원 영역만 다시 생성
        else if (wParam == '3') {
            MakeCircles();
            InvalidateRect(hWnd, NULL, TRUE);
        }
        break;

    case WM_PAINT:
    {
        int i;

        hDC = BeginPaint(hWnd, &ps);

        // ---------------------------
        // 영역 경계선 그리기
        // ---------------------------
        Rectangle(hDC, 0, 0, 350, 280);      // 영역 1 : 삼각형
        Rectangle(hDC, 370, 0, 780, 280);    // 영역 2 : 사각형
        Rectangle(hDC, 0, 370, 780, 550);    // 영역 3 : 원

        // 영역 이름 표시
        TextOut(hDC, 10, 10, L"Area 1 : Triangles", 18);
        TextOut(hDC, 380, 10, L"Area 2 : Rectangles", 19);
        TextOut(hDC, 10, 380, L"Area 3 : Circles", 16);

        // ---------------------------
        // 삼각형 그리기
        // ---------------------------
        for (i = 0; i < triCount; i++)
        {
            HBRUSH hBrush = CreateSolidBrush(triangles[i].color);
            HBRUSH oldBrush = (HBRUSH)SelectObject(hDC, hBrush);

            Polygon(hDC, triangles[i].pt, 3);

            SelectObject(hDC, oldBrush);
            DeleteObject(hBrush);
        }

        // ---------------------------
        // 사각형 그리기
        // ---------------------------
        for (i = 0; i < rectCount; i++)
        {
            HBRUSH hBrush = CreateSolidBrush(rects[i].color);
            HBRUSH oldBrush = (HBRUSH)SelectObject(hDC, hBrush);

            Rectangle(hDC,
                rects[i].left,
                rects[i].top,
                rects[i].right,
                rects[i].bottom);

            SelectObject(hDC, oldBrush);
            DeleteObject(hBrush);
        }

        // ---------------------------
        // 원 그리기
        // ---------------------------
        for (i = 0; i < circleCount; i++)
        {
            HBRUSH hBrush = CreateSolidBrush(circles[i].color);
            HBRUSH oldBrush = (HBRUSH)SelectObject(hDC, hBrush);

            Ellipse(hDC,
                circles[i].left,
                circles[i].top,
                circles[i].right,
                circles[i].bottom);

            SelectObject(hDC, oldBrush);
            DeleteObject(hBrush);
        }

        EndPaint(hWnd, &ps);
    }
    break;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}