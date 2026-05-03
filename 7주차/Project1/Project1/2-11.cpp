#include <windows.h>      // 윈도우 API에서 제공하는 자료형(HWND, HDC, RECT 등)과 함수(CreateWindow, BeginPaint 등)를 쓰기 위한 헤더
#include <tchar.h>        // 유니코드 문자열(L"문자열")과 TCHAR 계열 자료형을 사용하기 위한 헤더
#include <stdlib.h>       // rand(), srand() 같은 난수 관련 함수를 사용하기 위한 헤더
#include <time.h>         // time() 함수를 사용해서 난수 시드를 현재 시간으로 설정하기 위한 헤더
#include <math.h>         // cos(), sin() 함수를 사용해서 오각형 꼭짓점을 계산하기 위한 헤더

/* ---------------- 전역 변수 ---------------- */

// 현재 실행 중인 프로그램의 인스턴스 핸들을 저장하는 전역 변수
// WinMain에서 전달받은 hInstance 값을 저장해 두면, 나중에 다른 함수에서도 프로그램 인스턴스 정보를 사용할 수 있다.
HINSTANCE g_hInst;

// 윈도우 클래스를 등록할 때 사용할 클래스 이름 문자열
// CreateWindow에서 어떤 종류의 창을 만들지 지정할 때 이 이름을 사용한다.
LPCTSTR lpszClass = L"My Window Class";

// 실제 창의 제목 표시줄에 보일 문자열
LPCTSTR lpszWindowName = L"Window Programming Lab";

// 윈도우 메시지를 처리하는 함수 원형 선언
// 실제 구현은 아래쪽에 나오며, 키 입력/그리기/종료 등을 처리한다.
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);


/* ---------------- 도형 종류 ---------------- */

// 도형 종류를 숫자로 구분하기 위한 열거형
// 0,1,2,3 값으로 각 도형을 구분하게 된다.
enum SHAPE_TYPE
{
    SHAPE_CIRCLE = 0,   // 원 도형
    SHAPE_HOURGLASS,    // 모래시계 도형
    SHAPE_PENTAGON,     // 오각형 도형
    SHAPE_PIE           // 파이 도형
};

// 도형이 배치되는 위치를 숫자로 구분하기 위한 열거형
// 왼쪽/위/오른쪽/아래 위치를 배열 인덱스로 사용하기 좋게 만든다.
enum SLOT_POS
{
    SLOT_LEFT = 0,      // 왼쪽 위치
    SLOT_TOP,           // 위쪽 위치
    SLOT_RIGHT,         // 오른쪽 위치
    SLOT_BOTTOM         // 아래쪽 위치
};


/* ---------------- 도형 색 정보 구조체 ---------------- */

// 도형 하나당 색 정보를 저장하기 위한 구조체
typedef struct
{
    COLORREF baseColor;     // 도형의 원래 기본 색
    COLORREF drawColor;     // 현재 화면에 그릴 색 (키를 누르는 동안만 랜덤 색으로 바뀔 수 있음)
} SHAPE_INFO;


/* ---------------- 전역 데이터 ---------------- */

// 4개 도형(원, 모래시계, 오각형, 파이)의 색 정보를 저장하는 배열
// 인덱스는 SHAPE_TYPE 값과 맞춰 사용한다.
SHAPE_INFO g_shapes[4];

// 각 위치(왼쪽, 위, 오른쪽, 아래)에 현재 어떤 도형이 배치되어 있는지를 저장하는 배열
// 예를 들어 g_slotShape[SLOT_LEFT] = SHAPE_CIRCLE 이면, 왼쪽 위치에 원이 있다는 뜻이다.
int g_slotShape[4];

// 현재 선택된 도형의 종류를 저장하는 변수
// 중앙 사각형 안에는 이 선택된 도형의 "변형 모양"이 그려진다.
int g_selectedShape = SHAPE_CIRCLE;

// c/s/p/e 키가 현재 눌린 상태인지를 저장하는 배열
// 인덱스는 도형 번호와 같게 사용한다.
// FALSE이면 아직 안 눌린 상태, TRUE이면 이미 누르고 있는 상태이다.
// 이렇게 하면 키를 꾹 누르고 있어도 색이 여러 번 바뀌지 않게 만들 수 있다.
BOOL g_keyPressed[4] = { FALSE, FALSE, FALSE, FALSE };


/* ---------------- 함수 선언 ---------------- */

// 랜덤 색을 만들어 반환하는 함수 선언
COLORREF RandomColor(void);

// RECT 구조체의 중심 x 좌표를 계산해서 반환하는 함수 선언
int CenterX(RECT rc);

// RECT 구조체의 중심 y 좌표를 계산해서 반환하는 함수 선언
int CenterY(RECT rc);

// 원(또는 타원)을 그리는 함수 선언
void DrawCircle(HDC hdc, RECT rc, COLORREF color, BOOL highlight);

// 일반 모래시계(위아래 삼각형 2개 모양)를 그리는 함수 선언
void DrawHourglass(HDC hdc, RECT rc, COLORREF color, BOOL highlight);

// 중앙용 가로 모래시계(>< 모양)를 그리는 함수 선언
void DrawHourglassRotated(HDC hdc, RECT rc, COLORREF color, BOOL highlight);

