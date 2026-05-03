#include <windows.h>   // 윈도우 API 사용 헤더
#include <tchar.h>     // 유니코드 문자열 관련 헤더
#include <stdlib.h>    // rand, srand 사용 헤더
#include <time.h>      // time 사용 헤더
#include <math.h>      // fabs 사용 헤더

// 프로그램 인스턴스 핸들을 저장하는 전역 변수
HINSTANCE g_hInst;

// 윈도우 클래스 이름
LPCTSTR lpszClass = L"My Window Class";

// 창 제목
LPCTSTR lpszWindowName = L"Window Programming Lab - Practice 2-12";

// 윈도우 메시지 처리 함수 선언
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

// 보드 가로 칸 수
#define BOARD_COLS 40

// 보드 세로 칸 수
#define BOARD_ROWS 40

// 한 칸의 픽셀 크기
#define CELL_SIZE 15

// 보드 시작 X 좌표
#define BOARD_LEFT 60

// 보드 시작 Y 좌표
#define BOARD_TOP 60

// 목표 위치 X 좌표
#define GOAL_X (BOARD_COLS / 2)

// 목표 위치 Y 좌표
#define GOAL_Y (BOARD_ROWS - 3)

// 특수 칸 총 개수
#define SPECIAL_COUNT 25

// 최대 크기 제한
#define MAX_SCALE 2.5

// 최소 크기 제한
#define MIN_SCALE 0.3

// 돌 모양 종류
enum SHAPE_TYPE
{
    SHAPE_TRIANGLE = 0,   // 세모
    SHAPE_RECT,           // 네모
    SHAPE_CIRCLE,         // 원
    SHAPE_ELLIPSE         // 타원
};

// 특수 칸 종류
enum CELL_TYPE
{
    CELL_EMPTY = 0,       // 빈 칸
    CELL_OBSTACLE,        // 장애물 칸
    CELL_COLOR,           // 색상 변경 칸
    CELL_SHRINK1,         // 10% 축소
    CELL_SHRINK2,         // 30% 축소
    CELL_SHRINK3,         // 50% 축소
    CELL_GROW1,           // 10% 확대
    CELL_GROW2,           // 30% 확대
    CELL_GROW3,           // 50% 확대
    CELL_SHAPE            // 모양 변경 칸
};

// 플레이어 정보를 저장하는 구조체
typedef struct PLAYER_TAG
{
    int x;                // 현재 x 칸 좌표
    int y;                // 현재 y 칸 좌표
    COLORREF color;       // 현재 색상
    double scale;         // 현재 크기 배율
    int shape;            // 현재 모양
} PLAYER;

// 목표 칸 정보를 저장하는 구조체
typedef struct GOAL_TAG
{
    int x;                // 목표 x 칸 좌표
    int y;                // 목표 y 칸 좌표
    COLORREF color;       // 목표 색상
    double scale;         // 목표 크기
    int shape;            // 목표 모양
} GOAL_INFO;

// 특수 칸 정보를 저장하는 구조체
typedef struct SPECIAL_CELL_TAG
{
    int x;                // 특수 칸 x 좌표
    int y;                // 특수 칸 y 좌표
    int type;             // 특수 칸 종류
    COLORREF drawColor;   // 화면에 보일 칸 색상
} SPECIAL_CELL;

// 플레이어 2명 정보 저장
PLAYER g_player[2];

// 목표 칸 정보 저장
GOAL_INFO g_goal;

// 특수 칸 정보 저장
SPECIAL_CELL g_special[SPECIAL_COUNT];

// 현재 턴 저장, 0이면 플레이어1, 1이면 플레이어2
int g_currentTurn = 0;

// 게임 종료 여부 저장, 0이면 진행 중, 1이면 종료
int g_gameOver = 0;

// 자유 이동 모드 저장, 0이면 턴제, 1이면 두 플레이어 모두 자유 이동 가능
int g_freeMoveMode = 0;

// 마지막 승리한 플레이어 번호 저장
int g_lastWinner = 0;

// 실제 저장된 특수 칸 개수
int g_specialCount = 0;

// 색상 후보 배열
COLORREF g_colorPool[8] =
{
    RGB(255, 0, 0),       // 빨강
    RGB(0, 0, 255),       // 파랑
    RGB(0, 180, 0),       // 초록
    RGB(255, 140, 0),     // 주황
    RGB(160, 0, 200),     // 보라
    RGB(0, 180, 180),     // 청록
    RGB(255, 105, 180),   // 분홍
    RGB(120, 80, 40)      // 갈색
};

