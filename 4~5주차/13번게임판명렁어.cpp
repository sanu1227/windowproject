#include <windows.h>                         // 윈도우 API 기본 헤더
#include <tchar.h>                           // 유니코드 문자열 관련 헤더
#include <stdlib.h>                          // rand, srand 사용
#include <time.h>                            // time 사용

// -------------------------------
// 상수 정의
// -------------------------------

#define MAX_SHAPES 10                        // 최대 도형 개수
#define BOARD_COLS 40                        // 보드 가로 칸 수
#define BOARD_ROWS 40                        // 보드 세로 칸 수

// -------------------------------
// 도형 종류 구분용 enum
// -------------------------------

enum SHAPE_TYPE
{
    SHAPE_CIRCLE = 0,                        // 원
    SHAPE_TRIANGLE = 1,                      // 삼각형
    SHAPE_RECT = 2                           // 사각형
};

// -------------------------------
// 도형 정보를 저장할 구조체
// -------------------------------

typedef struct tagSHAPE
{
    int type;                                // 현재 도형 종류
    int col;                                 // 보드에서의 열 위치(0~39)
    int row;                                 // 보드에서의 행 위치(0~39)
    int scale;                               // 도형 크기 비율(100 = 기본)
    COLORREF color;                          // 현재 도형 색상

    int transformed;                         // c 키로 변형된 상태인지 여부
    int originalType;                        // 원래 도형 종류
    int originalScale;                       // 원래 크기
    COLORREF originalColor;                  // 원래 색상
} SHAPE;

// -------------------------------
// 전역 변수
// -------------------------------

HINSTANCE g_hInst;                           // 프로그램 인스턴스 핸들
LPCTSTR lpszClass = L"My Window Class";      // 윈도우 클래스 이름
LPCTSTR lpszWindowName = L"Window Programming Lab"; // 창 제목

SHAPE g_shapes[MAX_SHAPES];                  // 도형 배열
int g_shapeCount = 0;                        // 현재 도형 개수
int g_selectedIndex = -1;                    // 현재 선택된 도형 인덱스

// -------------------------------
// 함수 원형 선언
// -------------------------------

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM); // 메시지 처리 함수

COLORREF GetRandomColor();                                  // 랜덤 색상 반환
int GetRandomInt(int min, int max);                         // 랜덤 정수 반환
void GetBoardRect(HWND hWnd, RECT* boardRc);                // 실제 보드가 그려질 사각형 계산
void GetCellSize(HWND hWnd, int* cellW, int* cellH);        // 한 칸의 픽셀 크기 계산
void AddShape(HWND hWnd, int shapeType);                    // 도형 추가
void DrawBoard(HDC hdc, HWND hWnd);                         // 40x40 보드 그리기
void DrawOneShape(HDC hdc, HWND hWnd, const SHAPE* s, int selected); // 도형 1개 그리기
void DrawTriangle(HDC hdc, int left, int top, int right, int bottom); // 삼각형 그리기
void MoveSelectedShape(HWND hWnd, int dx, int dy);          // 선택 도형 이동
void ResizeSelectedShape(HWND hWnd, int delta);             // 선택 도형 확대/축소
int GetNextShapeType(int currentType);                      // 다른 모양 반환
void ToggleSameTypeShapes(HWND hWnd);                       // 같은 모양 전체 변환/복구
void DeleteSelectedShape(HWND hWnd);                        // 선택 도형 삭제
void ClearAllShapes(HWND hWnd);                             // 전체 삭제
void SelectShapeByNumber(HWND hWnd, int number);            // 숫자 키로 도형 선택
void ShiftLeftFrom(int start);                              // 배열 앞으로 당기기
void BringSelectionVisualTop(HWND hWnd);                    // 선택 도형을 맨 위에 보이게 다시 그림

// -------------------------------
// 랜덤 색상 생성 함수
// -------------------------------