// 오각형을 그리는 함수 선언
// flipped가 TRUE이면 뒤집힌 오각형, FALSE이면 일반 오각형을 그린다.
void DrawPentagon(HDC hdc, RECT rc, COLORREF color, BOOL highlight, BOOL flipped);

// 일반 파이를 그리는 함수 선언
void DrawPieNormal(HDC hdc, RECT rc, COLORREF color, BOOL highlight);

// 남은 부분 파이를 그리는 함수 선언
void DrawPieRemain(HDC hdc, RECT rc, COLORREF color, BOOL highlight);

// 바깥쪽 도형을 종류에 따라 선택해서 그리는 함수 선언
void DrawOuterShape(HDC hdc, int shapeType, RECT rc, COLORREF color, BOOL highlight);

// 중앙 사각형 안의 변형 도형을 종류에 따라 선택해서 그리는 함수 선언
void DrawCenterShape(HDC hdc, int shapeType, RECT rc, COLORREF color);

// 도형 배치를 시계 방향으로 한 칸 회전시키는 함수 선언
void RotateClockwise(void);

// 도형 배치를 반시계 방향으로 한 칸 회전시키는 함수 선언
void RotateCounterClockwise(void);

// 위/아래 위치의 도형을 서로 바꾸는 함수 선언
void SwapTopBottom(void);

// 좌/우 위치의 도형을 서로 바꾸는 함수 선언
void SwapLeftRight(void);


/* ---------------- 보조 함수 ---------------- */

// 랜덤 색을 만들어 반환하는 함수
COLORREF RandomColor(void)
{
    int r;  // 빨강 값 저장용 변수
    int g;  // 초록 값 저장용 변수
    int b;  // 파랑 값 저장용 변수

    // 30~255 범위의 랜덤 빨강 값 생성
    // 너무 어두운 색이 나오지 않도록 0이 아니라 30부터 시작한다.
    r = 30 + rand() % 226;

    // 30~255 범위의 랜덤 초록 값 생성
    g = 30 + rand() % 226;

    // 30~255 범위의 랜덤 파랑 값 생성
    b = 30 + rand() % 226;

    // RGB 매크로를 이용해 COLORREF 값으로 합쳐 반환
    return RGB(r, g, b);
}

// RECT 구조체의 중심 x 좌표를 계산해 반환하는 함수
int CenterX(RECT rc)
{
    // 왼쪽 좌표와 오른쪽 좌표의 평균을 구하면 중심 x 좌표가 된다.
    return (rc.left + rc.right) / 2;
}

// RECT 구조체의 중심 y 좌표를 계산해 반환하는 함수
int CenterY(RECT rc)
{
    // 위쪽 좌표와 아래쪽 좌표의 평균을 구하면 중심 y 좌표가 된다.
    return (rc.top + rc.bottom) / 2;
}


/* ---------------- 도형 그리기 함수 ---------------- */

// 원 또는 타원을 그리는 함수
void DrawCircle(HDC hdc, RECT rc, COLORREF color, BOOL highlight)
{
    HPEN hPen;         // 도형의 테두리를 그릴 펜 핸들
    HBRUSH hBrush;     // 도형 내부를 채울 브러시 핸들
    HPEN oldPen;       // 원래 선택되어 있던 펜을 보관할 변수
    HBRUSH oldBrush;   // 원래 선택되어 있던 브러시를 보관할 변수

    // highlight가 TRUE이면 두껍고 빨간 테두리 펜 생성
    // highlight가 FALSE이면 보통 두께의 검정 펜 생성
    hPen = CreatePen(PS_SOLID, highlight ? 4 : 2,
        highlight ? RGB(255, 0, 0) : RGB(0, 0, 0));

    // 전달받은 색으로 내부를 채울 브러시 생성
    hBrush = CreateSolidBrush(color);

    // 새 펜을 DC에 선택하고, 이전 펜을 oldPen에 저장
    oldPen = (HPEN)SelectObject(hdc, hPen);

    // 새 브러시를 DC에 선택하고, 이전 브러시를 oldBrush에 저장
    oldBrush = (HBRUSH)SelectObject(hdc, hBrush);

    // rc 영역 안에 타원(정사각형이면 원)을 그림
    Ellipse(hdc, rc.left, rc.top, rc.right, rc.bottom);

    // 원래 펜을 다시 복구
    SelectObject(hdc, oldPen);

    // 원래 브러시를 다시 복구
    SelectObject(hdc, oldBrush);

    // 새로 만든 펜 객체 삭제
    DeleteObject(hPen);

    // 새로 만든 브러시 객체 삭제
    DeleteObject(hBrush);
}

