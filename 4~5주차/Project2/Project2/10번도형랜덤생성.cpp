#include <windows.h>   // 윈도우 API 함수, 자료형(HWND, HDC, RECT 등)을 사용하기 위한 헤더
#include <tchar.h>     // TCHAR, LPCTSTR, 유니코드 문자열(L"...") 관련 헤더
#include <stdlib.h>    // rand(), srand() 같은 난수 함수 사용을 위한 헤더
#include <time.h>      // time() 함수 사용을 위한 헤더



// 현재 실행 중인 프로그램(인스턴스)의 핸들을 저장하는 전역 변수
HINSTANCE g_hInst;

// 윈도우 클래스를 구분하기 위한 이름
LPCTSTR lpszClass = L"My Window Class";

// 실제 창의 제목 표시줄에 보일 문자열
LPCTSTR lpszWindowName = L"Window Programming Lab";



// 윈도우 메시지를 처리하는 함수의 원형 선언
// 나중에 아래에서 실제 내용을 구현한다.
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);



// -------------------------------
// 구조체 선언 부분
// -------------------------------

// 삼각형 1개의 정보를 저장하기 위한 구조체
typedef struct
{
    POINT pt[3];        // 삼각형의 3개 꼭짓점 좌표를 저장하는 배열
    // pt[0], pt[1], pt[2] 각각이 x, y 좌표를 가짐

    COLORREF color;     // 삼각형 내부를 채울 색상 정보
    // RGB(r, g, b) 형태로 만들어지는 값이 저장됨

    int number;         // 삼각형 안에 표시할 랜덤 숫자
} TRIANGLE;



// 사각형 1개의 정보를 저장하기 위한 구조체
typedef struct
{
    RECT rc;            // 사각형의 위치와 크기를 저장하는 RECT 구조체
    // left, top, right, bottom 네 값으로 구성됨

    COLORREF color;     // 사각형 내부 채우기 색상

    int number;         // 사각형 안에 표시할 숫자
} RECT_SHAPE;



// 원(정확히는 타원) 1개의 정보를 저장하기 위한 구조체
typedef struct
{
    RECT rc;            // 원을 감싸는 사각형 영역
    // Ellipse 함수는 이 사각형 안에 타원을 그림

    COLORREF color;     // 원 내부 채우기 색상

    int number;         // 원 안에 표시할 숫자
} CIRCLE_SHAPE;



// -------------------------------
// 전역 변수 부분
// -------------------------------

// 현재 삼각형이 몇 개 생성되었는지를 저장
int triCount = 0;

// 현재 사각형이 몇 개 생성되었는지를 저장
int rectCount = 0;

// 현재 원이 몇 개 생성되었는지를 저장
int circleCount = 0;



// 삼각형 정보를 최대 5개 저장하는 배열
TRIANGLE triangles[5];

// 사각형 정보를 최대 5개 저장하는 배열
RECT_SHAPE rects[5];

// 원 정보를 최대 5개 저장하는 배열
CIRCLE_SHAPE circles[5];



// 화면에서 삼각형이 배치될 영역
RECT areaTri = { 0, 0, 350, 280 };

// 화면에서 사각형이 배치될 영역
RECT areaRect = { 370, 0, 780, 280 };

// 화면에서 원이 배치될 영역
RECT areaCircle = { 0, 370, 780, 550 };



// -------------------------------
// 보조 함수 부분
// -------------------------------

// 랜덤 색상을 하나 만들어서 반환하는 함수
COLORREF GetRandomColor()
{
    // rand() % 256 은 0 ~ 255 중 하나의 값을 만듦
    // RGB 함수는 빨강, 초록, 파랑 값을 합쳐 하나의 COLORREF 값을 만듦
    return RGB(rand() % 256, rand() % 256, rand() % 256);
}



// 랜덤 숫자를 하나 만들어서 반환하는 함수
int GetRandomNumber()
{
    // 0 ~ 99 사이의 정수를 반환
    return rand() % 100;
}



// 두 개의 RECT가 서로 겹치는지 검사하는 함수
int IsOverlapRect(RECT a, RECT b)
{
    RECT temp;  // 두 RECT의 교집합 영역을 임시로 저장할 변수

    // IntersectRect는 두 RECT가 겹치면 temp에 겹치는 부분을 저장하고 TRUE 반환
    // 겹치지 않으면 FALSE 반환
    return IntersectRect(&temp, &a, &b);
}