COLORREF GetRandomColor()
{
    int r = rand() % 256;                   // 빨강 값 생성
    int g = rand() % 256;                   // 초록 값 생성
    int b = rand() % 256;                   // 파랑 값 생성

    return RGB(r, g, b);                    // RGB 색상 반환
}

// -------------------------------
// min ~ max 범위 랜덤 정수 반환
// -------------------------------

int GetRandomInt(int min, int max)
{
    return min + rand() % (max - min + 1);  // 범위 내 랜덤 수 반환
}

// -------------------------------
// 창 안에서 보드가 그려질 실제 사각형 계산
// 40x40칸을 정사각형 느낌으로 최대한 맞춰서 중앙 배치
// -------------------------------

void GetBoardRect(HWND hWnd, RECT* boardRc)
{
    RECT clientRc;                          // 창의 클라이언트 영역
    int clientW;                            // 창 내부 너비
    int clientH;                            // 창 내부 높이
    int cellW;                              // 칸 1개의 가로 크기
    int cellH;                              // 칸 1개의 세로 크기
    int boardW;                             // 실제 보드 전체 너비
    int boardH;                             // 실제 보드 전체 높이
    int startX;                             // 보드 시작 x좌표
    int startY;                             // 보드 시작 y좌표

    GetClientRect(hWnd, &clientRc);         // 현재 창 내부 영역 가져오기

    clientW = clientRc.right - clientRc.left; // 클라이언트 너비 계산
    clientH = clientRc.bottom - clientRc.top; // 클라이언트 높이 계산

    cellW = clientW / BOARD_COLS;           // 가로 한 칸 크기
    cellH = clientH / BOARD_ROWS;           // 세로 한 칸 크기

    if (cellW < 1) cellW = 1;               // 최소 1픽셀 보정
    if (cellH < 1) cellH = 1;               // 최소 1픽셀 보정

    boardW = cellW * BOARD_COLS;            // 실제 보드 전체 너비
    boardH = cellH * BOARD_ROWS;            // 실제 보드 전체 높이

    startX = (clientW - boardW) / 2;        // 창 중앙 정렬용 x 시작점
    startY = (clientH - boardH) / 2;        // 창 중앙 정렬용 y 시작점

    boardRc->left = startX;                 // 보드 왼쪽
    boardRc->top = startY;                  // 보드 위쪽
    boardRc->right = startX + boardW;       // 보드 오른쪽
    boardRc->bottom = startY + boardH;      // 보드 아래쪽
}

// -------------------------------
// 현재 창 크기에 따른 칸 크기 계산
// -------------------------------

void GetCellSize(HWND hWnd, int* cellW, int* cellH)
{
    RECT boardRc;                           // 보드 영역
    GetBoardRect(hWnd, &boardRc);           // 보드 사각형 계산

    *cellW = (boardRc.right - boardRc.left) / BOARD_COLS; // 칸 가로 크기
    *cellH = (boardRc.bottom - boardRc.top) / BOARD_ROWS; // 칸 세로 크기
}

// -------------------------------
// 도형 추가 함수
// -------------------------------

void AddShape(HWND hWnd, int shapeType)
{
    SHAPE newShape;                         // 새 도형 변수

    newShape.type = shapeType;              // 도형 종류 저장
    newShape.col = GetRandomInt(0, BOARD_COLS - 1); // 랜덤 열 위치
    newShape.row = GetRandomInt(0, BOARD_ROWS - 1); // 랜덤 행 위치
    newShape.scale = 100;                   // 기본 크기 100%
    newShape.color = GetRandomColor();      // 랜덤 색상

    newShape.transformed = 0;               // 아직 변형되지 않음
    newShape.originalType = shapeType;      // 원래 타입 저장
    newShape.originalScale = 100;           // 원래 크기 저장
    newShape.originalColor = newShape.color; // 원래 색상 저장

    // 최대 10개를 넘으면 첫 번째 도형 삭제 후 앞으로 당김
    if (g_shapeCount == MAX_SHAPES)
    {
        ShiftLeftFrom(0);                   // 맨 앞 삭제 효과
        g_shapeCount--;                     // 개수 감소

        if (g_selectedIndex > 0)            // 선택 도형이 뒤쪽에 있었으면
            g_selectedIndex--;              // 한 칸 앞으로 당김
        else if (g_selectedIndex == 0)      // 선택 도형이 삭제된 경우
            g_selectedIndex = -1;           // 선택 해제
    }

    g_shapes[g_shapeCount] = newShape;      // 맨 뒤에 새 도형 저장
    g_selectedIndex = g_shapeCount;         // 새 도형 자동 선택
    g_shapeCount++;                         // 개수 증가

    InvalidateRect(hWnd, NULL, TRUE);       // 다시 그리기 요청
}