// 일반 세로 모래시계(위아래 삼각형 2개)를 그리는 함수
void DrawHourglass(HDC hdc, RECT rc, COLORREF color, BOOL highlight)
{
    HPEN hPen;              // 테두리용 펜
    HBRUSH hBrush;          // 채우기용 브러시
    HPEN oldPen;            // 원래 펜 보관용
    HBRUSH oldBrush;        // 원래 브러시 보관용
    POINT topTri[3];        // 위쪽 삼각형의 세 꼭짓점 저장 배열
    POINT bottomTri[3];     // 아래쪽 삼각형의 세 꼭짓점 저장 배열
    int cx;                 // 중심 x 좌표
    int cy;                 // 중심 y 좌표

    // 선택 여부에 따라 펜 생성
    hPen = CreatePen(PS_SOLID, highlight ? 4 : 2,
        highlight ? RGB(255, 0, 0) : RGB(0, 0, 0));

    // 내부 채우기 색 브러시 생성
    hBrush = CreateSolidBrush(color);

    // 생성한 펜을 선택하고 이전 펜 저장
    oldPen = (HPEN)SelectObject(hdc, hPen);

    // 생성한 브러시를 선택하고 이전 브러시 저장
    oldBrush = (HBRUSH)SelectObject(hdc, hBrush);

    // 그릴 사각형 중심 좌표 계산
    cx = CenterX(rc);
    cy = CenterY(rc);

    // 위쪽 삼각형의 첫 번째 점: 왼쪽 위
    topTri[0].x = rc.left;
    topTri[0].y = rc.top;

    // 위쪽 삼각형의 두 번째 점: 오른쪽 위
    topTri[1].x = rc.right;
    topTri[1].y = rc.top;

    // 위쪽 삼각형의 세 번째 점: 중앙
    topTri[2].x = cx;
    topTri[2].y = cy;

    // 아래쪽 삼각형의 첫 번째 점: 왼쪽 아래
    bottomTri[0].x = rc.left;
    bottomTri[0].y = rc.bottom;

    // 아래쪽 삼각형의 두 번째 점: 오른쪽 아래
    bottomTri[1].x = rc.right;
    bottomTri[1].y = rc.bottom;

    // 아래쪽 삼각형의 세 번째 점: 중앙
    bottomTri[2].x = cx;
    bottomTri[2].y = cy;

    // 위쪽 삼각형 그리기
    Polygon(hdc, topTri, 3);

    // 아래쪽 삼각형 그리기
    Polygon(hdc, bottomTri, 3);

    // 원래 펜 복구
    SelectObject(hdc, oldPen);

    // 원래 브러시 복구
    SelectObject(hdc, oldBrush);

    // 생성한 펜 삭제
    DeleteObject(hPen);

    // 생성한 브러시 삭제
    DeleteObject(hBrush);
}


// 중앙용 가로 모래시계(>< 모양)를 그리는 함수
void DrawHourglassRotated(HDC hdc, RECT rc, COLORREF color, BOOL highlight)
{
    HPEN hPen;             // 테두리용 펜
    HBRUSH hBrush;         // 채우기용 브러시
    HPEN oldPen;           // 원래 펜 보관용
    HBRUSH oldBrush;       // 원래 브러시 보관용
    POINT leftTri[3];      // 왼쪽 삼각형 점 3개
    POINT rightTri[3];     // 오른쪽 삼각형 점 3개
    int cx;                // 중심 x 좌표
    int cy;                // 중심 y 좌표

    // 선택 여부에 따라 빨간 테두리/검정 테두리 펜 생성
    hPen = CreatePen(PS_SOLID, highlight ? 4 : 2,
        highlight ? RGB(255, 0, 0) : RGB(0, 0, 0));

    // 내부 채우기용 브러시 생성
    hBrush = CreateSolidBrush(color);

    // 새 펜 선택
    oldPen = (HPEN)SelectObject(hdc, hPen);

    // 새 브러시 선택
    oldBrush = (HBRUSH)SelectObject(hdc, hBrush);

    // 현재 영역의 중심 좌표 계산
    cx = CenterX(rc);
    cy = CenterY(rc);

    // 왼쪽 삼각형의 첫 번째 점: 왼쪽 위
    leftTri[0].x = rc.left;
    leftTri[0].y = rc.top;

    // 왼쪽 삼각형의 두 번째 점: 중앙
    leftTri[1].x = cx;
    leftTri[1].y = cy;

    // 왼쪽 삼각형의 세 번째 점: 왼쪽 아래
    leftTri[2].x = rc.left;
    leftTri[2].y = rc.bottom;

    // 오른쪽 삼각형의 첫 번째 점: 오른쪽 위
    rightTri[0].x = rc.right;
    rightTri[0].y = rc.top;

    // 오른쪽 삼각형의 두 번째 점: 중앙
    rightTri[1].x = cx;
    rightTri[1].y = cy;

    // 오른쪽 삼각형의 세 번째 점: 오른쪽 아래
    rightTri[2].x = rc.right;
    rightTri[2].y = rc.bottom;

    // 왼쪽 삼각형을 그림
    Polygon(hdc, leftTri, 3);

    // 오른쪽 삼각형을 그림
    Polygon(hdc, rightTri, 3);

    // 원래 펜 복구
    SelectObject(hdc, oldPen);

    // 원래 브러시 복구
    SelectObject(hdc, oldBrush);

    // 새로 만든 펜 삭제
    DeleteObject(hPen);

    // 새로 만든 브러시 삭제
    DeleteObject(hBrush);
}