// 삼각형의 3개 꼭짓점을 보고 그 삼각형을 감싸는 최소 사각형(외접 사각형)을 구하는 함수
RECT GetTriangleRect(TRIANGLE t)
{
    RECT r;     // 결과로 반환할 사각형
    int i;      // 반복문에 사용할 변수

    // 처음에는 첫 번째 꼭짓점의 x, y 값을 기준으로 초기화
    r.left = t.pt[0].x;
    r.right = t.pt[0].x;
    r.top = t.pt[0].y;
    r.bottom = t.pt[0].y;

    // 나머지 꼭짓점들과 비교하면서
    // 가장 작은 x -> left
    // 가장 큰 x -> right
    // 가장 작은 y -> top
    // 가장 큰 y -> bottom
    // 으로 갱신한다.
    for (i = 1; i < 3; i++)
    {
        if (t.pt[i].x < r.left)   r.left = t.pt[i].x;
        if (t.pt[i].x > r.right)  r.right = t.pt[i].x;
        if (t.pt[i].y < r.top)    r.top = t.pt[i].y;
        if (t.pt[i].y > r.bottom) r.bottom = t.pt[i].y;
    }

    // 완성된 외접 사각형 반환
    return r;
}



// 전달받은 RECT를 상하좌우로 조금씩 확장하는 함수
// gap 값만큼 바깥으로 늘려서 검사하면 도형끼리 딱 붙지 않게 간격을 줄 수 있다.
RECT ExpandRect(RECT r, int gap)
{
    r.left -= gap;      // 왼쪽 경계를 gap만큼 더 왼쪽으로 이동
    r.top -= gap;       // 위쪽 경계를 gap만큼 더 위로 이동
    r.right += gap;     // 오른쪽 경계를 gap만큼 더 오른쪽으로 이동
    r.bottom += gap;    // 아래쪽 경계를 gap만큼 더 아래로 이동

    return r;           // 확장된 RECT 반환
}