// -------------------------------
// 40 x 40 칸 보드 그리기
// -------------------------------

void DrawBoard(HDC hdc, HWND hWnd)
{
    RECT boardRc;                           // 보드 영역
    int cellW, cellH;                       // 칸 가로/세로 크기
    int i;                                  // 반복 변수
    HPEN hPen;                              // 보드 선용 펜
    HPEN hOldPen;                           // 이전 펜 저장
    HBRUSH hOldBrush;                       // 이전 브러시 저장

    GetBoardRect(hWnd, &boardRc);           // 보드 사각형 계산
    GetCellSize(hWnd, &cellW, &cellH);      // 칸 크기 계산

    hPen = CreatePen(PS_SOLID, 1, RGB(220, 220, 220)); // 연한 회색 펜 생성
    hOldPen = (HPEN)SelectObject(hdc, hPen);           // 펜 선택
    hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH)); // 내부는 안 채움

    // 세로선 그리기
    for (i = 0; i <= BOARD_COLS; i++)
    {
        int x = boardRc.left + i * cellW;   // 현재 세로선 x좌표
        MoveToEx(hdc, x, boardRc.top, NULL); // 시작점
        LineTo(hdc, x, boardRc.bottom);     // 아래로 선 그리기
    }

    // 가로선 그리기
    for (i = 0; i <= BOARD_ROWS; i++)
    {
        int y = boardRc.top + i * cellH;    // 현재 가로선 y좌표
        MoveToEx(hdc, boardRc.left, y, NULL); // 시작점
        LineTo(hdc, boardRc.right, y);      // 오른쪽으로 선 그리기
    }

    SelectObject(hdc, hOldBrush);           // 브러시 복구
    SelectObject(hdc, hOldPen);             // 펜 복구
    DeleteObject(hPen);                     // 생성한 펜 삭제
}

// -------------------------------
// 삼각형 그리기 함수
// -------------------------------

void DrawTriangle(HDC hdc, int left, int top, int right, int bottom)
{
    POINT pt[3];                            // 삼각형 꼭짓점 배열

    pt[0].x = (left + right) / 2;           // 위쪽 중앙 x
    pt[0].y = top;                          // 위쪽 중앙 y

    pt[1].x = left;                         // 왼쪽 아래 x
    pt[1].y = bottom;                       // 왼쪽 아래 y

    pt[2].x = right;                        // 오른쪽 아래 x
    pt[2].y = bottom;                       // 오른쪽 아래 y

    Polygon(hdc, pt, 3);                    // 삼각형 그리기
}

// -------------------------------
// 도형 1개 그리기
// -------------------------------