// 함수 원형 선언
int RandomInt(int min, int max);
int FindSpecialCellIndex(int x, int y);
int IsObstacleCell(int x, int y);
int IsReservedCell(int x, int y);
int IsDuplicateSpecialCell(int count, int x, int y);
void InitPlayers(void);
void InitGoal(void);
void AddSpecialCells(int startIndex, int count, int type, COLORREF drawColor, int randomColorMode);
void InitSpecialCells(void);
void InitGame(void);
void ApplyRandomColor(int playerIndex);
void ApplyScale(int playerIndex, double factor);
void ApplyRandomShape(int playerIndex);
void ApplyCellEffect(int playerIndex);
int IsGoalMatched(int playerIndex);
void MovePlayer(HWND hWnd, int playerIndex, int dx, int dy, int changeTurn);
void DrawBoardGrid(HDC hdc);
void DrawSpecialCells(HDC hdc);
void DrawGoal(HDC hdc);
void DrawStartMarks(HDC hdc);
void DrawStone(HDC hdc, int cellX, int cellY, int shape, COLORREF color, double scale, int highlight);
void DrawPlayers(HDC hdc);
void DrawTurnText(HDC hdc);

// 윈도우 프로그램 시작 함수
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
    HWND hWnd;                         // 메인 윈도우 핸들
    MSG Message;                       // 메시지 저장 구조체
    WNDCLASSEX WndClass;               // 윈도우 클래스 구조체

    g_hInst = hInstance;               // 인스턴스 핸들 저장

    WndClass.cbSize = sizeof(WndClass);                            // 구조체 크기 설정
    WndClass.style = CS_HREDRAW | CS_VREDRAW;                     // 창 크기 변경 시 다시 그림
    WndClass.lpfnWndProc = (WNDPROC)WndProc;                      // 메시지 처리 함수 등록
    WndClass.cbClsExtra = 0;                                      // 클래스 추가 메모리 없음
    WndClass.cbWndExtra = 0;                                      // 윈도우 추가 메모리 없음
    WndClass.hInstance = hInstance;                               // 인스턴스 핸들 설정
    WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);             // 기본 아이콘 설정
    WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);               // 기본 커서 설정
    WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); // 배경색 흰색 설정
    WndClass.lpszMenuName = NULL;                                 // 메뉴 없음
    WndClass.lpszClassName = lpszClass;                           // 클래스 이름 설정
    WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);           // 작은 아이콘 설정

    RegisterClassEx(&WndClass);                                   // 윈도우 클래스 등록

    hWnd = CreateWindow(
        lpszClass,                                                // 클래스 이름
        lpszWindowName,                                           // 창 제목
        WS_OVERLAPPEDWINDOW,                                      // 일반 윈도우 스타일
        100,                                                      // 창 시작 x 위치
        50,                                                       // 창 시작 y 위치
        760,                                                      // 창 가로 크기
        800,                                                      // 창 세로 크기
        NULL,                                                     // 부모 창 없음
        (HMENU)NULL,                                              // 메뉴 없음
        hInstance,                                                // 인스턴스 전달
        NULL                                                      // 추가 데이터 없음
    );

    ShowWindow(hWnd, nCmdShow);                                   // 창 표시
    UpdateWindow(hWnd);                                           // 창 갱신

    while (GetMessage(&Message, 0, 0, 0))                         // 메시지 루프
    {
        TranslateMessage(&Message);                               // 키보드 메시지 변환
        DispatchMessage(&Message);                                // 메시지 전달
    }

    return (int)Message.wParam;                                   // 종료 코드 반환
}

// min 이상 max 이하의 랜덤 정수 반환
int RandomInt(int min, int max)
{
    return min + rand() % (max - min + 1);                        // 지정 범위 안 랜덤값 반환
}

// 특정 좌표의 특수 칸 인덱스를 찾는 함수
int FindSpecialCellIndex(int x, int y)
{
    int i;                                                        // 반복문 변수

    for (i = 0; i < g_specialCount; i++)                          // 실제 저장된 특수 칸 개수만큼 검사
    {
        if (g_special[i].x == x && g_special[i].y == y)           // 좌표가 같으면
        {
            return i;                                             // 인덱스 반환
        }
    }

    return -1;                                                    // 못 찾으면 -1 반환
}

// 특정 좌표가 장애물 칸인지 검사
int IsObstacleCell(int x, int y)
{
    int idx;                                                      // 특수 칸 인덱스 저장 변수

    idx = FindSpecialCellIndex(x, y);                             // 특수 칸 위치 찾기

    if (idx == -1)                                                // 특수 칸이 아니면
    {
        return 0;                                                 // 장애물 아님
    }

    if (g_special[idx].type == CELL_OBSTACLE)                     // 장애물 칸이면
    {
        return 1;                                                 // 장애물 맞음
    }

    return 0;                                                     // 아니면 장애물 아님
}

// 시작 위치나 목표 위치와 겹치는지 검사
int IsReservedCell(int x, int y)
{
    if (x == 0 && y == 0)                                         // 플레이어1 시작 위치면
    {
        return 1;                                                 // 예약된 칸
    }

    if (x == BOARD_COLS - 1 && y == 0)                            // 플레이어2 시작 위치면
    {
        return 1;                                                 // 예약된 칸
    }

    if (x == GOAL_X && y == GOAL_Y)                               // 목표 위치면
    {
        return 1;                                                 // 예약된 칸
    }

    return 0;                                                     // 예약된 칸이 아니면 0
}