// 어떤 사각형 영역의 중앙에 숫자를 출력하는 함수
void DrawCenteredNumber(HDC hDC, RECT rc, int number)
{
    TCHAR str[20];      // 숫자를 문자열로 바꿔 저장할 배열

    // 정수 number를 문자열로 변환해서 str에 저장
    wsprintf(str, L"%d", number);

    // 글자 배경을 투명하게 설정
    // 배경 흰색 박스 없이 글자만 그려짐
    SetBkMode(hDC, TRANSPARENT);

    // DrawText를 사용해서 rc 영역 정중앙에 문자열 출력
    // DT_CENTER : 좌우 가운데 정렬
    // DT_VCENTER : 상하 가운데 정렬
    // DT_SINGLELINE : 한 줄로 출력
    DrawText(hDC, str, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}



// -------------------------------
// 도형 생성 함수 부분
// -------------------------------

// 삼각형 영역에 삼각형들을 랜덤하게 다시 생성하는 함수
void MakeTriangles()
{
    int i, j;   // 반복문용 변수

    // 삼각형 개수를 1 ~ 5개 사이에서 랜덤으로 결정
    triCount = rand() % 5 + 1;

    // triCount 개수만큼 삼각형을 생성
    for (i = 0; i < triCount; i++)
    {
        int retry = 0;      // 현재 i번째 삼각형을 몇 번 다시 시도했는지 저장
        int success = 0;    // 현재 삼각형 생성 성공 여부 저장 (0: 실패, 1: 성공)

        // 너무 많이 실패하면 무한 반복이 되므로 최대 100번까지만 시도
        while (retry < 100)
        {
            TRIANGLE temp;  // 새로 만들 삼각형 정보를 임시 저장할 변수
            RECT newRect;   // 새 삼각형의 외접 사각형을 저장할 변수
            int overlap = 0; // 기존 삼각형과 겹쳤는지 표시하는 변수

            // 삼각형 크기를 30 ~ 69 사이에서 랜덤 선택
            int size = rand() % 40 + 30;

            // 삼각형의 꼭대기 x 좌표를 랜덤으로 생성
            // 영역 바깥으로 튀어나가지 않게 size와 여백을 고려해서 생성
            int x = rand() % (areaTri.right - areaTri.left - size - 40) + areaTri.left + 20 + size / 2;

            // 삼각형의 꼭대기 y 좌표를 랜덤으로 생성
            int y = rand() % (areaTri.bottom - areaTri.top - size - 40) + areaTri.top + 20;

            // 첫 번째 꼭짓점(위쪽 꼭짓점) 좌표 저장
            temp.pt[0].x = x;
            temp.pt[0].y = y;

            // 두 번째 꼭짓점(왼쪽 아래) 좌표 저장
            temp.pt[1].x = x - size / 2;
            temp.pt[1].y = y + size;

            // 세 번째 꼭짓점(오른쪽 아래) 좌표 저장
            temp.pt[2].x = x + size / 2;
            temp.pt[2].y = y + size;

            // 삼각형 색상 랜덤 생성
            temp.color = GetRandomColor();

            // 삼각형 안에 들어갈 숫자 랜덤 생성
            temp.number = GetRandomNumber();

            // 새 삼각형을 감싸는 외접 사각형 구하기
            newRect = GetTriangleRect(temp);

            // 도형끼리 딱 붙는 것도 피하려고 사각형을 5만큼 확장
            newRect = ExpandRect(newRect, 5);

            // 이미 만들어진 이전 삼각형들과 겹치는지 검사
            for (j = 0; j < i; j++)
            {
                RECT oldRect = GetTriangleRect(triangles[j]); // 기존 삼각형의 외접 사각형 구하기
                oldRect = ExpandRect(oldRect, 5);            // 기존 삼각형도 같은 기준으로 확장

                // 새 삼각형과 기존 삼각형이 겹치면 overlap을 1로 바꾸고 중단
                if (IsOverlapRect(newRect, oldRect))
                {
                    overlap = 1;
                    break;
                }
            }

            // 겹치지 않았다면 이 삼각형은 배치 성공
            if (!overlap)
            {
                triangles[i] = temp; // 임시 temp를 실제 배열에 저장
                success = 1;         // 성공 표시
                break;               // while 종료
            }

            // 겹쳤다면 다시 시도 횟수 증가
            retry++;
        }

        // 100번 시도해도 실패했다면 더 이상 배치가 어렵다고 보고
        // 현재까지 성공한 개수까지만 사용
        if (!success)
        {
            triCount = i; // 실제 개수를 지금까지 성공한 개수로 줄임
            break;        // for 종료
        }
    }
}



// 사각형 영역에 사각형들을 랜덤하게 다시 생성하는 함수
void MakeRects()
{
    int i, j;   // 반복문용 변수

    // 사각형 개수를 1 ~ 5개 사이에서 랜덤 결정
    rectCount = rand() % 5 + 1;

    // rectCount 개수만큼 사각형 생성
    for (i = 0; i < rectCount; i++)
    {
        int retry = 0;      // 현재 사각형을 다시 시도한 횟수
        int success = 0;    // 현재 사각형 생성 성공 여부

        // 최대 100번까지 시도
        while (retry < 100)
        {
            RECT_SHAPE temp; // 새 사각형 정보를 임시 저장
            RECT newRect;    // 새 사각형의 RECT
            int overlap = 0; // 겹침 여부

            // 사각형 가로 길이를 30 ~ 89 사이에서 랜덤 생성
            int width = rand() % 60 + 30;

            // 사각형 세로 길이를 30 ~ 89 사이에서 랜덤 생성
            int height = rand() % 60 + 30;

            // 사각형의 왼쪽 좌표를 영역 안에서 랜덤 생성
            int left = rand() % (areaRect.right - areaRect.left - width - 40) + areaRect.left + 20;

            // 사각형의 위쪽 좌표를 영역 안에서 랜덤 생성
            int top = rand() % (areaRect.bottom - areaRect.top - height - 40) + areaRect.top + 20;

            // 임시 사각형의 좌표 저장
            temp.rc.left = left;
            temp.rc.top = top;
            temp.rc.right = left + width;
            temp.rc.bottom = top + height;

            // 색상 랜덤 생성
            temp.color = GetRandomColor();

            // 숫자 랜덤 생성
            temp.number = GetRandomNumber();

            // 새 사각형을 약간 확장해서 충돌 검사에 사용
            newRect = ExpandRect(temp.rc, 5);

            // 이미 생성된 이전 사각형들과 겹치는지 검사
            for (j = 0; j < i; j++)
            {
                RECT oldRect = ExpandRect(rects[j].rc, 5); // 기존 사각형도 동일하게 확장

                // 겹치면 overlap = 1
                if (IsOverlapRect(newRect, oldRect))
                {
                    overlap = 1;
                    break;
                }
            }

            // 겹치지 않으면 성공적으로 저장
            if (!overlap)
            {
                rects[i] = temp; // 배열에 저장
                success = 1;     // 성공 표시
                break;           // while 종료
            }

            // 겹쳤다면 재시도 횟수 증가
            retry++;
        }

        // 끝까지 실패하면 현재까지 성공한 개수만 사용
        if (!success)
        {
            rectCount = i;
            break;
        }
    }
}



// 원 영역에 원들을 랜덤하게 다시 생성하는 함수
void MakeCircles()
{
    int i, j;   // 반복문용 변수

    // 원 개수를 1 ~ 5개 사이에서 랜덤 결정
    circleCount = rand() % 5 + 1;

    // circleCount 개수만큼 원 생성
    for (i = 0; i < circleCount; i++)
    {
        int retry = 0;      // 현재 원의 재시도 횟수
        int success = 0;    // 현재 원 생성 성공 여부

        // 최대 100번까지 시도
        while (retry < 100)
        {
            CIRCLE_SHAPE temp; // 새 원 정보를 임시 저장
            RECT newRect;      // 새 원의 바깥 RECT
            int overlap = 0;   // 겹침 여부

            // 원의 지름을 30 ~ 79 사이에서 랜덤 생성
            int size = rand() % 50 + 30;

            // 원의 왼쪽 좌표를 영역 안에서 랜덤 생성
            int left = rand() % (areaCircle.right - areaCircle.left - size - 40) + areaCircle.left + 20;

            // 원의 위쪽 좌표를 영역 안에서 랜덤 생성
            int top = rand() % (areaCircle.bottom - areaCircle.top - size - 40) + areaCircle.top + 20;

            // 원을 감싸는 사각형 좌표 저장
            temp.rc.left = left;
            temp.rc.top = top;
            temp.rc.right = left + size;
            temp.rc.bottom = top + size;

            // 색상 랜덤 생성
            temp.color = GetRandomColor();

            // 숫자 랜덤 생성
            temp.number = GetRandomNumber();

            // 원의 RECT를 약간 키워서 충돌 검사
            newRect = ExpandRect(temp.rc, 5);

            // 이전에 생성된 원들과 겹치는지 검사
            for (j = 0; j < i; j++)
            {
                RECT oldRect = ExpandRect(circles[j].rc, 5); // 기존 원도 확장

                // 겹치면 overlap = 1
                if (IsOverlapRect(newRect, oldRect))
                {
                    overlap = 1;
                    break;
                }
            }

            // 겹치지 않으면 성공적으로 저장
            if (!overlap)
            {
                circles[i] = temp; // 배열에 저장
                success = 1;       // 성공 표시
                break;             // while 종료
            }

            // 겹쳤으면 다시 시도
            retry++;
        }

        // 계속 실패하면 현재까지 성공한 개수만 사용
        if (!success)
        {
            circleCount = i;
            break;
        }
    }
}



// 전체 영역의 도형을 모두 새로 생성하는 함수
void MakeAllShapes()
{
    MakeTriangles(); // 삼각형 영역 다시 생성
    MakeRects();     // 사각형 영역 다시 생성
    MakeCircles();   // 원 영역 다시 생성
}



// -------------------------------
// WinMain 함수
// -------------------------------

// 프로그램 시작점
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpszCmdParam, int nCmdShow)
{
    HWND hWnd;           // 생성될 창의 핸들을 저장할 변수
    MSG Message;         // 메시지 루프에서 사용할 메시지 구조체
    WNDCLASSEX WndClass; // 윈도우 클래스 정보를 담는 구조체

    g_hInst = hInstance; // 현재 프로그램 인스턴스 핸들을 전역 변수에 저장

    // 윈도우 클래스 구조체의 크기 설정
    WndClass.cbSize = sizeof(WndClass);

    // 창의 가로/세로 크기가 바뀌면 다시 그리도록 설정
    WndClass.style = CS_HREDRAW | CS_VREDRAW;

    // 메시지를 처리할 함수 지정
    WndClass.lpfnWndProc = WndProc;

    // 클래스 추가 메모리 사용 안 함
    WndClass.cbClsExtra = 0;

    // 윈도우 추가 메모리 사용 안 함
    WndClass.cbWndExtra = 0;

    // 이 클래스를 만든 인스턴스 지정
    WndClass.hInstance = hInstance;

    // 창의 큰 아이콘 지정
    WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);

    // 기본 마우스 커서 지정
    WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);

    // 창 배경을 흰색 브러시로 칠하도록 설정
    WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);

    // 메뉴 사용 안 함
    WndClass.lpszMenuName = NULL;

    // 윈도우 클래스 이름 지정
    WndClass.lpszClassName = lpszClass;

    // 작은 아이콘 지정
    WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    // 위에서 설정한 윈도우 클래스를 시스템에 등록
    RegisterClassEx(&WndClass);

    // 실제 화면에 보일 창 생성
    hWnd = CreateWindow(
        lpszClass,          // 사용할 윈도우 클래스 이름
        lpszWindowName,     // 창 제목
        WS_OVERLAPPEDWINDOW,// 일반적인 윈도우 스타일
        100, 100,           // 창의 시작 위치(x, y)
        800, 600,           // 창의 크기(width, height)
        NULL,               // 부모 윈도우 없음
        NULL,               // 메뉴 없음
        hInstance,          // 현재 인스턴스
        NULL                // 추가 데이터 없음
    );

    // 생성된 창을 화면에 표시
    ShowWindow(hWnd, nCmdShow);

    // 창을 즉시 한 번 그리도록 갱신
    UpdateWindow(hWnd);

    // 메시지 루프 시작
    // 프로그램이 종료될 때까지 계속 반복
    while (GetMessage(&Message, 0, 0, 0))
    {
        TranslateMessage(&Message); // 키보드 메시지를 문자 메시지로 변환
        DispatchMessage(&Message);  // 메시지를 WndProc에 전달
    }

    // 프로그램 종료 코드 반환
    return (int)Message.wParam;
}