void DrawOneShape(HDC hdc, HWND hWnd, const SHAPE* s, int selected)
{
    RECT boardRc;                           // 보드 영역
    int cellW, cellH;                       // 칸 가로/세로 크기
    int cellLeft, cellTop;                  // 해당 칸 왼쪽 위 좌표
    int cellRight, cellBottom;              // 해당 칸 오른쪽 아래 좌표
    int baseMarginX, baseMarginY;           // 도형 기본 여백
    int innerW, innerH;                     // 칸 내부 사용 가능 영역
    int drawW, drawH;                       // 실제 도형 크기
    int left, top, right, bottom;           // 실제 도형 그릴 영역
    HPEN hPen;                              // 일반 외곽선 펜
    HPEN hOldPen;                           // 이전 펜
    HBRUSH hBrush;                          // 채우기 브러시
    HBRUSH hOldBrush;                       // 이전 브러시

    GetBoardRect(hWnd, &boardRc);           // 보드 영역 계산
    GetCellSize(hWnd, &cellW, &cellH);      // 칸 크기 계산

    cellLeft = boardRc.left + s->col * cellW;   // 칸 왼쪽
    cellTop = boardRc.top + s->row * cellH;     // 칸 위쪽
    cellRight = cellLeft + cellW;               // 칸 오른쪽
    cellBottom = cellTop + cellH;               // 칸 아래쪽

    baseMarginX = cellW / 10;               // 좌우 여백
    baseMarginY = cellH / 10;               // 상하 여백

    if (baseMarginX < 2) baseMarginX = 2;   // 너무 작으면 최소값 설정
    if (baseMarginY < 2) baseMarginY = 2;   // 너무 작으면 최소값 설정

    innerW = cellW - baseMarginX * 2;       // 내부 가용 너비
    innerH = cellH - baseMarginY * 2;       // 내부 가용 높이

    if (innerW < 4) innerW = 4;             // 최소 내부 너비
    if (innerH < 4) innerH = 4;             // 최소 내부 높이

    drawW = innerW * s->scale / 100;        // 확대/축소 적용 너비
    drawH = innerH * s->scale / 100;        // 확대/축소 적용 높이

    if (drawW < 4) drawW = 4;               // 최소 도형 너비
    if (drawH < 4) drawH = 4;               // 최소 도형 높이

    left = (cellLeft + cellRight - drawW) / 2;   // 중앙 정렬된 왼쪽
    top = (cellTop + cellBottom - drawH) / 2;    // 중앙 정렬된 위쪽
    right = left + drawW;                        // 오른쪽
    bottom = top + drawH;                        // 아래쪽

    hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0)); // 검정 외곽선 펜
    hOldPen = (HPEN)SelectObject(hdc, hPen);     // 펜 선택

    hBrush = CreateSolidBrush(s->color);         // 도형 색 브러시 생성
    hOldBrush = (HBRUSH)SelectObject(hdc, hBrush); // 브러시 선택

    if (s->type == SHAPE_CIRCLE)                 // 원이면
    {
        Ellipse(hdc, left, top, right, bottom);  // 원 그리기
    }
    else if (s->type == SHAPE_TRIANGLE)          // 삼각형이면
    {
        DrawTriangle(hdc, left, top, right, bottom); // 삼각형 그리기
    }
    else if (s->type == SHAPE_RECT)              // 사각형이면
    {
        Rectangle(hdc, left, top, right, bottom); // 사각형 그리기
    }

    // 선택된 도형이면 테두리 표시
    if (selected)
    {
        int padX = cellW / 12;               // 선택 테두리 가로 여백
        int padY = cellH / 12;               // 선택 테두리 세로 여백
        HPEN hSelPen;                        // 선택 펜
        HPEN hPrevPen;                       // 이전 펜
        HBRUSH hPrevBrush;                   // 이전 브러시

        if (padX < 2) padX = 2;              // 최소 pad
        if (padY < 2) padY = 2;              // 최소 pad

        hSelPen = CreatePen(PS_SOLID, 2, RGB(255, 0, 0)); // 빨간 테두리 펜
        hPrevPen = (HPEN)SelectObject(hdc, hSelPen);      // 선택 펜 적용
        hPrevBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH)); // 내부 안 채움

        if (s->type == SHAPE_CIRCLE)         // 원이면
        {
            Ellipse(hdc, left - padX, top - padY, right + padX, bottom + padY);
        }
        else if (s->type == SHAPE_TRIANGLE)  // 삼각형이면
        {
            DrawTriangle(hdc, left - padX, top - padY, right + padX, bottom + padY);
        }
        else if (s->type == SHAPE_RECT)      // 사각형이면
        {
            Rectangle(hdc, left - padX, top - padY, right + padX, bottom + padY);
        }

        SelectObject(hdc, hPrevBrush);       // 브러시 복구
        SelectObject(hdc, hPrevPen);         // 펜 복구
        DeleteObject(hSelPen);               // 선택 펜 삭제
    }

    SelectObject(hdc, hOldBrush);            // 이전 브러시 복구
    SelectObject(hdc, hOldPen);              // 이전 펜 복구
    DeleteObject(hBrush);                    // 브러시 삭제
    DeleteObject(hPen);                      // 펜 삭제
}