// 이미 배치된 특수 칸과 겹치는지 검사
int IsDuplicateSpecialCell(int count, int x, int y)
{
    int i;                                                        // 반복문 변수

    for (i = 0; i < count; i++)                                   // 지금까지 저장한 특수 칸 개수만큼 검사
    {
        if (g_special[i].x == x && g_special[i].y == y)           // 좌표가 같으면
        {
            return 1;                                             // 중복
        }
    }

    return 0;                                                     // 중복 아님
}

// 플레이어 초기화
void InitPlayers(void)
{
    int i;                                                        // 반복문 변수

    g_player[0].x = 0;                                            // 플레이어1 시작 x
    g_player[0].y = 0;                                            // 플레이어1 시작 y
    g_player[1].x = BOARD_COLS - 1;                               // 플레이어2 시작 x
    g_player[1].y = 0;                                            // 플레이어2 시작 y

    for (i = 0; i < 2; i++)                                       // 두 플레이어 모두 초기화
    {
        g_player[i].color = g_colorPool[RandomInt(0, 7)];         // 랜덤 색상 설정
        g_player[i].scale = 0.9 + (RandomInt(0, 6) * 0.1);        // 랜덤 크기 설정
        g_player[i].shape = RandomInt(0, 3);                      // 랜덤 모양 설정
    }
}

// 목표 칸 초기화
void InitGoal(void)
{
    g_goal.x = GOAL_X;                                            // 목표 x 설정
    g_goal.y = GOAL_Y;                                            // 목표 y 설정
    g_goal.color = g_colorPool[RandomInt(0, 7)];                  // 목표 색상 랜덤 설정
    g_goal.scale = 0.7 + (RandomInt(0, 8) * 0.1);                 // 목표 크기 랜덤 설정
    g_goal.shape = RandomInt(0, 3);                               // 목표 모양 랜덤 설정
}

// 특수 칸을 한 종류씩 여러 개 만드는 함수
void AddSpecialCells(int startIndex, int count, int type, COLORREF drawColor, int randomColorMode)
{
    int i;                                                        // 현재 몇 개를 만들었는지 세는 변수
    int x;                                                        // 랜덤 x 저장 변수
    int y;                                                        // 랜덤 y 저장 변수
    COLORREF colorValue;                                          // 실제 칠할 색 저장 변수

    for (i = 0; i < count; i++)                                   // 요청한 개수만큼 반복
    {
        while (1)                                                 // 겹치지 않는 위치가 나올 때까지 반복
        {
            x = RandomInt(0, BOARD_COLS - 1);                     // 랜덤 x 생성
            y = RandomInt(2, BOARD_ROWS - 5);                     // 랜덤 y 생성

            if (IsReservedCell(x, y))                             // 시작점 또는 목표점이면
            {
                continue;                                         // 다시 뽑기
            }

            if (IsDuplicateSpecialCell(startIndex + i, x, y))     // 기존 특수 칸과 겹치면
            {
                continue;                                         // 다시 뽑기
            }

            if (randomColorMode == 1)                             // 랜덤 색 사용 모드면
            {
                colorValue = g_colorPool[RandomInt(0, 7)];        // 색상 후보 중 하나 선택
            }
            else                                                  // 고정 색 사용 모드면
            {
                colorValue = drawColor;                           // 전달받은 색 그대로 사용
            }

            g_special[startIndex + i].x = x;                      // x 저장
            g_special[startIndex + i].y = y;                      // y 저장
            g_special[startIndex + i].type = type;                // 칸 종류 저장
            g_special[startIndex + i].drawColor = colorValue;     // 화면 표시 색 저장
            break;                                                // 현재 칸 생성 완료
        }
    }
}

// 특수 칸 전체 초기화
void InitSpecialCells(void)
{
    g_specialCount = 0;                                           // 특수 칸 개수 초기화

    AddSpecialCells(g_specialCount, 5, CELL_OBSTACLE, RGB(255, 0, 0), 0);   // 장애물 5개 생성
    g_specialCount += 5;                                          // 저장 개수 갱신

    AddSpecialCells(g_specialCount, 5, CELL_COLOR, RGB(0, 0, 0), 1);         // 색상 변경 칸 5개 생성
    g_specialCount += 5;                                          // 저장 개수 갱신

    AddSpecialCells(g_specialCount, 1, CELL_SHRINK1, RGB(180, 220, 255), 0); // 축소1 칸 1개
    g_specialCount += 1;                                          // 저장 개수 갱신
    AddSpecialCells(g_specialCount, 1, CELL_SHRINK2, RGB(150, 200, 255), 0); // 축소2 칸 1개
    g_specialCount += 1;                                          // 저장 개수 갱신
    AddSpecialCells(g_specialCount, 1, CELL_SHRINK3, RGB(120, 180, 255), 0); // 축소3 칸 1개
    g_specialCount += 1;                                          // 저장 개수 갱신

    AddSpecialCells(g_specialCount, 1, CELL_GROW1, RGB(255, 235, 150), 0);   // 확대1 칸 1개
    g_specialCount += 1;                                          // 저장 개수 갱신
    AddSpecialCells(g_specialCount, 1, CELL_GROW2, RGB(255, 215, 110), 0);   // 확대2 칸 1개
    g_specialCount += 1;                                          // 저장 개수 갱신
    AddSpecialCells(g_specialCount, 1, CELL_GROW3, RGB(255, 190, 70), 0);    // 확대3 칸 1개
    g_specialCount += 1;                                          // 저장 개수 갱신

    AddSpecialCells(g_specialCount, 9, CELL_SHAPE, RGB(210, 160, 255), 0);   // 모양 변경 칸 9개 생성
    g_specialCount += 9;                                          // 저장 개수 갱신
}