// 오각형을 그리는 함수
void DrawPentagon(HDC hdc, RECT rc, COLORREF color, BOOL highlight, BOOL flipped)
{
    HPEN hPen;           // 테두리용 펜
    HBRUSH hBrush;       // 채우기용 브러시
    HPEN oldPen;         // 원래 펜 보관용
    HBRUSH oldBrush;     // 원래 브러시 보관용
    POINT pt[5];         // 오각형의 5개 꼭짓점을 저장할 배열
    int cx;              // 중심 x 좌표
    int cy;              // 중심 y 좌표
    int rx;              // x축 반지름 비슷한 값
    int ry;              // y축 반지름 비슷한 값
    double startAngle;   // 시작 각도
    int i;               // 반복문 인덱스

    // 선택 여부에 따라 펜 생성
    hPen = CreatePen(PS_SOLID, highlight ? 4 : 2,
        highlight ? RGB(255, 0, 0) : RGB(0, 0, 0));

    // 내부 채우기 브러시 생성
    hBrush = CreateSolidBrush(color);

    // 새 펜 선택
    oldPen = (HPEN)SelectObject(hdc, hPen);

    // 새 브러시 선택
    oldBrush = (HBRUSH)SelectObject(hdc, hBrush);

    // 중심 x 좌표 계산
    cx = CenterX(rc);

    // 중심 y 좌표 계산
    cy = CenterY(rc);

    // 전체 너비의 절반을 x축 크기 기준으로 사용
    rx = (rc.right - rc.left) / 2;

    // 전체 높이의 절반을 y축 크기 기준으로 사용
    ry = (rc.bottom - rc.top) / 2;

    // 뒤집지 않은 경우 위쪽이 꼭짓점이 되도록 -90도에서 시작
    if (flipped == FALSE)
        startAngle = -90.0;
    else
        // 뒤집는 경우 아래쪽이 꼭짓점이 되도록 90도에서 시작
        startAngle = 90.0;

    // 5개 꼭짓점을 차례로 계산
    for (i = 0; i < 5; i++)
    {
        double angle;   // 현재 꼭짓점의 각도(라디안)

        // 72도씩 회전하며 5개 점 생성, 라디안으로 변환
        angle = (startAngle + i * 72.0) * 3.141592 / 180.0;

        // x 좌표 계산
        pt[i].x = cx + (int)(rx * 0.95 * cos(angle));

        // y 좌표 계산
        pt[i].y = cy + (int)(ry * 0.95 * sin(angle));
    }

    // 계산된 5개 점으로 오각형 그리기
    Polygon(hdc, pt, 5);

    // 원래 펜 복구
    SelectObject(hdc, oldPen);

    // 원래 브러시 복구
    SelectObject(hdc, oldBrush);

    // 생성한 펜 삭제
    DeleteObject(hPen);

    // 생성한 브러시 삭제
    DeleteObject(hBrush);
}

// 일반 파이를 그리는 함수
void DrawPieNormal(HDC hdc, RECT rc, COLORREF color, BOOL highlight)
{
    HPEN hPen;          // 테두리용 펜
    HBRUSH hBrush;      // 채우기용 브러시
    HPEN oldPen;        // 원래 펜 보관용
    HBRUSH oldBrush;    // 원래 브러시 보관용
    int cx;             // 중심 x 좌표
    int cy;             // 중심 y 좌표

    // 선택 여부에 따라 펜 생성
    hPen = CreatePen(PS_SOLID, highlight ? 4 : 2,
        highlight ? RGB(255, 0, 0) : RGB(0, 0, 0));

    // 내부 채우기 브러시 생성
    hBrush = CreateSolidBrush(color);

    // 새 펜 선택
    oldPen = (HPEN)SelectObject(hdc, hPen);

    // 새 브러시 선택
    oldBrush = (HBRUSH)SelectObject(hdc, hBrush);

    // 중심 좌표 계산
    cx = CenterX(rc);
    cy = CenterY(rc);

    // 일반 파이 그리기
    // 시작점: 오른쪽 중앙, 끝점: 위쪽 중앙
    Pie(hdc, rc.left, rc.top, rc.right, rc.bottom,
        rc.right, cy,
        cx, rc.top);

    // 원래 펜 복구
    SelectObject(hdc, oldPen);

    // 원래 브러시 복구
    SelectObject(hdc, oldBrush);

    // 생성한 펜 삭제
    DeleteObject(hPen);

    // 생성한 브러시 삭제
    DeleteObject(hBrush);
}

// 남은 부분 파이를 그리는 함수
void DrawPieRemain(HDC hdc, RECT rc, COLORREF color, BOOL highlight)
{
    HPEN hPen;          // 테두리용 펜
    HBRUSH hBrush;      // 채우기용 브러시
    HPEN oldPen;        // 원래 펜 보관용
    HBRUSH oldBrush;    // 원래 브러시 보관용
    int cx;             // 중심 x 좌표
    int cy;             // 중심 y 좌표

    // 선택 여부에 따라 펜 생성
    hPen = CreatePen(PS_SOLID, highlight ? 4 : 2,
        highlight ? RGB(255, 0, 0) : RGB(0, 0, 0));

    // 내부 채우기 브러시 생성
    hBrush = CreateSolidBrush(color);

    // 새 펜 선택
    oldPen = (HPEN)SelectObject(hdc, hPen);

    // 새 브러시 선택
    oldBrush = (HBRUSH)SelectObject(hdc, hBrush);

    // 중심 좌표 계산
    cx = CenterX(rc);
    cy = CenterY(rc);

    // 일반 파이와 반대 방향처럼 보이도록 시작점/끝점을 바꿔 그림
    Pie(hdc, rc.left, rc.top, rc.right, rc.bottom,
        cx, rc.top,
        rc.right, cy);

    // 원래 펜 복구
    SelectObject(hdc, oldPen);

    // 원래 브러시 복구
    SelectObject(hdc, oldBrush);

    // 생성한 펜 삭제
    DeleteObject(hPen);

    // 생성한 브러시 삭제
    DeleteObject(hBrush);
}