// -------------------------------
// WndProc 함수
// -------------------------------

// 모든 윈도우 메시지를 처리하는 함수
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps; // WM_PAINT 처리 시 사용하는 구조체
    HDC hDC;        // 화면에 그림을 그릴 때 사용하는 디바이스 컨텍스트 핸들

    // 들어온 메시지 종류에 따라 분기 처리
    switch (uMsg)
    {
    case WM_CREATE:
        // 창이 처음 생성될 때 한 번 호출됨

        // 난수 시드 설정
        // time(NULL)을 사용해야 실행할 때마다 다른 난수가 나옴
        srand((unsigned int)time(NULL));

        // 프로그램 시작 시 처음 도형들을 한 번 생성
        MakeAllShapes();
        break;

    case WM_KEYDOWN:
        // 특수 키(Enter, 방향키, F키 등)를 처리할 때 주로 사용

        // Enter 키를 누르면 전체 도형을 모두 새로 생성
        if (wParam == VK_RETURN)
        {
            MakeAllShapes();                    // 삼각형, 사각형, 원 모두 다시 생성
            InvalidateRect(hWnd, NULL, TRUE);  // 창 전체를 무효화해서 다시 그리게 함
        }
        break;

    case WM_CHAR:
        // 일반 문자 입력 처리용 메시지
        // 숫자 키 '1', '2', '3' 같은 입력 처리에 적합

        // 1 키를 누르면 삼각형 영역만 다시 생성
        if (wParam == '1')
        {
            MakeTriangles();                    // 삼각형만 새로 생성
            InvalidateRect(hWnd, NULL, TRUE);  // 다시 그리기 요청
        }
        // 2 키를 누르면 사각형 영역만 다시 생성
        else if (wParam == '2')
        {
            MakeRects();                        // 사각형만 새로 생성
            InvalidateRect(hWnd, NULL, TRUE);  // 다시 그리기 요청
        }
        // 3 키를 누르면 원 영역만 다시 생성
        else if (wParam == '3')
        {
            MakeCircles();                      // 원만 새로 생성
            InvalidateRect(hWnd, NULL, TRUE);  // 다시 그리기 요청
        }
        break;

    case WM_PAINT:
    {
        // WM_PAINT는 창이 다시 그려져야 할 때 호출됨
        int i;              // 반복문용 변수
        HFONT hFont;        // 새로 만들 폰트 핸들
        HFONT oldFont;      // 원래 선택되어 있던 폰트를 저장할 변수

        // 실제 그리기 시작
        hDC = BeginPaint(hWnd, &ps);

        // 숫자를 보기 좋게 출력하기 위한 폰트 생성
        hFont = CreateFont(
            24,                 // 글자 높이
            0,                  // 글자 너비(0이면 자동)
            0,                  // 글자 기울기 각도
            0,                  // 기준선 기울기 각도
            FW_BOLD,            // 굵은 글씨
            FALSE,              // italic 아님
            FALSE,              // underline 아님
            FALSE,              // strikeout 아님
            DEFAULT_CHARSET,    // 기본 문자셋
            OUT_DEFAULT_PRECIS, // 기본 출력 정밀도
            CLIP_DEFAULT_PRECIS,// 기본 클리핑 정밀도
            DEFAULT_QUALITY,    // 기본 품질
            DEFAULT_PITCH | FF_SWISS, // 기본 피치 + 고딕 계열
            L"Arial"            // 폰트 이름
        );

        // 방금 만든 폰트를 DC에 선택하고 기존 폰트는 oldFont에 저장
        oldFont = (HFONT)SelectObject(hDC, hFont);

        // 각 영역의 테두리 사각형을 그림
        Rectangle(hDC, areaTri.left, areaTri.top, areaTri.right, areaTri.bottom);
        Rectangle(hDC, areaRect.left, areaRect.top, areaRect.right, areaRect.bottom);
        Rectangle(hDC, areaCircle.left, areaCircle.top, areaCircle.right, areaCircle.bottom);

        // 각 영역 제목 출력
        TextOut(hDC, 10, 10, L"Area 1 : Triangle", 17);
        TextOut(hDC, 380, 10, L"Area 2 : Rectangle", 18);
        TextOut(hDC, 10, 380, L"Area 3 : Circle", 15);

        // -------------------------
        // 삼각형 그리기 부분
        // -------------------------
        for (i = 0; i < triCount; i++)
        {
            HBRUSH hBrush = CreateSolidBrush(triangles[i].color); // 삼각형 색 브러시 생성
            HBRUSH oldBrush = (HBRUSH)SelectObject(hDC, hBrush);  // 브러시 선택, 기존 브러시는 저장

            Polygon(hDC, triangles[i].pt, 3); // 삼각형 3개 꼭짓점을 이용해 다각형 그림

            SelectObject(hDC, oldBrush);      // 원래 브러시 복원
            DeleteObject(hBrush);             // 새로 만든 브러시 삭제 (메모리 누수 방지)

            DrawCenteredNumber(hDC, GetTriangleRect(triangles[i]), triangles[i].number); // 중앙에 숫자 출력
        }

        // -------------------------
        // 사각형 그리기 부분
        // -------------------------
        for (i = 0; i < rectCount; i++)
        {
            HBRUSH hBrush = CreateSolidBrush(rects[i].color);     // 사각형 색 브러시 생성
            HBRUSH oldBrush = (HBRUSH)SelectObject(hDC, hBrush);  // 브러시 선택

            Rectangle(hDC,
                rects[i].rc.left,      // 왼쪽
                rects[i].rc.top,       // 위쪽
                rects[i].rc.right,     // 오른쪽
                rects[i].rc.bottom);   // 아래쪽

            SelectObject(hDC, oldBrush);  // 원래 브러시 복원
            DeleteObject(hBrush);         // 생성한 브러시 삭제

            DrawCenteredNumber(hDC, rects[i].rc, rects[i].number); // 사각형 중앙에 숫자 출력
        }

        // -------------------------
        // 원 그리기 부분
        // -------------------------
        for (i = 0; i < circleCount; i++)
        {
            HBRUSH hBrush = CreateSolidBrush(circles[i].color);   // 원 색 브러시 생성
            HBRUSH oldBrush = (HBRUSH)SelectObject(hDC, hBrush);  // 브러시 선택

            Ellipse(hDC,
                circles[i].rc.left,     // 원을 감싸는 사각형의 왼쪽
                circles[i].rc.top,      // 위쪽
                circles[i].rc.right,    // 오른쪽
                circles[i].rc.bottom);  // 아래쪽

            SelectObject(hDC, oldBrush); // 원래 브러시 복원
            DeleteObject(hBrush);        // 생성한 브러시 삭제

            DrawCenteredNumber(hDC, circles[i].rc, circles[i].number); // 원 중앙에 숫자 출력
        }

        // 원래 폰트 복원
        SelectObject(hDC, oldFont);

        // 생성한 폰트 삭제
        DeleteObject(hFont);

        // 그리기 종료
        EndPaint(hWnd, &ps);
    }
    break;

    case WM_DESTROY:
        // 창이 닫힐 때 호출됨

        // 메시지 루프를 종료시키는 종료 메시지 보냄
        PostQuitMessage(0);

        return 0;
    }

    // 위에서 처리하지 않은 메시지는 기본 윈도우 처리 함수에 맡김
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}