// 게임 전체 초기화
void InitGame(void)
{
    srand((unsigned int)time(NULL));                              // 난수 시드 초기화
    InitGoal();                                                   // 목표 설정
    InitPlayers();                                                // 플레이어 설정
    InitSpecialCells();                                           // 특수 칸 설정
    g_currentTurn = 0;                                            // 플레이어1부터 시작
    g_gameOver = 0;                                               // 게임 진행 상태
    g_freeMoveMode = 0;                                           // 자유 이동 모드 끔
    g_lastWinner = 0;                                             // 마지막 승자 없음
}

// 색상 랜덤 변경
void ApplyRandomColor(int playerIndex)
{
    g_player[playerIndex].color = g_colorPool[RandomInt(0, 7)];   // 해당 플레이어 색상 랜덤 변경
}

// 크기 변경
void ApplyScale(int playerIndex, double factor)
{
    g_player[playerIndex].scale = g_player[playerIndex].scale * factor; // 현재 크기에 배율 적용

    if (g_player[playerIndex].scale > MAX_SCALE)                  // 최대값보다 크면
    {
        g_player[playerIndex].scale = MAX_SCALE;                  // 최대값으로 제한
    }

    if (g_player[playerIndex].scale < MIN_SCALE)                  // 최소값보다 작으면
    {
        g_player[playerIndex].scale = MIN_SCALE;                  // 최소값으로 제한
    }
}

// 모양 랜덤 변경
void ApplyRandomShape(int playerIndex)
{
    int newShape;                                                 // 새 모양 저장 변수

    newShape = RandomInt(0, 3);                                   // 랜덤 모양 선택

    while (newShape == g_player[playerIndex].shape)               // 현재 모양과 같으면
    {
        newShape = RandomInt(0, 3);                               // 다시 선택
    }

    g_player[playerIndex].shape = newShape;                       // 플레이어 모양 변경
}

// 현재 칸 효과 적용
void ApplyCellEffect(int playerIndex)
{
    int idx;                                                      // 특수 칸 인덱스 저장 변수

    idx = FindSpecialCellIndex(g_player[playerIndex].x, g_player[playerIndex].y); // 현재 위치의 특수 칸 찾기

    if (idx == -1)                                                // 특수 칸이 아니면
    {
        return;                                                   // 종료
    }

    switch (g_special[idx].type)                                  // 칸 종류에 따라 처리
    {
    case CELL_COLOR:
        ApplyRandomColor(playerIndex);                            // 색상 랜덤 변경
        break;

    case CELL_SHRINK1:
        ApplyScale(playerIndex, 0.9);                             // 10% 축소
        break;

    case CELL_SHRINK2:
        ApplyScale(playerIndex, 0.7);                             // 30% 축소
        break;

    case CELL_SHRINK3:
        ApplyScale(playerIndex, 0.5);                             // 50% 축소
        break;

    case CELL_GROW1:
        ApplyScale(playerIndex, 1.1);                             // 10% 확대
        break;

    case CELL_GROW2:
        ApplyScale(playerIndex, 1.3);                             // 30% 확대
        break;

    case CELL_GROW3:
        ApplyScale(playerIndex, 1.5);                             // 50% 확대
        break;

    case CELL_SHAPE:
        ApplyRandomShape(playerIndex);                            // 모양 랜덤 변경
        break;
    }
}

// 플레이어가 목표 상태와 일치하는지 검사
int IsGoalMatched(int playerIndex)
{
    if (g_player[playerIndex].x != g_goal.x || g_player[playerIndex].y != g_goal.y) // 위치가 다르면
    {
        return 0;                                                 // 실패
    }

    if (g_player[playerIndex].shape != g_goal.shape)              // 모양이 다르면
    {
        return 0;                                                 // 실패
    }

    if (g_player[playerIndex].color != g_goal.color)              // 색이 다르면
    {
        return 0;                                                 // 실패
    }

    if (fabs(g_player[playerIndex].scale - g_goal.scale) > 0.05)  // 크기가 충분히 비슷하지 않으면
    {
        return 0;                                                 // 실패
    }

    return 1;                                                     // 전부 같으면 성공
}