// -------------------------------
// 선택 도형 이동
// 가장자리를 넘어가면 반대편으로 이동
// -------------------------------

void MoveSelectedShape(HWND hWnd, int dx, int dy)
{
    if (g_selectedIndex < 0 || g_selectedIndex >= g_shapeCount) // 선택 없으면
        return;                              // 종료

    g_shapes[g_selectedIndex].col += dx;     // 열 이동
    g_shapes[g_selectedIndex].row += dy;     // 행 이동

    if (g_shapes[g_selectedIndex].col < 0)   // 왼쪽 밖으로 나가면
        g_shapes[g_selectedIndex].col = BOARD_COLS - 1; // 맨 오른쪽으로

    if (g_shapes[g_selectedIndex].col >= BOARD_COLS) // 오른쪽 밖으로 나가면
        g_shapes[g_selectedIndex].col = 0;   // 맨 왼쪽으로

    if (g_shapes[g_selectedIndex].row < 0)   // 위로 나가면
        g_shapes[g_selectedIndex].row = BOARD_ROWS - 1; // 맨 아래로

    if (g_shapes[g_selectedIndex].row >= BOARD_ROWS) // 아래로 나가면
        g_shapes[g_selectedIndex].row = 0;   // 맨 위로

    InvalidateRect(hWnd, NULL, TRUE);        // 다시 그리기
}

// -------------------------------
// 선택 도형 크기 변경
// -------------------------------

void ResizeSelectedShape(HWND hWnd, int delta)
{
    if (g_selectedIndex < 0 || g_selectedIndex >= g_shapeCount) // 선택 없으면
        return;                              // 종료

    g_shapes[g_selectedIndex].scale += delta; // scale 증가/감소

    if (g_shapes[g_selectedIndex].scale < 30) // 너무 작아지면
        g_shapes[g_selectedIndex].scale = 30; // 최소 30%

    if (g_shapes[g_selectedIndex].scale > 250) // 너무 커지면
        g_shapes[g_selectedIndex].scale = 250; // 최대 250%

    InvalidateRect(hWnd, NULL, TRUE);        // 다시 그리기
}

// -------------------------------
// 현재 모양이 아닌 다른 모양 반환
// -------------------------------

int GetNextShapeType(int currentType)
{
    if (currentType == SHAPE_CIRCLE)         // 원이면
        return SHAPE_TRIANGLE;               // 삼각형으로
    else if (currentType == SHAPE_TRIANGLE)  // 삼각형이면
        return SHAPE_RECT;                   // 사각형으로
    else                                     // 사각형이면
        return SHAPE_CIRCLE;                 // 원으로
}

// -------------------------------
// c 키 기능
// 선택 도형과 같은 모양 전체를 다른 모양/랜덤색으로 변경
// 다시 누르면 원래 상태로 복구
// -------------------------------