/* ---------------- 도형 선택 출력 함수 ---------------- */

// 바깥쪽 도형을 종류에 따라 선택해서 그리는 함수
void DrawOuterShape(HDC hdc, int shapeType, RECT rc, COLORREF color, BOOL highlight)
{
    // 현재 도형 종류에 따라 알맞은 그리기 함수 호출
    switch (shapeType)
    {
    case SHAPE_CIRCLE:
        // 원이면 원 그리기
        DrawCircle(hdc, rc, color, highlight);
        break;

    case SHAPE_HOURGLASS:
        // 모래시계면 일반 세로 모래시계 그리기
        DrawHourglass(hdc, rc, color, highlight);
        break;

    case SHAPE_PENTAGON:
        // 오각형이면 일반 오각형 그리기
        DrawPentagon(hdc, rc, color, highlight, FALSE);
        break;

    case SHAPE_PIE:
        // 파이면 일반 파이 그리기
        DrawPieNormal(hdc, rc, color, highlight);
        break;
    }
}

// 중앙 사각형 안의 변형 도형을 종류에 따라 선택해서 그리는 함수
void DrawCenterShape(HDC hdc, int shapeType, RECT rc, COLORREF color)
{
    // 현재 선택된 도형 종류에 따라 중앙에 다른 모양을 그림
    switch (shapeType)
    {
    case SHAPE_CIRCLE:
        // 원 선택 시 중앙에는 타원/원 계열 출력
        DrawCircle(hdc, rc, color, FALSE);
        break;

    case SHAPE_HOURGLASS:
        // 모래시계 선택 시 중앙에는 가로 모래시계(><) 출력
        DrawHourglassRotated(hdc, rc, color, FALSE);
        break;

    case SHAPE_PENTAGON:
        // 오각형 선택 시 중앙에는 뒤집힌 오각형 출력
        DrawPentagon(hdc, rc, color, FALSE, TRUE);
        break;

    case SHAPE_PIE:
        // 파이 선택 시 중앙에는 남은 파이 모양 출력
        DrawPieRemain(hdc, rc, color, FALSE);
        break;
    }
}


/* ---------------- 위치 변경 함수 ---------------- */

// 바깥 도형들을 시계 방향으로 한 칸씩 회전시키는 함수
void RotateClockwise(void)
{
    int temp;   // 임시 저장 변수

    // 아래쪽 도형을 임시 저장
    temp = g_slotShape[SLOT_BOTTOM];

    // 오른쪽 도형을 아래로 이동
    g_slotShape[SLOT_BOTTOM] = g_slotShape[SLOT_RIGHT];

    // 위쪽 도형을 오른쪽으로 이동
    g_slotShape[SLOT_RIGHT] = g_slotShape[SLOT_TOP];

    // 왼쪽 도형을 위로 이동
    g_slotShape[SLOT_TOP] = g_slotShape[SLOT_LEFT];

    // 원래 아래쪽 도형을 왼쪽으로 이동
    g_slotShape[SLOT_LEFT] = temp;

    // 문제 조건상 위치가 바뀌면 위쪽 칸 도형이 중앙에 출력되므로 선택 도형을 위쪽 도형으로 변경
    g_selectedShape = g_slotShape[SLOT_TOP];
}

// 바깥 도형들을 반시계 방향으로 한 칸씩 회전시키는 함수
void RotateCounterClockwise(void)
{
    int temp;   // 임시 저장 변수

    // 왼쪽 도형을 임시 저장
    temp = g_slotShape[SLOT_LEFT];

    // 위쪽 도형을 왼쪽으로 이동
    g_slotShape[SLOT_LEFT] = g_slotShape[SLOT_TOP];

    // 오른쪽 도형을 위쪽으로 이동
    g_slotShape[SLOT_TOP] = g_slotShape[SLOT_RIGHT];

    // 아래쪽 도형을 오른쪽으로 이동
    g_slotShape[SLOT_RIGHT] = g_slotShape[SLOT_BOTTOM];

    // 원래 왼쪽 도형을 아래쪽으로 이동
    g_slotShape[SLOT_BOTTOM] = temp;

    // 회전 후 위쪽 칸 도형을 중앙에 표시
    g_selectedShape = g_slotShape[SLOT_TOP];
}

// 위쪽과 아래쪽 위치의 도형을 서로 바꾸는 함수
void SwapTopBottom(void)
{
    int temp;   // 임시 저장 변수

    // 위쪽 도형 임시 저장
    temp = g_slotShape[SLOT_TOP];

    // 아래쪽 도형을 위로 올림
    g_slotShape[SLOT_TOP] = g_slotShape[SLOT_BOTTOM];

    // 원래 위쪽 도형을 아래로 내림
    g_slotShape[SLOT_BOTTOM] = temp;

    // 문제 조건에 따라 위쪽 칸 도형이 중앙에 출력되도록 선택 도형 갱신
    g_selectedShape = g_slotShape[SLOT_TOP];
}

// 왼쪽과 오른쪽 위치의 도형을 서로 바꾸는 함수
void SwapLeftRight(void)
{
    int temp;   // 임시 저장 변수

    // 왼쪽 도형 임시 저장
    temp = g_slotShape[SLOT_LEFT];

    // 오른쪽 도형을 왼쪽으로 이동
    g_slotShape[SLOT_LEFT] = g_slotShape[SLOT_RIGHT];

    // 원래 왼쪽 도형을 오른쪽으로 이동
    g_slotShape[SLOT_RIGHT] = temp;

    // 좌우를 바꿔도 중앙에는 위쪽 칸 도형이 출력되어야 하므로 선택 도형을 위쪽 칸 도형으로 맞춤
    g_selectedShape = g_slotShape[SLOT_TOP];
}