// 플레이어 이동 처리
void MovePlayer(HWND hWnd, int playerIndex, int dx, int dy, int changeTurn)
{
    int nx;                                                       // 새 x 저장 변수
    int ny;                                                       // 새 y 저장 변수

    if (g_gameOver)                                               // 게임이 끝났으면
    {
        return;                                                   // 이동 금지
    }

    nx = g_player[playerIndex].x + dx;                            // 새 x 계산
    ny = g_player[playerIndex].y + dy;                            // 새 y 계산

    if (nx < 0)                                                   // 왼쪽 밖으로 나가면
    {
        nx = BOARD_COLS - 1;                                      // 오른쪽 끝으로 이동
    }
    else if (nx >= BOARD_COLS)                                    // 오른쪽 밖으로 나가면
    {
        nx = 0;                                                   // 왼쪽 끝으로 이동
    }

    if (ny < 0)                                                   // 위쪽 밖으로 나가면
    {
        ny = BOARD_ROWS - 1;                                      // 아래쪽 끝으로 이동
    }
    else if (ny >= BOARD_ROWS)                                    // 아래쪽 밖으로 나가면
    {
        ny = 0;                                                   // 위쪽 끝으로 이동
    }

    if (IsObstacleCell(nx, ny))                                   // 이동할 칸이 장애물이면
    {
        return;                                                   // 이동 취소
    }

    g_player[playerIndex].x = nx;                                 // 실제 x 이동
    g_player[playerIndex].y = ny;                                 // 실제 y 이동

    ApplyCellEffect(playerIndex);                                 // 이동한 칸의 효과 적용

    if (IsGoalMatched(playerIndex))                               // 목표 도착 및 조건 일치 검사
    {
        g_gameOver = 1;                                           // 게임 종료
        g_lastWinner = playerIndex + 1;                           // 승자 저장
        InvalidateRect(hWnd, NULL, TRUE);                         // 다시 그리기 요청
        return;                                                   // 함수 종료
    }

    if (changeTurn == 1)                                          // 턴을 바꿔야 하는 경우만
    {
        g_currentTurn = 1 - g_currentTurn;                        // 현재 턴 반전
    }

    InvalidateRect(hWnd, NULL, TRUE);                             // 다시 그리기 요청
}

// 보드 격자 그리기
void DrawBoardGrid(HDC hdc)
{
    int i;                                                        // 반복문 변수

    for (i = 0; i <= BOARD_COLS; i++)                             // 세로선 출력
    {
        MoveToEx(hdc, BOARD_LEFT + i * CELL_SIZE, BOARD_TOP, NULL); // 선 시작점 설정
        LineTo(hdc, BOARD_LEFT + i * CELL_SIZE, BOARD_TOP + BOARD_ROWS * CELL_SIZE); // 선 끝점까지 그림
    }

    for (i = 0; i <= BOARD_ROWS; i++)                             // 가로선 출력
    {
        MoveToEx(hdc, BOARD_LEFT, BOARD_TOP + i * CELL_SIZE, NULL); // 선 시작점 설정
        LineTo(hdc, BOARD_LEFT + BOARD_COLS * CELL_SIZE, BOARD_TOP + i * CELL_SIZE); // 선 끝점까지 그림
    }
}

// 특수 칸 그리기
void DrawSpecialCells(HDC hdc)
{
    int i;                                                        // 반복문 변수

    for (i = 0; i < g_specialCount; i++)                          // 실제 특수 칸 개수만큼 반복
    {
        RECT rc;                                                  // 칸 영역 저장
        HBRUSH hBrush;                                            // 칠하기용 브러시
        HBRUSH hOldBrush;                                         // 이전 브러시 저장
        TCHAR mark[8] = L"";                                      // 칸 내부 표시 문자 저장

        rc.left = BOARD_LEFT + g_special[i].x * CELL_SIZE + 1;    // 왼쪽 좌표 계산
        rc.top = BOARD_TOP + g_special[i].y * CELL_SIZE + 1;      // 위쪽 좌표 계산
        rc.right = rc.left + CELL_SIZE - 1;                       // 오른쪽 좌표 계산
        rc.bottom = rc.top + CELL_SIZE - 1;                       // 아래쪽 좌표 계산

        hBrush = CreateSolidBrush(g_special[i].drawColor);        // 칸 색 브러시 생성
        hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);            // 브러시 선택
        Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);     // 칸 사각형 그리기
        SelectObject(hdc, hOldBrush);                             // 이전 브러시 복구
        DeleteObject(hBrush);                                     // 브러시 삭제

        switch (g_special[i].type)                                // 칸 종류에 따라 문자 설정
        {
        case CELL_OBSTACLE: lstrcpy(mark, L"X"); break;           // 장애물
        case CELL_COLOR:    lstrcpy(mark, L"C"); break;           // 색상 변경
        case CELL_SHRINK1:  lstrcpy(mark, L"S1"); break;          // 축소1
        case CELL_SHRINK2:  lstrcpy(mark, L"S2"); break;          // 축소2
        case CELL_SHRINK3:  lstrcpy(mark, L"S3"); break;          // 축소3
        case CELL_GROW1:    lstrcpy(mark, L"G1"); break;          // 확대1
        case CELL_GROW2:    lstrcpy(mark, L"G2"); break;          // 확대2
        case CELL_GROW3:    lstrcpy(mark, L"G3"); break;          // 확대3
        case CELL_SHAPE:    lstrcpy(mark, L"M"); break;           // 모양 변경
        }

        SetBkMode(hdc, TRANSPARENT);                              // 글자 배경 투명 처리
        TextOut(hdc, rc.left + 1, rc.top + 1, mark, lstrlen(mark)); // 칸 안에 문자 출력
    }
}