void ToggleSameTypeShapes(HWND hWnd)
{
    int i;                                   // 반복 변수
    int baseType;                            // 기준 타입

    if (g_selectedIndex < 0 || g_selectedIndex >= g_shapeCount) // 선택 없으면
        return;                              // 종료

    if (g_shapes[g_selectedIndex].transformed) // 이미 변형 상태면
        baseType = g_shapes[g_selectedIndex].originalType; // 원래 타입 기준
    else
        baseType = g_shapes[g_selectedIndex].type; // 현재 타입 기준

    if (!g_shapes[g_selectedIndex].transformed) // 아직 변형 안 된 상태면
    {
        for (i = 0; i < g_shapeCount; i++)   // 모든 도형 확인
        {
            if (!g_shapes[i].transformed && g_shapes[i].type == baseType) // 같은 타입이면
            {
                g_shapes[i].originalType = g_shapes[i].type;   // 원래 타입 저장
                g_shapes[i].originalColor = g_shapes[i].color; // 원래 색 저장
                g_shapes[i].originalScale = g_shapes[i].scale; // 원래 크기 저장

                g_shapes[i].type = GetNextShapeType(g_shapes[i].type); // 다른 모양
                g_shapes[i].color = GetRandomColor();                  // 랜덤 색
                g_shapes[i].transformed = 1;                           // 변형 표시
            }
        }
    }
    else
    {
        for (i = 0; i < g_shapeCount; i++)   // 모든 도형 확인
        {
            if (g_shapes[i].transformed && g_shapes[i].originalType == baseType) // 같은 원래 타입이면
            {
                g_shapes[i].type = g_shapes[i].originalType;      // 원래 모양 복구
                g_shapes[i].color = g_shapes[i].originalColor;    // 원래 색 복구
                g_shapes[i].scale = g_shapes[i].originalScale;    // 원래 크기 복구
                g_shapes[i].transformed = 0;                      // 변형 해제
            }
        }
    }

    InvalidateRect(hWnd, NULL, TRUE);        // 다시 그리기
}

// -------------------------------
// 배열을 앞으로 한 칸 당김
// -------------------------------

void ShiftLeftFrom(int start)
{
    int i;                                   // 반복 변수

    for (i = start; i < MAX_SHAPES - 1; i++) // start부터 끝-1까지
    {
        g_shapes[i] = g_shapes[i + 1];       // 뒤 요소를 앞으로 복사
    }
}

// -------------------------------
// 선택 도형 삭제
// -------------------------------

void DeleteSelectedShape(HWND hWnd)
{
    int i;                                   // 반복 변수

    if (g_selectedIndex < 0 || g_selectedIndex >= g_shapeCount) // 선택 없으면
        return;                              // 종료

    for (i = g_selectedIndex; i < g_shapeCount - 1; i++) // 삭제 위치부터
    {
        g_shapes[i] = g_shapes[i + 1];       // 한 칸씩 앞으로 당김
    }

    g_shapeCount--;                          // 개수 감소

    if (g_shapeCount == 0)                   // 아무것도 없으면
    {
        g_selectedIndex = -1;                // 선택 해제
    }
    else if (g_selectedIndex >= g_shapeCount) // 마지막 것을 지운 경우
    {
        g_selectedIndex = g_shapeCount - 1;  // 마지막 인덱스로 맞춤
    }

    InvalidateRect(hWnd, NULL, TRUE);        // 다시 그리기
}

// -------------------------------
// 전체 초기화
// -------------------------------

void ClearAllShapes(HWND hWnd)
{
    g_shapeCount = 0;                        // 도형 개수 0
    g_selectedIndex = -1;                    // 선택 없음

    InvalidateRect(hWnd, NULL, TRUE);        // 다시 그리기
}

// -------------------------------
// 숫자 번호로 도형 선택
// -------------------------------

void SelectShapeByNumber(HWND hWnd, int number)
{
    if (number < 1 || number > 10)           // 1~10 범위가 아니면
        return;                              // 종료

    if (number > g_shapeCount)               // 없는 번호면
        return;                              // 종료

    g_selectedIndex = number - 1;            // 1번 -> 인덱스 0
    BringSelectionVisualTop(hWnd);           // 선택 도형이 맨 위에 보이게 다시 그림
}

// -------------------------------
// 선택 도형 맨 위 표시용
// -------------------------------

void BringSelectionVisualTop(HWND hWnd)
{
    InvalidateRect(hWnd, NULL, TRUE);        // 다시 그리기 요청
}