/* ---------------- WinMain ---------------- */

// 프로그램의 시작 함수
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
    HWND hWnd;           // 생성될 메인 윈도우의 핸들
    MSG Message;         // 메시지 루프에서 사용할 메시지 구조체
    WNDCLASSEX WndClass; // 윈도우 클래스 정보를 담을 구조체

    // 전달받은 현재 프로그램 인스턴스 핸들을 전역 변수에 저장
    g_hInst = hInstance;

    // 구조체 크기 설정
    WndClass.cbSize = sizeof(WndClass);

    // 창을 가로/세로로 다시 그릴 필요가 있을 때 WM_PAINT가 잘 일어나도록 스타일 설정
    WndClass.style = CS_HREDRAW | CS_VREDRAW;

    // 메시지 처리 함수 연결
    WndClass.lpfnWndProc = WndProc;

    // 클래스 추가 메모리 사용 안 함
    WndClass.cbClsExtra = 0;

    // 윈도우 추가 메모리 사용 안 함
    WndClass.cbWndExtra = 0;

    // 인스턴스 핸들 저장
    WndClass.hInstance = hInstance;

    // 기본 응용프로그램 아이콘 사용
    WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);

    // 기본 화살표 커서 사용
    WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);

    // 배경은 흰색 브러시 사용
    WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);

    // 메뉴는 사용하지 않음
    WndClass.lpszMenuName = NULL;

    // 클래스 이름 설정
    WndClass.lpszClassName = lpszClass;

    // 작은 아이콘도 기본 응용프로그램 아이콘 사용
    WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    // 윈도우 클래스 등록
    RegisterClassEx(&WndClass);

    // 실제 창 생성
    hWnd = CreateWindow(
        lpszClass,          // 등록된 클래스 이름
        lpszWindowName,     // 창 제목
        WS_OVERLAPPEDWINDOW,// 일반적인 윈도우 스타일
        100, 100,           // 창이 처음 나타날 x, y 위치
        900, 700,           // 창의 너비와 높이
        NULL,               // 부모 창 없음
        NULL,               // 메뉴 없음
        hInstance,          // 현재 인스턴스 핸들 전달
        NULL                // 추가 데이터 없음
    );

    // 창을 화면에 보이게 함
    ShowWindow(hWnd, nCmdShow);

    // 창을 즉시 다시 그리도록 요청
    UpdateWindow(hWnd);

    // 메시지 루프 시작
    while (GetMessage(&Message, NULL, 0, 0))
    {
        // 키보드 메시지 번역
        TranslateMessage(&Message);

        // 실제 메시지 처리 함수(WndProc)로 전달
        DispatchMessage(&Message);
    }

    // 프로그램 종료 시 운영체제에 종료 코드 반환
    return (int)Message.wParam;
}


/* ---------------- WndProc ---------------- */