// 목표 칸과 목표 돌 그리기
void DrawGoal(HDC hdc)
{
    RECT rc;                                                      // 목표 칸 영역 저장
    HBRUSH hBrush;                                                // 목표 칸 브러시
    HBRUSH hOldBrush;                                             // 이전 브러시 저장

    rc.left = BOARD_LEFT + g_goal.x * CELL_SIZE + 1;              // 왼쪽 좌표
    rc.top = BOARD_TOP + g_goal.y * CELL_SIZE + 1;                // 위쪽 좌표
    rc.right = rc.left + CELL_SIZE - 1;                           // 오른쪽 좌표
    rc.bottom = rc.top + CELL_SIZE - 1;                           // 아래쪽 좌표

    hBrush = CreateSolidBrush(RGB(255, 255, 180));                // 연노랑 브러시 생성
    hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);                // 브러시 선택
    Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);         // 목표 칸 그리기
    SelectObject(hdc, hOldBrush);                                 // 브러시 복구
    DeleteObject(hBrush);                                         // 브러시 삭제

    TextOut(hdc, rc.left - 4, rc.top - 16, L"GOAL", 4);           // GOAL 문자 출력

    DrawStone(hdc, g_goal.x, g_goal.y, g_goal.shape, g_goal.color, g_goal.scale, 1); // 목표 돌 그리기
}

// 시작 위치 표시
void DrawStartMarks(HDC hdc)
{
    TextOut(hdc, BOARD_LEFT + 2, BOARD_TOP - 18, L"P1", 2);       // 플레이어1 시작 위치 표시
    TextOut(hdc, BOARD_LEFT + (BOARD_COLS - 1) * CELL_SIZE - 10, BOARD_TOP - 18, L"P2", 2); // 플레이어2 시작 위치 표시
}

// 돌 하나 그리기
void DrawStone(HDC hdc, int cellX, int cellY, int shape, COLORREF color, double scale, int highlight)
{
    int cx;                                                       // 중심 x 좌표
    int cy;                                                       // 중심 y 좌표
    int halfW;                                                    // 반 너비
    int halfH;                                                    // 반 높이
    HPEN hPen;                                                    // 펜 객체
    HPEN hOldPen;                                                 // 이전 펜 저장
    HBRUSH hBrush;                                                // 브러시 객체
    HBRUSH hOldBrush;                                             // 이전 브러시 저장
    POINT pt[3];                                                  // 삼각형의 세 꼭짓점 저장

    cx = BOARD_LEFT + cellX * CELL_SIZE + CELL_SIZE / 2;          // 중심 x 계산
    cy = BOARD_TOP + cellY * CELL_SIZE + CELL_SIZE / 2;           // 중심 y 계산

    halfW = (int)((CELL_SIZE / 2 - 2) * scale);                   // 가로 반크기 계산
    halfH = (int)((CELL_SIZE / 2 - 2) * scale);                   // 세로 반크기 계산

    if (halfW < 3)                                                // 너무 작으면
    {
        halfW = 3;                                                // 최소값 보정
    }

    if (halfH < 3)                                                // 너무 작으면
    {
        halfH = 3;                                                // 최소값 보정
    }

    if (shape == SHAPE_ELLIPSE)                                   // 타원이면
    {
        halfW = (int)(halfW * 1.2);                               // 가로를 조금 길게
        halfH = (int)(halfH * 0.75);                              // 세로를 조금 짧게

        if (halfH < 3)                                            // 너무 작으면
        {
            halfH = 3;                                            // 최소값 보정
        }
    }

    hPen = CreatePen(PS_SOLID, highlight ? 2 : 1, RGB(0, 0, 0)); // 강조 여부에 따라 테두리 두께 설정
    hOldPen = (HPEN)SelectObject(hdc, hPen);                      // 펜 선택
    hBrush = CreateSolidBrush(color);                             // 내부 색 브러시 생성
    hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);                // 브러시 선택

    switch (shape)                                                // 모양에 따라 실제 도형 출력
    {
    case SHAPE_TRIANGLE:
        pt[0].x = cx; pt[0].y = cy - halfH;                       // 위 꼭짓점
        pt[1].x = cx - halfW; pt[1].y = cy + halfH;               // 왼쪽 아래 꼭짓점
        pt[2].x = cx + halfW; pt[2].y = cy + halfH;               // 오른쪽 아래 꼭짓점
        Polygon(hdc, pt, 3);                                      // 삼각형 그리기
        break;

    case SHAPE_RECT:
        Rectangle(hdc, cx - halfW, cy - halfH, cx + halfW, cy + halfH); // 사각형 그리기
        break;

    case SHAPE_CIRCLE:
        Ellipse(hdc, cx - halfW, cy - halfW, cx + halfW, cy + halfW);   // 원 그리기
        break;

    case SHAPE_ELLIPSE:
        Ellipse(hdc, cx - halfW, cy - halfH, cx + halfW, cy + halfH);   // 타원 그리기
        break;
    }

    SelectObject(hdc, hOldPen);                                   // 이전 펜 복구
    SelectObject(hdc, hOldBrush);                                 // 이전 브러시 복구
    DeleteObject(hPen);                                           // 펜 삭제
    DeleteObject(hBrush);                                         // 브러시 삭제
}