// -------------------------------
// WinMain
// -------------------------------

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
    HWND hWnd;                               // 창 핸들
    MSG Message;                             // 메시지 구조체
    WNDCLASSEX WndClass;                     // 윈도우 클래스 구조체

    srand((unsigned int)time(NULL));         // 랜덤 시드 초기화

    g_hInst = hInstance;                     // 인스턴스 저장

    WndClass.cbSize = sizeof(WndClass);      // 구조체 크기
    WndClass.style = CS_HREDRAW | CS_VREDRAW; // 창 크기 변경 시 다시 그림
    WndClass.lpfnWndProc = WndProc;          // 메시지 처리 함수 등록
    WndClass.cbClsExtra = 0;                 // 추가 클래스 메모리 없음
    WndClass.cbWndExtra = 0;                 // 추가 윈도우 메모리 없음
    WndClass.hInstance = hInstance;          // 인스턴스 핸들
    WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION); // 기본 아이콘
    WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);   // 기본 커서
    WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); // 흰 배경
    WndClass.lpszMenuName = NULL;            // 메뉴 없음
    WndClass.lpszClassName = lpszClass;      // 클래스 이름
    WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION); // 작은 아이콘

    RegisterClassEx(&WndClass);              // 윈도우 클래스 등록

    hWnd = CreateWindow(
        lpszClass,                           // 클래스 이름
        lpszWindowName,                      // 창 제목
        WS_OVERLAPPEDWINDOW,                 // 기본 창 스타일
        100, 100, 1000, 900,                 // 창 위치와 크기
        NULL,                                // 부모 윈도우 없음
        NULL,                                // 메뉴 없음
        hInstance,                           // 인스턴스
        NULL                                 // 추가 데이터 없음
    );

    ShowWindow(hWnd, nCmdShow);              // 창 보이기
    UpdateWindow(hWnd);                      // 즉시 다시 그리기

    while (GetMessage(&Message, NULL, 0, 0)) // 메시지 루프
    {
        TranslateMessage(&Message);          // 키보드 메시지 변환
        DispatchMessage(&Message);           // WndProc에 전달
    }

    return (int)Message.wParam;              // 종료 코드 반환
}