// 윈도우 메시지를 처리하는 핵심 함수
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // 메시지 종류에 따라 분기
    switch (uMsg)
    {
    case WM_CREATE:
    {
        int i;  // 반복문 인덱스

        // 난수 시드를 현재 시간으로 초기화해서 실행할 때마다 다른 랜덤값이 나오게 함
        srand((unsigned int)time(NULL));

        // 처음 시작할 때 왼쪽에는 원 배치
        g_slotShape[SLOT_LEFT] = SHAPE_CIRCLE;

        // 처음 시작할 때 위쪽에는 모래시계 배치
        g_slotShape[SLOT_TOP] = SHAPE_HOURGLASS;

        // 처음 시작할 때 오른쪽에는 오각형 배치
        g_slotShape[SLOT_RIGHT] = SHAPE_PENTAGON;

        // 처음 시작할 때 아래쪽에는 파이 배치
        g_slotShape[SLOT_BOTTOM] = SHAPE_PIE;

        // 4개의 도형 색을 각각 랜덤으로 초기화
        for (i = 0; i < 4; i++)
        {
            // 기본 색을 랜덤으로 설정
            g_shapes[i].baseColor = RandomColor();

            // 처음엔 현재 그릴 색도 기본 색과 같게 설정
            g_shapes[i].drawColor = g_shapes[i].baseColor;
        }

        // 시작 시 선택 도형을 0~3 중 랜덤하게 정함
        g_selectedShape = rand() % 4;

        // 메시지 처리 완료
        return 0;
    }

    case WM_KEYDOWN:
    {
        // 어떤 키가 눌렸는지에 따라 처리
        switch (wParam)
        {
        case 'C':
        case 'c':
            // 현재 선택 도형을 원으로 설정
            g_selectedShape = SHAPE_CIRCLE;

            // 아직 c 키가 눌린 상태가 아니었다면
            if (g_keyPressed[SHAPE_CIRCLE] == FALSE)
            {
                // 원의 현재 색을 랜덤 색으로 바꿈
                g_shapes[SHAPE_CIRCLE].drawColor = RandomColor();

                // 이제 원 키가 눌린 상태라고 기록
                g_keyPressed[SHAPE_CIRCLE] = TRUE;
            }
            break;

        case 'S':
        case 's':
            // 현재 선택 도형을 모래시계로 설정
            g_selectedShape = SHAPE_HOURGLASS;

            // 아직 s 키가 눌린 상태가 아니었다면
            if (g_keyPressed[SHAPE_HOURGLASS] == FALSE)
            {
                // 모래시계 색을 랜덤으로 변경
                g_shapes[SHAPE_HOURGLASS].drawColor = RandomColor();

                // 눌린 상태로 기록
                g_keyPressed[SHAPE_HOURGLASS] = TRUE;
            }
            break;

        case 'P':
        case 'p':
            // 현재 선택 도형을 오각형으로 설정
            g_selectedShape = SHAPE_PENTAGON;

            // 아직 p 키가 눌린 상태가 아니었다면
            if (g_keyPressed[SHAPE_PENTAGON] == FALSE)
            {
                // 오각형 색을 랜덤으로 변경
                g_shapes[SHAPE_PENTAGON].drawColor = RandomColor();

                // 눌린 상태로 기록
                g_keyPressed[SHAPE_PENTAGON] = TRUE;
            }
            break;

        case 'E':
        case 'e':
            // 현재 선택 도형을 파이로 설정
            g_selectedShape = SHAPE_PIE;

            // 아직 e 키가 눌린 상태가 아니었다면
            if (g_keyPressed[SHAPE_PIE] == FALSE)
            {
                // 파이 색을 랜덤으로 변경
                g_shapes[SHAPE_PIE].drawColor = RandomColor();

                // 눌린 상태로 기록
                g_keyPressed[SHAPE_PIE] = TRUE;
            }
            break;

        case VK_LEFT:
            // 왼쪽 방향키면 반시계 방향 회전
            RotateCounterClockwise();
            break;

        case VK_RIGHT:
            // 오른쪽 방향키면 시계 방향 회전
            RotateClockwise();
            break;

        case VK_UP:
            // 위쪽 방향키면 위/아래 도형 교환
            SwapTopBottom();
            break;

        case VK_DOWN:
            // 아래쪽 방향키면 좌/우 도형 교환
            SwapLeftRight();
            break;
        }

        // 키 처리 후 화면을 다시 그리도록 요청
        InvalidateRect(hWnd, NULL, TRUE);

        // 메시지 처리 완료
        return 0;
    }

    case WM_KEYUP:
    {
        // 키에서 손을 뗐을 때 처리
        switch (wParam)
        {
        case 'C':
        case 'c':
            // 원의 현재 색을 기본 색으로 되돌림
            g_shapes[SHAPE_CIRCLE].drawColor = g_shapes[SHAPE_CIRCLE].baseColor;

            // 더 이상 눌린 상태가 아니라고 기록
            g_keyPressed[SHAPE_CIRCLE] = FALSE;
            break;

        case 'S':
        case 's':
            // 모래시계 현재 색을 기본 색으로 되돌림
            g_shapes[SHAPE_HOURGLASS].drawColor = g_shapes[SHAPE_HOURGLASS].baseColor;

            // 더 이상 눌린 상태가 아니라고 기록
            g_keyPressed[SHAPE_HOURGLASS] = FALSE;
            break;

        case 'P':
        case 'p':
            // 오각형 현재 색을 기본 색으로 되돌림
            g_shapes[SHAPE_PENTAGON].drawColor = g_shapes[SHAPE_PENTAGON].baseColor;

            // 더 이상 눌린 상태가 아니라고 기록
            g_keyPressed[SHAPE_PENTAGON] = FALSE;
            break;

        case 'E':
        case 'e':
            // 파이 현재 색을 기본 색으로 되돌림
            g_shapes[SHAPE_PIE].drawColor = g_shapes[SHAPE_PIE].baseColor;

            // 더 이상 눌린 상태가 아니라고 기록
            g_keyPressed[SHAPE_PIE] = FALSE;
            break;
        }

        // 키를 뗀 뒤에도 색이 복귀되었으므로 다시 그리기 요청
        InvalidateRect(hWnd, NULL, TRUE);

        // 메시지 처리 완료
        return 0;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;   // 그리기 정보를 저장하는 구조체
        HDC hDC;          // 디바이스 컨텍스트
        RECT client;      // 클라이언트 영역 전체 크기를 저장할 RECT
        int w;            // 클라이언트 영역 너비
        int h;            // 클라이언트 영역 높이
        RECT centerRect;  // 중앙 사각형 영역
        int shapeW;       // 바깥 도형의 너비
        int shapeH;       // 바깥 도형의 높이
        RECT leftRect;    // 왼쪽 도형 영역
        RECT topRect;     // 위쪽 도형 영역
        RECT rightRect;   // 오른쪽 도형 영역
        RECT bottomRect;  // 아래쪽 도형 영역
        HPEN hPen;        // 중앙 사각형 테두리용 펜
        HBRUSH hBrush;    // 중앙 사각형 내부를 비워 두기 위한 브러시
        HPEN oldPen;      // 원래 펜 보관용
        HBRUSH oldBrush;  // 원래 브러시 보관용

        // 실제 그리기 시작
        hDC = BeginPaint(hWnd, &ps);

        // 현재 창의 클라이언트 영역 크기 가져오기
        GetClientRect(hWnd, &client);

        // 창 너비 계산
        w = client.right - client.left;

        // 창 높이 계산
        h = client.bottom - client.top;

        // 중앙 사각형의 왼쪽 좌표 계산
        centerRect.left = w / 2 - 120;

        // 중앙 사각형의 위쪽 좌표 계산
        centerRect.top = h / 2 - 90;

        // 중앙 사각형의 오른쪽 좌표 계산
        centerRect.right = w / 2 + 120;

        // 중앙 사각형의 아래쪽 좌표 계산
        centerRect.bottom = h / 2 + 90;

        // 바깥 도형 너비 지정
        shapeW = 120;

        // 바깥 도형 높이 지정
        shapeH = 120;

        // 왼쪽 도형 사각형의 왼쪽 좌표
        leftRect.left = centerRect.left - 180;

        // 왼쪽 도형 사각형의 위쪽 좌표
        leftRect.top = h / 2 - shapeH / 2;

        // 왼쪽 도형 사각형의 오른쪽 좌표
        leftRect.right = leftRect.left + shapeW;

        // 왼쪽 도형 사각형의 아래쪽 좌표
        leftRect.bottom = leftRect.top + shapeH;

        // 위쪽 도형 사각형의 왼쪽 좌표
        topRect.left = w / 2 - shapeW / 2;

        // 위쪽 도형 사각형의 위쪽 좌표
        topRect.top = centerRect.top - 160;

        // 위쪽 도형 사각형의 오른쪽 좌표
        topRect.right = topRect.left + shapeW;

        // 위쪽 도형 사각형의 아래쪽 좌표
        topRect.bottom = topRect.top + shapeH;

        // 오른쪽 도형 사각형의 왼쪽 좌표
        rightRect.left = centerRect.right + 60;

        // 오른쪽 도형 사각형의 위쪽 좌표
        rightRect.top = h / 2 - shapeH / 2;

        // 오른쪽 도형 사각형의 오른쪽 좌표
        rightRect.right = rightRect.left + shapeW;

        // 오른쪽 도형 사각형의 아래쪽 좌표
        rightRect.bottom = rightRect.top + shapeH;

        // 아래쪽 도형 사각형의 왼쪽 좌표
        bottomRect.left = w / 2 - shapeW / 2;

        // 아래쪽 도형 사각형의 위쪽 좌표
        bottomRect.top = centerRect.bottom + 40;

        // 아래쪽 도형 사각형의 오른쪽 좌표
        bottomRect.right = bottomRect.left + shapeW;

        // 아래쪽 도형 사각형의 아래쪽 좌표
        bottomRect.bottom = bottomRect.top + shapeH;

        // 중앙 사각형 테두리를 그릴 검정 펜 생성
        hPen = CreatePen(PS_SOLID, 3, RGB(0, 0, 0));

        // 내부를 비워 두기 위한 투명 브러시 선택
        hBrush = (HBRUSH)GetStockObject(HOLLOW_BRUSH);

        // 새 펜 선택
        oldPen = (HPEN)SelectObject(hDC, hPen);

        // 새 브러시 선택
        oldBrush = (HBRUSH)SelectObject(hDC, hBrush);

        // 중앙 사각형 테두리 그리기
        Rectangle(hDC, centerRect.left, centerRect.top, centerRect.right, centerRect.bottom);

        // 원래 펜 복구
        SelectObject(hDC, oldPen);

        // 원래 브러시 복구
        SelectObject(hDC, oldBrush);

        // 중앙 사각형용 펜 삭제
        DeleteObject(hPen);

        // 왼쪽 위치의 도형을 그림
        DrawOuterShape(
            hDC,
            g_slotShape[SLOT_LEFT],
            leftRect,
            g_shapes[g_slotShape[SLOT_LEFT]].drawColor,
            (g_slotShape[SLOT_LEFT] == g_selectedShape)
        );

        // 위쪽 위치의 도형을 그림
        DrawOuterShape(
            hDC,
            g_slotShape[SLOT_TOP],
            topRect,
            g_shapes[g_slotShape[SLOT_TOP]].drawColor,
            (g_slotShape[SLOT_TOP] == g_selectedShape)
        );

        // 오른쪽 위치의 도형을 그림
        DrawOuterShape(
            hDC,
            g_slotShape[SLOT_RIGHT],
            rightRect,
            g_shapes[g_slotShape[SLOT_RIGHT]].drawColor,
            (g_slotShape[SLOT_RIGHT] == g_selectedShape)
        );

        // 아래쪽 위치의 도형을 그림
        DrawOuterShape(
            hDC,
            g_slotShape[SLOT_BOTTOM],
            bottomRect,
            g_shapes[g_slotShape[SLOT_BOTTOM]].drawColor,
            (g_slotShape[SLOT_BOTTOM] == g_selectedShape)
        );

        // 중앙 사각형 안에 현재 선택된 도형의 변형 버전을 그림
        DrawCenterShape(
            hDC,
            g_selectedShape,
            centerRect,
            g_shapes[g_selectedShape].drawColor
        );

        // 그리기 종료
        EndPaint(hWnd, &ps);

        // 메시지 처리 완료
        return 0;
    }

    case WM_DESTROY:
        // 창이 닫히면 메시지 루프를 끝내기 위해 종료 메시지를 보냄
        PostQuitMessage(0);

        // 메시지 처리 완료
        return 0;
    }

    // 위에서 처리하지 않은 메시지는 기본 처리 함수에 맡김
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}