// 플레이어 두 명 출력
void DrawPlayers(HDC hdc)
{
    DrawStone(hdc, g_player[0].x, g_player[0].y, g_player[0].shape, g_player[0].color, g_player[0].scale, g_currentTurn == 0 || g_freeMoveMode == 1); // 플레이어1 출력
    DrawStone(hdc, g_player[1].x, g_player[1].y, g_player[1].shape, g_player[1].color, g_player[1].scale, g_currentTurn == 1 || g_freeMoveMode == 1); // 플레이어2 출력

    TextOut(hdc, BOARD_LEFT + g_player[0].x * CELL_SIZE + 2, BOARD_TOP + g_player[0].y * CELL_SIZE + 1, L"1", 1); // 숫자 1 출력
    TextOut(hdc, BOARD_LEFT + g_player[1].x * CELL_SIZE + 2, BOARD_TOP + g_player[1].y * CELL_SIZE + 1, L"2", 1); // 숫자 2 출력
}

// 현재 턴 또는 승리 문구 출력
void DrawTurnText(HDC hdc)
{
    if (g_gameOver == 1)                                          // 게임이 끝났으면
    {
        if (g_lastWinner == 1)                                    // 플레이어1 승리면
        {
            TextOut(hdc, BOARD_LEFT, BOARD_TOP + BOARD_ROWS * CELL_SIZE + 20, L"Player 1 Win!", 13); // 승리 문구 출력
        }
        else if (g_lastWinner == 2)                               // 플레이어2 승리면
        {
            TextOut(hdc, BOARD_LEFT, BOARD_TOP + BOARD_ROWS * CELL_SIZE + 20, L"Player 2 Win!", 13); // 승리 문구 출력
        }
    }
    else                                                          // 게임 진행 중이면
    {
        if (g_freeMoveMode == 1)                                  // 자유 이동 모드이면
        {
            TextOut(hdc, BOARD_LEFT, BOARD_TOP + BOARD_ROWS * CELL_SIZE + 20, L"Free Move Mode : ON   (V to OFF)", 31); // 자유 이동 모드 안내
        }
        else                                                      // 일반 턴제 모드이면
        {
            if (g_currentTurn == 0)                               // 플레이어1 차례
            {
                TextOut(hdc, BOARD_LEFT, BOARD_TOP + BOARD_ROWS * CELL_SIZE + 20, L"Turn : Player 1   (V to Free Move)", 35); // 현재 턴 표시
            }
            else                                                  // 플레이어2 차례
            {
                TextOut(hdc, BOARD_LEFT, BOARD_TOP + BOARD_ROWS * CELL_SIZE + 20, L"Turn : Player 2   (V to Free Move)", 35); // 현재 턴 표시
            }
        }
    }
}