// -------------------------------
// WndProc
// -------------------------------

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;                          // WM_PAINT용 구조체
    HDC hDC;                                 // 디바이스 컨텍스트
    int i;                                   // 반복 변수

    switch (uMsg)
    {
    case WM_CREATE:                          // 창 생성 시
        return 0;                            // 별도 초기화 없음

    case WM_KEYDOWN:                         // 방향키, +, - 등 처리
        switch (wParam)
        {
        case VK_LEFT:                        // 왼쪽 화살표
            MoveSelectedShape(hWnd, -1, 0);  // 왼쪽 한 칸 이동
            break;

        case VK_RIGHT:                       // 오른쪽 화살표
            MoveSelectedShape(hWnd, 1, 0);   // 오른쪽 한 칸 이동
            break;

        case VK_UP:                          // 위 화살표
            MoveSelectedShape(hWnd, 0, -1);  // 위 한 칸 이동
            break;

        case VK_DOWN:                        // 아래 화살표
            MoveSelectedShape(hWnd, 0, 1);   // 아래 한 칸 이동
            break;

        case VK_NUMPAD1:                     // 숫자패드 1
            SelectShapeByNumber(hWnd, 1);
            break;
        case VK_NUMPAD2:                     // 숫자패드 2
            SelectShapeByNumber(hWnd, 2);
            break;
        case VK_NUMPAD3:                     // 숫자패드 3
            SelectShapeByNumber(hWnd, 3);
            break;
        case VK_NUMPAD4:                     // 숫자패드 4
            SelectShapeByNumber(hWnd, 4);
            break;
        case VK_NUMPAD5:                     // 숫자패드 5
            SelectShapeByNumber(hWnd, 5);
            break;
        case VK_NUMPAD6:                     // 숫자패드 6
            SelectShapeByNumber(hWnd, 6);
            break;
        case VK_NUMPAD7:                     // 숫자패드 7
            SelectShapeByNumber(hWnd, 7);
            break;
        case VK_NUMPAD8:                     // 숫자패드 8
            SelectShapeByNumber(hWnd, 8);
            break;
        case VK_NUMPAD9:                     // 숫자패드 9
            SelectShapeByNumber(hWnd, 9);
            break;
        case VK_NUMPAD0:                     // 숫자패드 0
            SelectShapeByNumber(hWnd, 10);
            break;

        case VK_OEM_PLUS:                    // 일반 키보드 +
        case VK_ADD:                         // 숫자패드 +
            ResizeSelectedShape(hWnd, 20);   // 20% 확대
            break;

        case VK_OEM_MINUS:                   // 일반 키보드 -
        case VK_SUBTRACT:                    // 숫자패드 -
            ResizeSelectedShape(hWnd, -20);  // 20% 축소
            break;
        }
        return 0;

    case WM_CHAR:                            // 문자 키 처리
        switch (wParam)
        {
        case 'e':
        case 'E':
            AddShape(hWnd, SHAPE_CIRCLE);    // 원 추가
            break;

        case 't':
        case 'T':
            AddShape(hWnd, SHAPE_TRIANGLE);  // 삼각형 추가
            break;

        case 'r':
        case 'R':
            AddShape(hWnd, SHAPE_RECT);      // 사각형 추가
            break;

        case '1':
            SelectShapeByNumber(hWnd, 1);    // 1번 선택
            break;
        case '2':
            SelectShapeByNumber(hWnd, 2);    // 2번 선택
            break;
        case '3':
            SelectShapeByNumber(hWnd, 3);    // 3번 선택
            break;
        case '4':
            SelectShapeByNumber(hWnd, 4);    // 4번 선택
            break;
        case '5':
            SelectShapeByNumber(hWnd, 5);    // 5번 선택
            break;
        case '6':
            SelectShapeByNumber(hWnd, 6);    // 6번 선택
            break;
        case '7':
            SelectShapeByNumber(hWnd, 7);    // 7번 선택
            break;
        case '8':
            SelectShapeByNumber(hWnd, 8);    // 8번 선택
            break;
        case '9':
            SelectShapeByNumber(hWnd, 9);    // 9번 선택
            break;
        case '0':
            SelectShapeByNumber(hWnd, 10);   // 10번 선택
            break;

        case 'c':
        case 'C':
            ToggleSameTypeShapes(hWnd);      // 같은 모양 전체 변형/복구
            break;

        case 'd':
        case 'D':
            DeleteSelectedShape(hWnd);       // 선택 도형 삭제
            break;

        case 'p':
        case 'P':
            ClearAllShapes(hWnd);            // 전체 삭제
            break;

        case 'q':
        case 'Q':
            DestroyWindow(hWnd);             // 종료
            break;
        }
        return 0;

    case WM_PAINT:                           // 다시 그리기
        hDC = BeginPaint(hWnd, &ps);         // 페인팅 시작

        DrawBoard(hDC, hWnd);                // 먼저 40x40 보드 그림

        // 선택되지 않은 도형 먼저 그림
        for (i = 0; i < g_shapeCount; i++)
        {
            if (i != g_selectedIndex)        // 선택된 도형 제외
            {
                DrawOneShape(hDC, hWnd, &g_shapes[i], 0); // 일반 도형 그리기
            }
        }

        // 선택된 도형은 마지막에 그려서 위에 오게 함
        if (g_selectedIndex >= 0 && g_selectedIndex < g_shapeCount)
        {
            DrawOneShape(hDC, hWnd, &g_shapes[g_selectedIndex], 1); // 선택 강조 포함
        }

        EndPaint(hWnd, &ps);                 // 페인팅 종료
        return 0;

    case WM_DESTROY:                         // 창 닫을 때
        PostQuitMessage(0);                  // 프로그램 종료
        return 0;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam); // 기본 처리
}