// 윈도우 메시지 처리 함수
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;                                               // 그리기용 구조체
    HDC hDC;                                                      // 디바이스 컨텍스트 핸들

    switch (uMsg)
    {
    case WM_CREATE:                                               // 창이 생성될 때
        InitGame();                                               // 게임 전체 초기화
        break;

    case WM_KEYDOWN:                                              // 키가 눌렸을 때
        if (wParam == 'Q' || wParam == 'q')                       // q를 누르면
        {
            DestroyWindow(hWnd);                                  // 프로그램 종료
            return 0;                                             // 메시지 처리 완료
        }

        if (wParam == 'R' || wParam == 'r')                       // r을 누르면
        {
            InitGame();                                           // 새 게임 시작
            InvalidateRect(hWnd, NULL, TRUE);                     // 다시 그리기 요청
            return 0;                                             // 메시지 처리 완료
        }

        if (wParam == 'V' || wParam == 'v')                       // v를 누르면
        {
            if (g_freeMoveMode == 0)                              // 현재 일반 턴제 모드면
            {
                g_freeMoveMode = 1;                               // 자유 이동 모드 켜기
            }
            else                                                  // 현재 자유 이동 모드면
            {
                g_freeMoveMode = 0;                               // 자유 이동 모드 끄기
            }

            InvalidateRect(hWnd, NULL, TRUE);                     // 화면 다시 그리기
            return 0;                                             // 메시지 처리 완료
        }

        if (g_gameOver == 0)                                      // 게임 진행 중일 때만 이동 처리
        {
            if (g_freeMoveMode == 0)                              // 일반 턴제 모드일 때
            {
                if (g_currentTurn == 0)                           // 플레이어1 차례면
                {
                    if (wParam == 'W' || wParam == 'w')           // W 입력이면 위로 이동
                    {
                        MovePlayer(hWnd, 0, 0, -1, 1);            // 플레이어1 이동 후 턴 변경
                        return 0;
                    }
                    else if (wParam == 'S' || wParam == 's')      // S 입력이면 아래로 이동
                    {
                        MovePlayer(hWnd, 0, 0, 1, 1);             // 플레이어1 이동 후 턴 변경
                        return 0;
                    }
                    else if (wParam == 'A' || wParam == 'a')      // A 입력이면 왼쪽 이동
                    {
                        MovePlayer(hWnd, 0, -1, 0, 1);            // 플레이어1 이동 후 턴 변경
                        return 0;
                    }
                    else if (wParam == 'D' || wParam == 'd')      // D 입력이면 오른쪽 이동
                    {
                        MovePlayer(hWnd, 0, 1, 0, 1);             // 플레이어1 이동 후 턴 변경
                        return 0;
                    }
                }
                else                                              // 플레이어2 차례면
                {
                    if (wParam == 'I' || wParam == 'i')           // I 입력이면 위로 이동
                    {
                        MovePlayer(hWnd, 1, 0, -1, 1);            // 플레이어2 이동 후 턴 변경
                        return 0;
                    }
                    else if (wParam == 'K' || wParam == 'k')      // K 입력이면 아래로 이동
                    {
                        MovePlayer(hWnd, 1, 0, 1, 1);             // 플레이어2 이동 후 턴 변경
                        return 0;
                    }
                    else if (wParam == 'J' || wParam == 'j')      // J 입력이면 왼쪽 이동
                    {
                        MovePlayer(hWnd, 1, -1, 0, 1);            // 플레이어2 이동 후 턴 변경
                        return 0;
                    }
                    else if (wParam == 'L' || wParam == 'l')      // L 입력이면 오른쪽 이동
                    {
                        MovePlayer(hWnd, 1, 1, 0, 1);             // 플레이어2 이동 후 턴 변경
                        return 0;
                    }
                }
            }
            else                                                  // 자유 이동 모드일 때
            {
                if (wParam == 'W' || wParam == 'w')               // 플레이어1 위 이동
                {
                    MovePlayer(hWnd, 0, 0, -1, 0);                // 턴 변경 없이 이동
                    return 0;
                }
                else if (wParam == 'S' || wParam == 's')          // 플레이어1 아래 이동
                {
                    MovePlayer(hWnd, 0, 0, 1, 0);                 // 턴 변경 없이 이동
                    return 0;
                }
                else if (wParam == 'A' || wParam == 'a')          // 플레이어1 왼쪽 이동
                {
                    MovePlayer(hWnd, 0, -1, 0, 0);                // 턴 변경 없이 이동
                    return 0;
                }
                else if (wParam == 'D' || wParam == 'd')          // 플레이어1 오른쪽 이동
                {
                    MovePlayer(hWnd, 0, 1, 0, 0);                 // 턴 변경 없이 이동
                    return 0;
                }
                else if (wParam == 'I' || wParam == 'i')          // 플레이어2 위 이동
                {
                    MovePlayer(hWnd, 1, 0, -1, 0);                // 턴 변경 없이 이동
                    return 0;
                }
                else if (wParam == 'K' || wParam == 'k')          // 플레이어2 아래 이동
                {
                    MovePlayer(hWnd, 1, 0, 1, 0);                 // 턴 변경 없이 이동
                    return 0;
                }
                else if (wParam == 'J' || wParam == 'j')          // 플레이어2 왼쪽 이동
                {
                    MovePlayer(hWnd, 1, -1, 0, 0);                // 턴 변경 없이 이동
                    return 0;
                }
                else if (wParam == 'L' || wParam == 'l')          // 플레이어2 오른쪽 이동
                {
                    MovePlayer(hWnd, 1, 1, 0, 0);                 // 턴 변경 없이 이동
                    return 0;
                }
            }
        }
        break;

    case WM_PAINT:                                                // 화면을 다시 그릴 때
        hDC = BeginPaint(hWnd, &ps);                              // 그리기 시작

        SetBkMode(hDC, TRANSPARENT);                              // 글자 배경 투명 처리

        DrawBoardGrid(hDC);                                       // 보드 그리기
        DrawSpecialCells(hDC);                                    // 특수 칸 그리기
        DrawGoal(hDC);                                            // 목표 칸 그리기
        DrawStartMarks(hDC);                                      // 시작 위치 표시
        DrawPlayers(hDC);                                         // 플레이어 두 명 그리기
        DrawTurnText(hDC);                                        // 상태 문구 출력

        EndPaint(hWnd, &ps);                                      // 그리기 종료
        return 0;                                                 // 메시지 처리 완료

    case WM_DESTROY:                                              // 창이 닫힐 때
        PostQuitMessage(0);                                       // 종료 메시지 전달
        return 0;                                                 // 메시지 처리 완료
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);             // 기본 메시지 처리
}