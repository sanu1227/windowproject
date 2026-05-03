#include <windows.h>                        // 윈도우 API 사용을 위한 헤더
#include <tchar.h>                          // TCHAR, LPCTSTR 사용을 위한 헤더
#include <stdlib.h>                         // rand, srand 사용
#include <time.h>                           // time 함수 사용
#include <math.h>                           // abs 사용

HINSTANCE g_hInst;                          // 프로그램 인스턴스 핸들
LPCTSTR lpszClass = L"My Window Class";     // 윈도우 클래스 이름
LPCTSTR lpszWindowName = L"Window Programming Lab"; // 윈도우 제목
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam); // 메시지 처리 함수 선언

#define BOARD_COLS 40                       // 보드 열 개수
#define BOARD_ROWS 40                       // 보드 행 개수
#define CELL_SIZE 15                        // 한 칸의 픽셀 크기
#define BOARD_LEFT 40                       // 보드 시작 x 좌표
#define BOARD_TOP 40                        // 보드 시작 y 좌표
#define BOARD_WIDTH (BOARD_COLS * CELL_SIZE) // 보드 전체 너비
#define BOARD_HEIGHT (BOARD_ROWS * CELL_SIZE) // 보드 전체 높이

#define MAX_FOOD 20                         // 먹이 최대 개수
#define MAX_TAIL 120                        // 꼬리 최대 개수
#define MAX_OBS 20                          // 장애물 최대 개수
#define HERO_BASE_RADIUS 10                 // 주인공 반지름
#define TIMER_ID 1                          // 타이머 ID
#define DEFAULT_SPEED 120                   // 자유꼬리 기본 속도
#define BASE_TIMER_INTERVAL 20              // 실제 타이머가 도는 공통 간격

#define DIR_LEFT 0                          // 왼쪽 방향
#define DIR_RIGHT 1                         // 오른쪽 방향
#define DIR_UP 2                            // 위 방향
#define DIR_DOWN 3                          // 아래 방향

#define SHAPE_CIRCLE 0                      // 원 모양
#define SHAPE_TRIANGLE 1                    // 삼각형 모양

#define MOVE_HORZ 1                         // 좌우 이동
#define MOVE_VERT 2                         // 상하 이동
#define MOVE_RECT 3                         // 사각형 경로 이동
#define MOVE_STOP 4                         // 정지
#define MOVE_PULSE 5                        // 제자리 크기 변화

typedef struct
{
    int row;                                // 먹이 행 위치
    int col;                                // 먹이 열 위치
    COLORREF color;                         // 먹이 색상
    int shape;                              // 먹이 모양
    int alive;                              // 존재 여부
} FOOD;

typedef struct
{
    int row;                                // 장애물 행 위치
    int col;                                // 장애물 열 위치
    int alive;                              // 존재 여부
} OBSTACLE;

typedef struct
{
    int row;                                // 꼬리 행 위치
    int col;                                // 꼬리 열 위치
    COLORREF color;                         // 꼬리 색상
    int baseRadius;                         // 기본 반지름
    int drawRadius;                         // 실제 그릴 반지름
    int attached;                           // 주인공에 붙어있는지 여부
    int alive;                              // 존재 여부
    int moveType;                           // 이동 방식
    int dir;                                // 이동 방향
    int rectStep;                           // 사각형 이동 단계
    int rectClockwise;                      // 사각형 경로 방향
    int pulseDir;                           // 크기 변화 방향

    int freePrev;                           // 자유꼬리 체인에서 앞 꼬리 인덱스(-1이면 체인 맨 앞)
    int freeNext;                           // 자유꼬리 체인에서 뒤 꼬리 인덱스(-1이면 체인 맨 뒤)

    int attachedPrev;                       // 붙은 꼬리 체인에서 앞 꼬리 인덱스(-1이면 첫 꼬리)
    int attachedNext;                       // 붙은 꼬리 체인에서 뒤 꼬리 인덱스(-1이면 마지막 꼬리)

    int label;                              // 꼬리 고유 번호
} TAIL;

typedef struct
{
    int row;                                // 주인공 행 위치
    int col;                                // 주인공 열 위치
    COLORREF color;                         // 주인공 색상
    int radius;                             // 주인공 반지름
    int shape;                              // 주인공 모양
    int dir;                                // 이동 방향
} HERO;

RECT g_clientRect;                          // 클라이언트 영역
FOOD g_foods[MAX_FOOD];                     // 먹이 배열
TAIL g_tails[MAX_TAIL];                     // 꼬리 배열
OBSTACLE g_obs[MAX_OBS];                    // 장애물 배열
HERO g_hero;                                // 주인공 정보

int g_started = 0;                          // 게임 시작 여부
int g_speed = DEFAULT_SPEED;                // 주인공 현재 속도
int g_attachedCount = 0;                    // 붙은 꼬리 수
int g_obstacleCount = 0;                    // 장애물 수
int g_aMode = 0;                            // a 모드 여부
int g_aStartRow = 0;                        // a 모드 시작 행
int g_aStartCol = 0;                        // a 모드 시작 열
int g_jumpPending = 0;                      // 점프 예약 여부

int g_attachedHead = -1;                    // 붙은 꼬리 체인의 첫 꼬리 인덱스
int g_attachedTail = -1;                    // 붙은 꼬리 체인의 마지막 꼬리 인덱스

int g_heroTick = 0;                         // 주인공 이동 누적 시간
int g_tailTick = 0;                         // 자유꼬리 이동 누적 시간

int g_jumpReturnPending = 0;                // 점프 후 원래 줄로 복귀해야 하는지 여부
int g_jumpReturnRow = -1;                   // 점프 전 원래 행 저장
int g_jumpReturnCol = -1;                   // 점프 전 원래 열 저장

int g_rotateEffect = 0;                     // t 리더교체 후 주인공 강조 효과 남은 횟수
int g_heroLabel = 0;                        // 현재 주인공이 가진 번호
int g_nextLabel = 1;                        // 새로 생성되는 자유꼬리에 붙일 다음 번호

int RandInt(int min, int max);              // 랜덤 정수 함수 선언
COLORREF RandColor();                       // 랜덤 색 함수 선언
int IsInsideBoardRC(int row, int col);      // 보드 내부 검사
int IsObstacleAt(int row, int col);         // 장애물 위치 검사
int IsFoodAt(int row, int col);             // 먹이 위치 검사
int IsFreeTailAt(int row, int col);         // 자유 꼬리 위치 검사
void RCToXY(int row, int col, int* x, int* y); // row,col -> x,y 변환
void InitGame();                            // 게임 초기화
void SpawnFoods();                          // 먹이 전체 생성
void SpawnOneFood(int idx);                 // 먹이 1개 생성
void SpawnTailFromFood(COLORREF color);     // 먹이 먹었을 때 꼬리 생성
int FindEmptyTailSlot();                    // 빈 꼬리 슬롯 찾기
void MoveGame(HWND hWnd);                   // 게임 전체 이동
void MoveHero();                            // 주인공 이동
void MoveFreeTails();                       // 자유 꼬리 이동
void MoveOneFreeTail(int i);                // 자유 꼬리 1개 이동
void MoveFreeTailChain(int leader);         // 자유꼬리 체인 한 줄 이동
void CheckFoodCollision();                  // 먹이 충돌 검사
void CheckHeroTailCollision();              // 주인공-자유꼬리 충돌 검사
void CheckTailTailCollision();              // 자유꼬리끼리 충돌 검사
void AttachTail(int idx);                   // 꼬리 붙이기
void AttachFreeChainToHero(int idx);        // 자유체인 전체를 주인공에게 붙이기
void RebuildAttachedAppearance();           // 붙은 꼬리 외형 재정리
void DetachTailChainFrom(int attachedIdx);  // 특정 붙은 꼬리부터 분리
void DetachAllTails();                      // 붙은 꼬리 전체 제거
void AddObstacleByMouse(int x, int y);      // 장애물 추가
void ChangeHeroDirectionByClick(int x, int y); // 클릭 방향으로 방향 전환
int GetBoardRowFromY(int y);                // y -> row 변환
int GetBoardColFromX(int x);                // x -> col 변환
int HitHero(int x, int y);                  // 주인공 클릭 판정
int HitAttachedTail(int x, int y);          // 붙은 꼬리 클릭 판정
void RotateLeader();                        // 리더 교체
void EnterAMode();                          // a 모드 시작
void ExitAMode();                           // a 모드 종료
void DrawScene(HDC hDC);                    // 전체 화면 그리기
void DrawBoard(HDC hDC);                    // 보드 그리기
void DrawFoods(HDC hDC);                    // 먹이 그리기
void DrawObstacles(HDC hDC);                // 장애물 그리기
void DrawHero(HDC hDC);                     // 주인공 그리기
void DrawTails(HDC hDC);                    // 꼬리 그리기
void DrawCircleCell(HDC hDC, int row, int col, int radius, COLORREF color); // 원 그리기
void DrawTriangleCell(HDC hDC, int row, int col, int radius, COLORREF color); // 삼각형 그리기
void DrawRectCell(HDC hDC, int row, int col, COLORREF color); // 사각형 먹이 그리기
void DrawTextInfo(HDC hDC);                 // 안내문 그리기
void DrawTailNumbers(HDC hDC);              // 주인공과 붙은 꼬리 번호 표시
void ReverseDirForObstacle(int* dir);       // 방향 반전
int GetFreeChainLeader(int idx);            // 자유꼬리 체인의 리더 찾기
int GetFreeChainTailEnd(int idx);           // 자유꼬리 체인의 맨 끝 찾기
void ConnectFreeTailChains(int a, int b);   // 두 자유꼬리 체인 연결
void ClearTailLinks(int idx);               // 꼬리 연결 정보 초기화
void ResetFreeTailState(int idx);           // 자유꼬리 상태 초기화
void RemoveFromAttachedChain(int idx);      // 붙은 체인에서 하나 제거
void AppendToAttachedChain(int idx);        // 붙은 체인 맨 뒤에 하나 추가

int RandInt(int min, int max)               // min~max 사이 랜덤 정수 반환
{
    return min + rand() % (max - min + 1);  // 범위 계산 후 반환
}

COLORREF RandColor()                        // 랜덤 색상 반환
{
    return RGB(RandInt(40, 255), RandInt(40, 255), RandInt(40, 255)); // 너무 어둡지 않은 색
}

int IsInsideBoardRC(int row, int col)       // 보드 안쪽인지 검사
{
    if (row < 0 || row >= BOARD_ROWS) return 0; // 행 범위 밖이면 0
    if (col < 0 || col >= BOARD_COLS) return 0; // 열 범위 밖이면 0
    return 1;                               // 정상 범위면 1
}

void RCToXY(int row, int col, int* x, int* y) // 보드 좌표를 실제 픽셀 중심 좌표로 변환
{
    *x = BOARD_LEFT + col * CELL_SIZE + CELL_SIZE / 2; // x 중심 계산
    *y = BOARD_TOP + row * CELL_SIZE + CELL_SIZE / 2;  // y 중심 계산
}

void ClearTailLinks(int idx)                // 꼬리의 자유체인/붙은체인 연결 정보 모두 초기화
{
    g_tails[idx].freePrev = -1;             // 자유체인 앞 연결 해제
    g_tails[idx].freeNext = -1;             // 자유체인 뒤 연결 해제
    g_tails[idx].attachedPrev = -1;         // 붙은체인 앞 연결 해제
    g_tails[idx].attachedNext = -1;         // 붙은체인 뒤 연결 해제
}

void ResetFreeTailState(int idx)            // 꼬리를 독립 자유꼬리 상태로 되돌리는 함수
{
    g_tails[idx].attached = 0;              // 주인공에게 안 붙어 있음
    g_tails[idx].moveType = RandInt(1, 5);  // 이동 방식 랜덤
    g_tails[idx].dir = RandInt(0, 3);       // 방향 랜덤
    g_tails[idx].rectStep = 0;              // 사각형 단계 초기화
    g_tails[idx].rectClockwise = RandInt(0, 1); // 회전 방향 랜덤
    g_tails[idx].pulseDir = 1;              // 크기 변화 방향 초기화
    ClearTailLinks(idx);                    // 모든 연결 해제
}

int IsObstacleAt(int row, int col)          // 해당 칸에 장애물이 있는지 검사
{
    int i;                                  // 반복 변수
    for (i = 0; i < MAX_OBS; i++)           // 장애물 전체 검사
    {
        if (g_obs[i].alive && g_obs[i].row == row && g_obs[i].col == col) return 1; // 있으면 1
    }
    return 0;                               // 없으면 0
}

int IsFoodAt(int row, int col)              // 해당 칸에 먹이가 있는지 검사
{
    int i;                                  // 반복 변수
    for (i = 0; i < MAX_FOOD; i++)          // 먹이 전체 검사
    {
        if (g_foods[i].alive && g_foods[i].row == row && g_foods[i].col == col) return i; // 있으면 인덱스 반환
    }
    return -1;                              // 없으면 -1
}

int IsFreeTailAt(int row, int col)          // 해당 칸에 자유 꼬리가 있는지 검사
{
    int i;                                  // 반복 변수
    for (i = 0; i < MAX_TAIL; i++)          // 꼬리 전체 검사
    {
        if (g_tails[i].alive && !g_tails[i].attached && g_tails[i].row == row && g_tails[i].col == col) return i; // 자유 꼬리면 인덱스 반환
    }
    return -1;                              // 없으면 -1
}

int FindEmptyTailSlot()                     // 비어 있는 꼬리 슬롯 찾기
{
    int i;                                  // 반복 변수
    for (i = 0; i < MAX_TAIL; i++)          // 전체 꼬리 슬롯 검사
    {
        if (!g_tails[i].alive) return i;    // 비어 있으면 그 인덱스 반환
    }
    return -1;                              // 없으면 -1
}

void SpawnOneFood(int idx)                  // 먹이 1개 랜덤 생성
{
    int row;                                // 후보 행
    int col;                                // 후보 열

    while (1)                               // 빈 칸을 찾을 때까지 반복
    {
        row = RandInt(0, BOARD_ROWS - 1);   // 랜덤 행
        col = RandInt(0, BOARD_COLS - 1);   // 랜덤 열

        if (row == g_hero.row && col == g_hero.col) continue; // 주인공 위치면 다시
        if (IsObstacleAt(row, col)) continue; // 장애물 위치면 다시
        if (IsFoodAt(row, col) != -1) continue; // 먹이 위치면 다시
        if (IsFreeTailAt(row, col) != -1) continue; // 자유 꼬리 위치면 다시

        g_foods[idx].row = row;             // 행 저장
        g_foods[idx].col = col;             // 열 저장
        g_foods[idx].color = RandColor();   // 색상 저장
        g_foods[idx].shape = RandInt(0, 1); // 모양 저장
        g_foods[idx].alive = 1;             // 활성화
        break;                              // 생성 완료
    }
}

void SpawnFoods()                           // 먹이 전체 생성
{
    int i;                                  // 반복 변수
    for (i = 0; i < MAX_FOOD; i++)          // 20개 생성
    {
        SpawnOneFood(i);                    // 한 개씩 생성
    }
}

void SpawnTailFromFood(COLORREF color)      // 먹이를 먹었을 때 자유 꼬리를 랜덤한 빈 칸에 생성
{
    int idx;                                // 새 꼬리 인덱스
    int newRow;                             // 새 랜덤 행
    int newCol;                             // 새 랜덤 열

    idx = FindEmptyTailSlot();              // 빈 슬롯 찾기
    if (idx == -1) return;                  // 빈 슬롯이 없으면 종료

    while (1)                               // 빈 랜덤 위치 찾기
    {
        newRow = RandInt(0, BOARD_ROWS - 1); // 랜덤 행 선택
        newCol = RandInt(0, BOARD_COLS - 1); // 랜덤 열 선택

        if (newRow == g_hero.row && newCol == g_hero.col) continue; // 주인공 위치 제외
        if (IsObstacleAt(newRow, newCol)) continue; // 장애물 위치 제외
        if (IsFoodAt(newRow, newCol) != -1) continue; // 먹이 위치 제외
        if (IsFreeTailAt(newRow, newCol) != -1) continue; // 자유 꼬리 위치 제외

        break;                              // 빈 칸 찾았으면 종료
    }

    g_tails[idx].row = newRow;              // 랜덤 행 저장
    g_tails[idx].col = newCol;              // 랜덤 열 저장
    g_tails[idx].color = color;             // 먹이 색과 같은 색 저장
    g_tails[idx].baseRadius = 15;           // 기본 반지름
    g_tails[idx].drawRadius = 15;           // 실제 반지름
    g_tails[idx].attached = 0;              // 자유 꼬리 상태
    g_tails[idx].alive = 1;                 // 활성화
    g_tails[idx].moveType = RandInt(1, 5);  // 이동 방식 랜덤
    g_tails[idx].dir = RandInt(0, 3);       // 방향 랜덤
    g_tails[idx].rectStep = 0;              // 사각형 단계 초기화
    g_tails[idx].rectClockwise = RandInt(0, 1); // 경로 방향 랜덤
    g_tails[idx].pulseDir = 1;              // 펄스 방향 초기화
    ClearTailLinks(idx);                    // 연결 정보 초기화
    g_tails[idx].label = g_nextLabel;       // 새 자유꼬리에 고유 번호 부여
    g_nextLabel++;                          // 다음 번호 증가
}

void InitGame()                             // 전체 게임 초기화
{
    int i;                                  // 반복 변수

    g_rotateEffect = 0;                     // 리더교체 강조 효과 초기화
    g_heroLabel = 0;                        // 시작 주인공 번호는 0
    g_nextLabel = 1;                        // 새 꼬리 번호는 1부터 시작

    g_jumpReturnPending = 0;                // 점프 복귀 상태 초기화
    g_jumpReturnRow = -1;                   // 복귀 행 초기화
    g_jumpReturnCol = -1;                   // 복귀 열 초기화

    g_started = 0;                          // 시작 전 상태
    g_speed = DEFAULT_SPEED;                // 주인공 기본 속도
    g_attachedCount = 0;                    // 붙은 꼬리 0개
    g_obstacleCount = 0;                    // 장애물 0개
    g_aMode = 0;                            // a 모드 꺼짐
    g_jumpPending = 0;                      // 점프 꺼짐
    g_attachedHead = -1;                    // 붙은 꼬리 시작 없음
    g_attachedTail = -1;                    // 붙은 꼬리 끝 없음
    g_heroTick = 0;                         // 주인공 이동 누적 시간 초기화
    g_tailTick = 0;                         // 자유꼬리 이동 누적 시간 초기화

    g_hero.row = RandInt(0, BOARD_ROWS - 1); // 주인공 시작 행
    g_hero.col = RandInt(0, BOARD_COLS - 1); // 주인공 시작 열
    g_hero.color = RGB(255, 210, 0);        // 주인공 시작 색
    g_hero.radius = HERO_BASE_RADIUS;       // 주인공 반지름
    g_hero.shape = SHAPE_CIRCLE;            // 주인공 시작 모양
    g_hero.dir = DIR_LEFT;                  // 시작 방향

    for (i = 0; i < MAX_TAIL; i++)          // 꼬리 배열 초기화
    {
        g_tails[i].alive = 0;               // 모두 비활성화
        g_tails[i].attached = 0;            // 안 붙은 상태
        ClearTailLinks(i);                  // 연결 정보 초기화
        g_tails[i].label = -1;              // 번호 없음으로 초기화
    }

    for (i = 0; i < MAX_OBS; i++)           // 장애물 배열 초기화
    {
        g_obs[i].alive = 0;                 // 모두 비활성화
    }

    SpawnFoods();                           // 먹이 생성
}

void RemoveFromAttachedChain(int idx)       // 붙은 체인에서 꼬리 하나를 떼는 함수
{
    int prevIdx;                            // 앞 꼬리 인덱스
    int nextIdx;                            // 뒤 꼬리 인덱스

    prevIdx = g_tails[idx].attachedPrev;    // 앞 연결 저장
    nextIdx = g_tails[idx].attachedNext;    // 뒤 연결 저장

    if (prevIdx != -1) g_tails[prevIdx].attachedNext = nextIdx; // 앞쪽 꼬리의 뒤 연결 복구
    else g_attachedHead = nextIdx;          // 맨 앞이 빠지면 헤드 이동

    if (nextIdx != -1) g_tails[nextIdx].attachedPrev = prevIdx; // 뒤쪽 꼬리의 앞 연결 복구
    else g_attachedTail = prevIdx;          // 맨 뒤가 빠지면 테일 이동

    g_tails[idx].attachedPrev = -1;         // 연결 해제
    g_tails[idx].attachedNext = -1;         // 연결 해제
}

void AppendToAttachedChain(int idx)         // 붙은 체인 맨 뒤에 꼬리 하나를 추가하는 함수
{
    g_tails[idx].attached = 1;              // 붙은 상태로 변경
    g_tails[idx].moveType = MOVE_STOP;      // 독립 이동 정지
    g_tails[idx].freePrev = -1;             // 자유체인 연결 해제
    g_tails[idx].freeNext = -1;             // 자유체인 연결 해제
    g_tails[idx].attachedPrev = g_attachedTail; // 현재 마지막 꼬리를 앞 꼬리로 지정
    g_tails[idx].attachedNext = -1;         // 뒤에는 아직 없음

    if (g_attachedTail != -1) g_tails[g_attachedTail].attachedNext = idx; // 기존 마지막 꼬리 뒤에 연결
    else g_attachedHead = idx;              // 빈 체인이면 첫 꼬리가 됨

    g_attachedTail = idx;                   // 새 마지막 꼬리로 설정
    g_attachedCount++;                      // 붙은 꼬리 수 증가
}

void RebuildAttachedAppearance()            // 붙은 꼬리 외형 재정리
{
    int idx;                                // 현재 순회할 꼬리 인덱스
    int r = 15;                             // 첫 꼬리 반지름

    idx = g_attachedHead;                   // 첫 꼬리부터 시작
    while (idx != -1)                       // 끝까지 순회
    {
        g_tails[idx].color = g_hero.color;  // 주인공 색으로 통일
        g_tails[idx].baseRadius = r;        // 반지름 저장
        g_tails[idx].drawRadius = r;        // 그릴 반지름 저장
        if (r > 5) r -= 2;                  // 뒤로 갈수록 작게
        idx = g_tails[idx].attachedNext;    // 다음 붙은 꼬리로 이동
    }
}

void AttachTail(int idx)                    // 자유 꼬리를 주인공 뒤에 붙이는 함수
{
    if (idx < 0 || idx >= MAX_TAIL) return; // 범위 검사
    if (!g_tails[idx].alive) return;        // 살아있지 않으면 종료
    if (g_tails[idx].attached) return;      // 이미 붙어 있으면 종료

    ClearTailLinks(idx);                    // 자유체인/붙은체인 연결 정리
    AppendToAttachedChain(idx);             // 붙은 체인 뒤에 추가
    RebuildAttachedAppearance();            // 외형 재정리
}

void DetachTailChainFrom(int attachedIdx)   // 클릭한 붙은 꼬리부터 뒤를 전부 분리
{
    int prevIdx;                            // 클릭한 꼬리의 앞 꼬리 인덱스
    int cur;                                // 현재 분리 중인 꼬리
    int nextIdx;                            // 다음 꼬리 저장용
    int detachedCount = 0;                  // 이번에 분리된 꼬리 수

    if (attachedIdx < 0 || attachedIdx >= MAX_TAIL) return; // 범위 검사
    if (!g_tails[attachedIdx].alive) return; // 죽은 꼬리면 종료
    if (!g_tails[attachedIdx].attached) return; // 붙은 꼬리 아니면 종료

    prevIdx = g_tails[attachedIdx].attachedPrev; // 클릭한 꼬리 앞쪽 기억

    if (prevIdx != -1) g_tails[prevIdx].attachedNext = -1; // 앞쪽 꼬리를 체인의 끝으로 만듦
    else g_attachedHead = -1;               // 맨 앞부터 분리되면 붙은 체인 비움

    g_attachedTail = prevIdx;               // 남은 체인의 마지막 꼬리 갱신

    cur = attachedIdx;                      // 클릭한 꼬리부터 시작
    while (cur != -1)                       // 뒤쪽 끝까지 순회
    {
        nextIdx = g_tails[cur].attachedNext; // 다음 꼬리 미리 저장
        ResetFreeTailState(cur);            // 자유꼬리 상태로 변경
        detachedCount++;                    // 분리 개수 증가
        cur = nextIdx;                      // 다음 꼬리로 이동
    }

    g_attachedCount -= detachedCount;       // 분리된 수만큼 감소
    if (g_attachedCount < 0) g_attachedCount = 0; // 안전 보정
    RebuildAttachedAppearance();            // 남은 붙은 꼬리 외형 다시 정리
}

void DetachAllTails()                       // 붙은 꼬리 전체 제거
{
    int cur;                                // 현재 순회할 꼬리 인덱스
    int nextIdx;                            // 다음 꼬리 인덱스

    cur = g_attachedHead;                   // 첫 붙은 꼬리부터 시작
    while (cur != -1)                       // 붙은 꼬리 전체 순회
    {
        nextIdx = g_tails[cur].attachedNext; // 다음 꼬리 저장
        g_tails[cur].alive = 0;             // 꼬리 제거
        g_tails[cur].attached = 0;          // 붙은 상태 해제
        g_tails[cur].label = -1;            // 번호 제거
        ClearTailLinks(cur);                // 연결 해제
        cur = nextIdx;                      // 다음 꼬리로 이동
    }

    g_attachedHead = -1;                    // 붙은 체인 비움
    g_attachedTail = -1;                    // 붙은 체인 비움
    g_attachedCount = 0;                    // 붙은 꼬리 수 0
}

void EnterAMode()                           // a 모드 시작
{
    int i;                                  // 반복 변수

    if (g_aMode) return;                    // 이미 켜져 있으면 종료

    g_aMode = 1;                            // a 모드 켜기
    g_aStartRow = g_hero.row;               // 시작 행 저장
    g_aStartCol = g_hero.col;               // 시작 열 저장

    for (i = 0; i < MAX_TAIL; i++)          // 모든 꼬리 순회
    {
        if (g_tails[i].alive && !g_tails[i].attached) // 자유 꼬리면
        {
            AttachTail(i);                  // 주인공에게 붙이기
        }
    }

    g_speed = 35;                           // a모드에서는 주인공만 빨라짐
}

void ExitAMode()                            // a 모드 종료
{
    g_aMode = 0;                            // a 모드 끄기
    g_speed = DEFAULT_SPEED;                // 주인공 속도 기본값으로 복원
}

void RotateLeader()                         // 첫 번째 붙은 꼬리를 새 주인공으로 교체
{
    int firstIdx;                           // 첫 번째 붙은 꼬리 인덱스
    int oldHeroRow;                         // 이전 주인공 행
    int oldHeroCol;                         // 이전 주인공 열
    COLORREF oldHeroColor;                  // 이전 주인공 색
    int oldHeroShape;                       // 이전 주인공 모양
    int oldHeroLabel;                       // 이전 주인공 번호

    if (g_attachedHead == -1) return;       // 붙은 꼬리가 없으면 종료

    firstIdx = g_attachedHead;              // 첫 번째 붙은 꼬리 저장

    oldHeroRow = g_hero.row;                // 이전 주인공 위치 저장
    oldHeroCol = g_hero.col;                // 이전 주인공 위치 저장
    oldHeroColor = g_hero.color;            // 이전 주인공 색 저장
    oldHeroShape = g_hero.shape;            // 이전 주인공 모양 저장
    oldHeroLabel = g_heroLabel;             // 이전 주인공 번호 저장

    RemoveFromAttachedChain(firstIdx);      // 첫 꼬리를 붙은 체인에서 제거
    g_attachedCount--;                      // 붙은 꼬리 수 1 감소

    g_hero.row = g_tails[firstIdx].row;     // 첫 꼬리가 있던 곳이 새 주인공 위치
    g_hero.col = g_tails[firstIdx].col;     // 첫 꼬리가 있던 곳이 새 주인공 위치
    g_hero.color = g_tails[firstIdx].color; // 새 주인공 색
    g_hero.radius = HERO_BASE_RADIUS;       // 주인공 반지름 복원
    g_hero.shape = SHAPE_CIRCLE;            // 기본은 원 모양
    g_heroLabel = g_tails[firstIdx].label;  // 첫 꼬리 번호가 새 주인공 번호가 됨

    if (oldHeroShape == SHAPE_TRIANGLE) g_hero.shape = SHAPE_TRIANGLE; // 이전 주인공이 삼각형이면 유지

    g_tails[firstIdx].row = oldHeroRow;     // 빠진 꼬리는 이전 주인공 위치로 이동
    g_tails[firstIdx].col = oldHeroCol;     // 빠진 꼬리는 이전 주인공 위치로 이동
    g_tails[firstIdx].color = oldHeroColor; // 이전 주인공 색을 이어받음
    g_tails[firstIdx].label = oldHeroLabel; // 이전 주인공 번호를 꼬리로 넘김
    g_tails[firstIdx].alive = 1;            // 살아 있는 상태 유지

    ClearTailLinks(firstIdx);               // 연결 초기화
    AppendToAttachedChain(firstIdx);        // 체인 맨 뒤로 다시 붙임
    RebuildAttachedAppearance();            // 붙은 꼬리 외형 재정리
    g_rotateEffect = 6;                     // t 직후 잠깐 크게 보이도록 효과 시작
}

void ReverseDirForObstacle(int* dir)        // 방향 반전
{
    if (*dir == DIR_LEFT) *dir = DIR_RIGHT; // 왼->오
    else if (*dir == DIR_RIGHT) *dir = DIR_LEFT; // 오->왼
    else if (*dir == DIR_UP) *dir = DIR_DOWN; // 위->아래
    else if (*dir == DIR_DOWN) *dir = DIR_UP; // 아래->위
}

int GetFreeChainLeader(int idx)             // 자유꼬리 체인의 맨 앞 리더 찾기
{
    if (idx < 0 || idx >= MAX_TAIL) return -1; // 범위 검사
    if (!g_tails[idx].alive) return -1;     // 죽은 꼬리면 실패
    if (g_tails[idx].attached) return -1;   // 붙은 꼬리면 실패

    while (g_tails[idx].freePrev != -1)     // 앞 꼬리가 있는 동안
    {
        idx = g_tails[idx].freePrev;        // 계속 앞쪽으로 이동
    }

    return idx;                             // 맨 앞 리더 반환
}

int GetFreeChainTailEnd(int idx)            // 자유꼬리 체인의 맨 뒤 찾기
{
    if (idx < 0 || idx >= MAX_TAIL) return -1; // 범위 검사
    if (!g_tails[idx].alive) return -1;     // 죽은 꼬리면 실패
    if (g_tails[idx].attached) return -1;   // 붙은 꼬리면 실패

    while (g_tails[idx].freeNext != -1)     // 뒤 꼬리가 있는 동안
    {
        idx = g_tails[idx].freeNext;        // 계속 뒤쪽으로 이동
    }

    return idx;                             // 맨 뒤 반환
}

void ConnectFreeTailChains(int a, int b)    // 두 자유꼬리 체인을 하나로 연결
{
    int leaderA;                            // 첫 번째 체인 리더
    int leaderB;                            // 두 번째 체인 리더
    int endA;                               // 첫 번째 체인 맨 끝

    if (a < 0 || b < 0) return;             // 잘못된 인덱스면 종료
    if (a == b) return;                     // 자기 자신이면 종료
    if (!g_tails[a].alive || !g_tails[b].alive) return; // 둘 중 하나라도 없으면 종료
    if (g_tails[a].attached || g_tails[b].attached) return; // 붙은 꼬리는 제외

    leaderA = GetFreeChainLeader(a);        // a가 속한 체인 리더 찾기
    leaderB = GetFreeChainLeader(b);        // b가 속한 체인 리더 찾기

    if (leaderA == -1 || leaderB == -1) return; // 실패면 종료
    if (leaderA == leaderB) return;         // 이미 같은 체인이면 종료

    endA = GetFreeChainTailEnd(leaderA);    // a 체인의 맨 끝 찾기

    g_tails[endA].freeNext = leaderB;       // a 체인 맨 뒤에 b 체인 리더 연결
    g_tails[leaderB].freePrev = endA;       // b 체인 리더의 앞 꼬리를 a 체인 끝으로 지정
}

void MoveHero()                             // 주인공 한 칸 이동
{
    int chain[MAX_TAIL];                    // 붙은 꼬리 순서 저장 배열
    int oldRows[MAX_TAIL];                  // 붙은 꼬리들의 이전 row 저장
    int oldCols[MAX_TAIL];                  // 붙은 꼬리들의 이전 col 저장
    int count = 0;                          // 붙은 꼬리 개수
    int nextRow = g_hero.row;               // 다음 row 후보
    int nextCol = g_hero.col;               // 다음 col 후보
    int cur;                                // 현재 붙은 꼬리 인덱스
    int k;                                  // 반복 변수
    int oldHeroRow;                         // 이동 전 주인공 row
    int oldHeroCol;                         // 이동 전 주인공 col

    oldHeroRow = g_hero.row;                // 현재 주인공 row 저장
    oldHeroCol = g_hero.col;                // 현재 주인공 col 저장

    cur = g_attachedHead;                   // 첫 붙은 꼬리부터 시작
    while (cur != -1 && count < MAX_TAIL)   // 붙은 꼬리 순서 저장
    {
        chain[count] = cur;                 // 인덱스 저장
        oldRows[count] = g_tails[cur].row;  // 현재 row 저장
        oldCols[count] = g_tails[cur].col;  // 현재 col 저장
        count++;                            // 개수 증가
        cur = g_tails[cur].attachedNext;    // 다음 붙은 꼬리로 이동
    }

    if (g_jumpPending)                      // 점프 예약 상태라면
    {
        g_jumpReturnRow = g_hero.row;       // 점프 전 원래 행 저장
        g_jumpReturnCol = g_hero.col;       // 점프 전 원래 열 저장

        if (g_hero.dir == DIR_LEFT || g_hero.dir == DIR_RIGHT) // 가로 이동 중 점프라면
        {
            nextRow = g_hero.row - 2;       // 위로 2칸 벗어나기
            nextCol = g_hero.col;           // 현재 열 유지
        }
        else                                // 세로 이동 중 점프라면
        {
            nextRow = g_hero.row;           // 현재 행 유지
            nextCol = g_hero.col + 2;       // 오른쪽으로 2칸 벗어나기
        }

        g_jumpPending = 0;                  // 점프 예약 해제
        g_jumpReturnPending = 1;            // 다음 이동에서 복귀해야 함
    }
    else if (g_jumpReturnPending)           // 점프 후 자연스럽게 복귀할 차례라면
    {
        if (g_hero.dir == DIR_LEFT)         // 왼쪽 진행 중이었다면
        {
            nextRow = g_jumpReturnRow;      // 원래 행으로 복귀
            nextCol = g_hero.col - 2;       // 왼쪽으로 진행
        }
        else if (g_hero.dir == DIR_RIGHT)   // 오른쪽 진행 중이었다면
        {
            nextRow = g_jumpReturnRow;      // 원래 행으로 복귀
            nextCol = g_hero.col + 2;       // 오른쪽으로 진행
        }
        else if (g_hero.dir == DIR_UP)      // 위로 진행 중이었다면
        {
            nextRow = g_hero.row - 2;       // 위로 진행
            nextCol = g_jumpReturnCol;      // 원래 열로 복귀
        }
        else if (g_hero.dir == DIR_DOWN)    // 아래로 진행 중이었다면
        {
            nextRow = g_hero.row + 2;       // 아래로 진행
            nextCol = g_jumpReturnCol;      // 원래 열로 복귀
        }

        g_jumpReturnPending = 0;            // 복귀 완료
    }
    else                                    // 일반 이동
    {
        if (g_hero.dir == DIR_LEFT) nextCol--;  // 왼쪽
        else if (g_hero.dir == DIR_RIGHT) nextCol++; // 오른쪽
        else if (g_hero.dir == DIR_UP) nextRow--; // 위
        else if (g_hero.dir == DIR_DOWN) nextRow++; // 아래
    }

    if (!IsInsideBoardRC(nextRow, nextCol)) // 보드 밖으로 나가면
    {
        if (g_hero.dir == DIR_LEFT)         // 왼쪽 끝에서
        {
            nextRow = g_hero.row + 1;       // 한 칸 아래로 이동
            nextCol = g_hero.col;           // 현재 열 유지
            if (!IsInsideBoardRC(nextRow, nextCol)) nextRow = 0; // 맨 아래면 맨 위로 순환
            g_hero.dir = DIR_RIGHT;         // 방향을 오른쪽으로 반전
        }
        else if (g_hero.dir == DIR_RIGHT)   // 오른쪽 끝에서
        {
            nextRow = g_hero.row + 1;       // 한 칸 아래로 이동
            nextCol = g_hero.col;           // 현재 열 유지
            if (!IsInsideBoardRC(nextRow, nextCol)) nextRow = 0; // 맨 아래면 맨 위로 순환
            g_hero.dir = DIR_LEFT;          // 방향을 왼쪽으로 반전
        }
        else if (g_hero.dir == DIR_UP)      // 위쪽 끝에서
        {
            nextRow = g_hero.row;           // 현재 행 유지
            nextCol = g_hero.col + 1;       // 오른쪽으로 한 칸 이동
            if (!IsInsideBoardRC(nextRow, nextCol)) nextCol = 0; // 맨 오른쪽이면 맨 왼쪽으로 순환
            g_hero.dir = DIR_DOWN;          // 방향을 아래로 반전
        }
        else if (g_hero.dir == DIR_DOWN)    // 아래쪽 끝에서
        {
            nextRow = g_hero.row;           // 현재 행 유지
            nextCol = g_hero.col + 1;       // 오른쪽으로 한 칸 이동
            if (!IsInsideBoardRC(nextRow, nextCol)) nextCol = 0; // 맨 오른쪽이면 맨 왼쪽으로 순환
            g_hero.dir = DIR_UP;            // 방향을 위로 반전
        }
    }

    if (!g_aMode && IsObstacleAt(nextRow, nextCol)) // a 모드가 아니고 장애물을 만나면
    {
        ReverseDirForObstacle(&g_hero.dir); // 방향 반전
        nextRow = g_hero.row;               // 제자리
        nextCol = g_hero.col;               // 제자리
    }

    g_hero.row = nextRow;                   // 주인공 위치 반영
    g_hero.col = nextCol;                   // 주인공 위치 반영

    if (count > 0)                          // 붙은 꼬리가 있으면
    {
        g_tails[chain[0]].row = oldHeroRow; // 첫 꼬리는 이전 주인공 위치로 이동
        g_tails[chain[0]].col = oldHeroCol; // 첫 꼬리는 이전 주인공 위치로 이동

        for (k = 1; k < count; k++)         // 두 번째 꼬리부터 끝까지
        {
            g_tails[chain[k]].row = oldRows[k - 1]; // 앞 꼬리의 이전 위치 따라감
            g_tails[chain[k]].col = oldCols[k - 1]; // 앞 꼬리의 이전 위치 따라감
        }
    }

    if (g_aMode && g_hero.row == g_aStartRow && g_hero.col == g_aStartCol) // a 모드에서 원위치 도착
    {
        ExitAMode();                        // 종료
    }
}

void MoveOneFreeTail(int i)                 // 자유 꼬리 1개 이동
{
    int nr = g_tails[i].row;                // 다음 row 후보
    int nc = g_tails[i].col;                // 다음 col 후보

    if (!g_tails[i].alive || g_tails[i].attached) return; // 자유 꼬리 아니면 종료

    if (g_tails[i].moveType == MOVE_HORZ)   // 좌우 이동
    {
        if (g_tails[i].dir != DIR_LEFT && g_tails[i].dir != DIR_RIGHT) g_tails[i].dir = DIR_RIGHT; // 초기 방향
        if (g_tails[i].dir == DIR_LEFT) nc--; else nc++; // 한 칸 이동

        if (!IsInsideBoardRC(nr, nc) || IsObstacleAt(nr, nc)) // 막히면
        {
            ReverseDirForObstacle(&g_tails[i].dir); // 방향 반전
            nc = g_tails[i].col;            // 제자리
        }

        g_tails[i].row = nr;                // 반영
        g_tails[i].col = nc;                // 반영
        g_tails[i].drawRadius = g_tails[i].baseRadius; // 크기 유지
    }
    else if (g_tails[i].moveType == MOVE_VERT) // 상하 이동
    {
        if (g_tails[i].dir != DIR_UP && g_tails[i].dir != DIR_DOWN) g_tails[i].dir = DIR_DOWN; // 초기 방향
        if (g_tails[i].dir == DIR_UP) nr--; else nr++; // 한 칸 이동

        if (!IsInsideBoardRC(nr, nc) || IsObstacleAt(nr, nc)) // 막히면
        {
            ReverseDirForObstacle(&g_tails[i].dir); // 방향 반전
            nr = g_tails[i].row;            // 제자리
        }

        g_tails[i].row = nr;                // 반영
        g_tails[i].col = nc;                // 반영
        g_tails[i].drawRadius = g_tails[i].baseRadius; // 크기 유지
    }
    else if (g_tails[i].moveType == MOVE_RECT) // 사각형 경로 이동
    {
        if (g_tails[i].rectClockwise)       // 시계 방향 계열
        {
            if (g_tails[i].rectStep == 0) nc++; // 오른쪽
            else if (g_tails[i].rectStep == 1) nr++; // 아래
            else if (g_tails[i].rectStep == 2) nc--; // 왼쪽
            else nr--;                      // 위
        }
        else                                // 반시계 방향 계열
        {
            if (g_tails[i].rectStep == 0) nr--; // 위
            else if (g_tails[i].rectStep == 1) nc++; // 오른쪽
            else if (g_tails[i].rectStep == 2) nr++; // 아래
            else nc--;                      // 왼쪽
        }

        if (!IsInsideBoardRC(nr, nc) || IsObstacleAt(nr, nc)) // 막히면
        {
            g_tails[i].rectClockwise = !g_tails[i].rectClockwise; // 방향 전환
            g_tails[i].rectStep = 0;       // 단계 초기화
        }
        else                                // 정상 이동이면
        {
            g_tails[i].row = nr;           // 위치 반영
            g_tails[i].col = nc;           // 위치 반영
            g_tails[i].rectStep = (g_tails[i].rectStep + 1) % 4; // 다음 단계
        }

        g_tails[i].drawRadius = g_tails[i].baseRadius; // 크기 유지
    }
    else if (g_tails[i].moveType == MOVE_STOP) // 정지
    {
        g_tails[i].drawRadius = g_tails[i].baseRadius; // 그대로
    }
    else if (g_tails[i].moveType == MOVE_PULSE) // 크기 변화
    {
        g_tails[i].drawRadius += g_tails[i].pulseDir; // 크기 증감

        if (g_tails[i].drawRadius >= g_tails[i].baseRadius + 4) g_tails[i].pulseDir = -1; // 너무 커지면 줄이기
        if (g_tails[i].drawRadius <= g_tails[i].baseRadius - 4) g_tails[i].pulseDir = 1; // 너무 작아지면 키우기
    }
}

void MoveFreeTailChain(int leader)          // 자유꼬리 체인 한 줄 이동
{
    int chain[MAX_TAIL];                    // 체인 순서 저장 배열
    int oldRows[MAX_TAIL];                  // 이동 전 row 저장
    int oldCols[MAX_TAIL];                  // 이동 전 col 저장
    int count = 0;                          // 체인 길이
    int cur;                                // 현재 꼬리 인덱스
    int k;                                  // 반복 변수

    if (leader < 0 || leader >= MAX_TAIL) return; // 범위 검사
    if (!g_tails[leader].alive) return;     // 죽은 꼬리면 종료
    if (g_tails[leader].attached) return;   // 붙은 꼬리면 종료
    if (g_tails[leader].freePrev != -1) return; // 리더가 아니면 종료

    cur = leader;                           // 리더부터 시작
    while (cur != -1 && count < MAX_TAIL)   // 체인 전체 수집
    {
        chain[count] = cur;                 // 현재 꼬리 저장
        oldRows[count] = g_tails[cur].row;  // 이동 전 row 저장
        oldCols[count] = g_tails[cur].col;  // 이동 전 col 저장
        count++;                            // 개수 증가
        cur = g_tails[cur].freeNext;        // 다음 꼬리로 이동
    }

    MoveOneFreeTail(leader);                // 맨 앞 꼬리만 기존 방식으로 이동

    for (k = 1; k < count; k++)             // 두 번째 꼬리부터 끝까지
    {
        g_tails[chain[k]].row = oldRows[k - 1]; // 앞 꼬리의 이전 row 따라감
        g_tails[chain[k]].col = oldCols[k - 1]; // 앞 꼬리의 이전 col 따라감
        g_tails[chain[k]].drawRadius = g_tails[chain[k]].baseRadius; // 크기 고정
    }
}

void MoveFreeTails()                        // 자유꼬리 전체 이동
{
    int i;                                  // 반복 변수

    for (i = 0; i < MAX_TAIL; i++)          // 전체 꼬리 순회
    {
        if (!g_tails[i].alive) continue;    // 살아있는 꼬리만
        if (g_tails[i].attached) continue;  // 붙은 꼬리는 제외
        if (g_tails[i].freePrev != -1) continue; // 체인 리더가 아니면 건너뜀

        MoveFreeTailChain(i);               // 체인 맨 앞만 이동시키고 뒤는 따라오게 함
    }
}

void CheckFoodCollision()                   // 주인공이 먹이를 먹었는지 검사
{
    int idx;                                // 먹이 인덱스

    idx = IsFoodAt(g_hero.row, g_hero.col); // 현재 칸에 먹이 있는지 검사
    if (idx != -1)                          // 먹이가 있으면
    {
        g_hero.color = g_foods[idx].color;  // 주인공 색 변경
        SpawnTailFromFood(g_foods[idx].color); // 랜덤한 빈 칸에 자유 꼬리 생성
        SpawnOneFood(idx);                  // 먹이 재생성
        RebuildAttachedAppearance();        // 붙은 꼬리 색 맞추기
    }
}

void AttachFreeChainToHero(int idx)         // 자유체인 전체를 주인공에게 붙이는 함수
{
    int leader;                             // 체인 리더
    int order[MAX_TAIL];                    // 체인 순서 저장
    int count = 0;                          // 체인 길이
    int cur;                                // 순회용 인덱스
    int k;                                  // 반복 변수

    leader = GetFreeChainLeader(idx);       // 충돌한 꼬리의 리더 찾기
    if (leader == -1) return;               // 실패면 종료

    cur = leader;                           // 리더부터 시작
    while (cur != -1 && count < MAX_TAIL)   // 체인 전체 수집
    {
        order[count++] = cur;               // 현재 꼬리 저장
        cur = g_tails[cur].freeNext;        // 다음 꼬리로 이동
    }

    for (k = 0; k < count; k++)             // 앞에서부터 순서대로 주인공 뒤에 붙임
    {
        ClearTailLinks(order[k]);           // 자유체인 연결 끊기
        AttachTail(order[k]);               // 주인공 뒤에 붙이기
    }
}

void CheckHeroTailCollision()               // 주인공과 자유 꼬리 충돌 검사
{
    int idx;                                // 꼬리 인덱스

    idx = IsFreeTailAt(g_hero.row, g_hero.col); // 현재 칸에 자유 꼬리 검사
    if (idx != -1)                          // 있으면
    {
        AttachFreeChainToHero(idx);         // 그 꼬리가 속한 자유체인 전체를 붙임
    }
}

void CheckTailTailCollision()               // 자유 꼬리끼리 충돌 검사
{
    int i, j;                               // 반복 변수
    int leaderI, leaderJ;                   // 각 꼬리가 속한 체인의 리더

    for (i = 0; i < MAX_TAIL; i++)          // 첫 번째 자유꼬리 검사
    {
        if (!g_tails[i].alive || g_tails[i].attached) continue; // 자유꼬리만 검사

        for (j = i + 1; j < MAX_TAIL; j++)  // 두 번째 자유꼬리 검사
        {
            if (!g_tails[j].alive || g_tails[j].attached) continue; // 자유꼬리만 검사

            if (g_tails[i].row == g_tails[j].row && g_tails[i].col == g_tails[j].col) // 같은 칸에서 만나면
            {
                leaderI = GetFreeChainLeader(i); // i 체인 리더 찾기
                leaderJ = GetFreeChainLeader(j); // j 체인 리더 찾기

                if (leaderI == -1 || leaderJ == -1) continue; // 실패면 무시
                if (leaderI == leaderJ) continue; // 이미 같은 체인이면 무시

                ConnectFreeTailChains(leaderI, leaderJ); // 두 체인을 하나로 연결

                g_tails[leaderJ].moveType = g_tails[leaderI].moveType; // 이동 방식 맞춤
                g_tails[leaderJ].dir = g_tails[leaderI].dir; // 방향 맞춤
                g_tails[leaderJ].rectStep = g_tails[leaderI].rectStep; // 단계 맞춤
                g_tails[leaderJ].rectClockwise = g_tails[leaderI].rectClockwise; // 방향 맞춤
            }
        }
    }
}

void MoveGame(HWND hWnd)                    // 게임 전체 갱신
{
    int heroMoved = 0;                      // 이번 틱에 주인공이 실제로 움직였는지 여부
    int tailMoved = 0;                      // 이번 틱에 자유꼬리가 실제로 움직였는지 여부

    if (!g_started) return;                 // 시작 전이면 종료

    g_heroTick += BASE_TIMER_INTERVAL;      // 주인공 누적 시간 증가
    g_tailTick += BASE_TIMER_INTERVAL;      // 자유꼬리 누적 시간 증가

    if (g_heroTick >= g_speed)              // 주인공 이동 시간이 되었으면
    {
        g_heroTick = 0;                     // 주인공 누적 시간 초기화
        MoveHero();                         // 주인공 이동
        CheckFoodCollision();               // 주인공이 움직인 뒤 먹이 충돌 검사
        CheckHeroTailCollision();           // 주인공이 움직인 뒤 자유꼬리 충돌 검사
        heroMoved = 1;                      // 주인공 이동 표시
    }

    if (g_tailTick >= DEFAULT_SPEED)        // 자유꼬리 이동 시간이 되었으면
    {
        g_tailTick = 0;                     // 자유꼬리 누적 시간 초기화
        MoveFreeTails();                    // 자유 꼬리 이동
        CheckTailTailCollision();           // 자유꼬리끼리 충돌 검사
        tailMoved = 1;                      // 자유꼬리 이동 표시
    }

    if (g_rotateEffect > 0)                 // 리더교체 강조 효과가 남아 있으면
    {
        g_rotateEffect--;                   // 한 틱마다 효과 시간 감소
        InvalidateRect(hWnd, NULL, TRUE);   // 효과가 보이도록 다시 그림
    }

    if (heroMoved || tailMoved)             // 둘 중 하나라도 움직였으면
    {
        InvalidateRect(hWnd, NULL, TRUE);   // 화면 다시 그리기
    }
}

int GetBoardRowFromY(int y)                 // y 좌표를 보드 row로 변환
{
    return (y - BOARD_TOP) / CELL_SIZE;     // 계산 후 반환
}

int GetBoardColFromX(int x)                 // x 좌표를 보드 col로 변환
{
    return (x - BOARD_LEFT) / CELL_SIZE;    // 계산 후 반환
}

void AddObstacleByMouse(int x, int y)       // 오른쪽 클릭으로 장애물 생성
{
    int row = GetBoardRowFromY(y);          // row 변환
    int col = GetBoardColFromX(x);          // col 변환
    int i;                                  // 반복 변수

    if (!IsInsideBoardRC(row, col)) return; // 보드 밖이면 종료
    if (g_obstacleCount >= MAX_OBS) return; // 최대 개수면 종료
    if (IsObstacleAt(row, col)) return;     // 이미 있으면 종료
    if (row == g_hero.row && col == g_hero.col) return; // 주인공 칸 제외
    if (IsFoodAt(row, col) != -1) return;   // 먹이 칸 제외
    if (IsFreeTailAt(row, col) != -1) return; // 자유꼬리 칸 제외

    for (i = 0; i < MAX_OBS; i++)           // 빈 장애물 슬롯 찾기
    {
        if (!g_obs[i].alive)                // 비어 있으면
        {
            g_obs[i].row = row;             // 저장
            g_obs[i].col = col;             // 저장
            g_obs[i].alive = 1;             // 활성화
            g_obstacleCount++;              // 개수 증가
            break;                          // 끝
        }
    }
}

void ChangeHeroDirectionByClick(int x, int y) // 클릭 방향 기준으로 주인공 방향 변경
{
    int hx, hy;                             // 주인공 중심 좌표
    int dx, dy;                             // 거리 차이

    RCToXY(g_hero.row, g_hero.col, &hx, &hy); // 주인공 중심 좌표 구하기
    dx = x - hx;                            // x 차이
    dy = y - hy;                            // y 차이

    if (abs(dx) > abs(dy))                  // 가로 차이가 더 크면
    {
        if (dx < 0) g_hero.dir = DIR_LEFT;  // 왼쪽
        else g_hero.dir = DIR_RIGHT;        // 오른쪽
    }
    else                                    // 세로 차이가 더 크면
    {
        if (dy < 0) g_hero.dir = DIR_UP;    // 위
        else g_hero.dir = DIR_DOWN;         // 아래
    }
}

int HitHero(int x, int y)                   // 주인공 클릭 판정
{
    int hx, hy;                             // 중심 좌표
    int dx, dy;                             // 거리 차이

    RCToXY(g_hero.row, g_hero.col, &hx, &hy); // 좌표 변환
    dx = x - hx;                            // 차이 계산
    dy = y - hy;                            // 차이 계산

    if (dx * dx + dy * dy <= g_hero.radius * g_hero.radius) return 1; // 원 내부면 1
    return 0;                               // 아니면 0
}

int HitAttachedTail(int x, int y)           // 붙은 꼬리 클릭 판정
{
    int idx;                                // 현재 순회 중인 붙은 꼬리 인덱스
    int tx, ty;                             // 꼬리 중심 좌표
    int dx, dy;                             // 거리 차이

    idx = g_attachedHead;                   // 첫 붙은 꼬리부터 시작
    while (idx != -1)                       // 끝까지 순회
    {
        RCToXY(g_tails[idx].row, g_tails[idx].col, &tx, &ty); // 중심 좌표 계산
        dx = x - tx;                        // x 차이
        dy = y - ty;                        // y 차이

        if (dx * dx + dy * dy <= g_tails[idx].drawRadius * g_tails[idx].drawRadius) return idx; // 내부면 해당 꼬리 인덱스 반환
        idx = g_tails[idx].attachedNext;    // 다음 붙은 꼬리로 이동
    }

    return -1;                              // 클릭한 붙은 꼬리 없음
}

void DrawCircleCell(HDC hDC, int row, int col, int radius, COLORREF color) // 원 그리기
{
    int x, y;                               // 중심 좌표
    HBRUSH hBrush, oldBrush;                // 브러시
    HPEN hPen, oldPen;                      // 펜

    RCToXY(row, col, &x, &y);               // 좌표 변환
    hBrush = CreateSolidBrush(color);       // 채우기 브러시 생성
    hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0)); // 테두리 펜 생성

    oldBrush = (HBRUSH)SelectObject(hDC, hBrush); // 브러시 선택
    oldPen = (HPEN)SelectObject(hDC, hPen); // 펜 선택
    Ellipse(hDC, x - radius, y - radius, x + radius, y + radius); // 원 그리기
    SelectObject(hDC, oldBrush);            // 원래 브러시 복구
    SelectObject(hDC, oldPen);              // 원래 펜 복구
    DeleteObject(hBrush);                   // 브러시 삭제
    DeleteObject(hPen);                     // 펜 삭제
}

void DrawTriangleCell(HDC hDC, int row, int col, int radius, COLORREF color) // 삼각형 그리기
{
    int x, y;                               // 중심 좌표
    POINT pt[3];                            // 꼭짓점 배열
    HBRUSH hBrush, oldBrush;                // 브러시
    HPEN hPen, oldPen;                      // 펜

    RCToXY(row, col, &x, &y);               // 좌표 변환

    pt[0].x = x;                            // 위쪽 꼭짓점 x
    pt[0].y = y - radius;                   // 위쪽 꼭짓점 y
    pt[1].x = x - radius;                   // 왼쪽 아래 x
    pt[1].y = y + radius;                   // 왼쪽 아래 y
    pt[2].x = x + radius;                   // 오른쪽 아래 x
    pt[2].y = y + radius;                   // 오른쪽 아래 y

    hBrush = CreateSolidBrush(color);       // 브러시 생성
    hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0)); // 펜 생성
    oldBrush = (HBRUSH)SelectObject(hDC, hBrush); // 선택
    oldPen = (HPEN)SelectObject(hDC, hPen); // 선택
    Polygon(hDC, pt, 3);                    // 삼각형 그리기
    SelectObject(hDC, oldBrush);            // 복구
    SelectObject(hDC, oldPen);              // 복구
    DeleteObject(hBrush);                   // 삭제
    DeleteObject(hPen);                     // 삭제
}

void DrawRectCell(HDC hDC, int row, int col, COLORREF color) // 사각형 먹이 그리기
{
    int x, y;                               // 중심 좌표
    HBRUSH hBrush, oldBrush;                // 브러시
    HPEN hPen, oldPen;                      // 펜

    RCToXY(row, col, &x, &y);               // 좌표 변환

    hBrush = CreateSolidBrush(color);       // 브러시 생성
    hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0)); // 펜 생성
    oldBrush = (HBRUSH)SelectObject(hDC, hBrush); // 선택
    oldPen = (HPEN)SelectObject(hDC, hPen); // 선택
    Rectangle(hDC, x - 6, y - 6, x + 6, y + 6); // 사각형 그리기
    SelectObject(hDC, oldBrush);            // 복구
    SelectObject(hDC, oldPen);              // 복구
    DeleteObject(hBrush);                   // 삭제
    DeleteObject(hPen);                     // 삭제
}

void DrawBoard(HDC hDC)                     // 보드 그리기
{
    int i;                                  // 반복 변수

    Rectangle(hDC, BOARD_LEFT, BOARD_TOP, BOARD_LEFT + BOARD_WIDTH, BOARD_TOP + BOARD_HEIGHT); // 바깥 테두리

    for (i = 1; i < BOARD_COLS; i++)        // 세로선 그리기
    {
        MoveToEx(hDC, BOARD_LEFT + i * CELL_SIZE, BOARD_TOP, NULL); // 시작점
        LineTo(hDC, BOARD_LEFT + i * CELL_SIZE, BOARD_TOP + BOARD_HEIGHT); // 끝점
    }

    for (i = 1; i < BOARD_ROWS; i++)        // 가로선 그리기
    {
        MoveToEx(hDC, BOARD_LEFT, BOARD_TOP + i * CELL_SIZE, NULL); // 시작점
        LineTo(hDC, BOARD_LEFT + BOARD_WIDTH, BOARD_TOP + i * CELL_SIZE); // 끝점
    }
}

void DrawFoods(HDC hDC)                     // 먹이 그리기
{
    int i;                                  // 반복 변수
    for (i = 0; i < MAX_FOOD; i++)          // 전체 먹이 검사
    {
        if (!g_foods[i].alive) continue;    // 살아 있는 먹이만 그림
        if (g_foods[i].shape == 0) DrawCircleCell(hDC, g_foods[i].row, g_foods[i].col, 5, g_foods[i].color); // 원 먹이
        else DrawRectCell(hDC, g_foods[i].row, g_foods[i].col, g_foods[i].color); // 사각형 먹이
    }
}

void DrawObstacles(HDC hDC)                 // 장애물 그리기
{
    int i;                                  // 반복 변수
    int x, y;                               // 좌표
    HBRUSH hBrush, oldBrush;                // 브러시
    HPEN hPen, oldPen;                      // 펜

    hBrush = CreateSolidBrush(RGB(80, 80, 80)); // 회색 브러시
    hPen = CreatePen(PS_SOLID, 1, RGB(20, 20, 20)); // 진한 펜
    oldBrush = (HBRUSH)SelectObject(hDC, hBrush); // 선택
    oldPen = (HPEN)SelectObject(hDC, hPen); // 선택

    for (i = 0; i < MAX_OBS; i++)           // 장애물 전체 검사
    {
        if (!g_obs[i].alive) continue;      // 살아 있는 장애물만
        RCToXY(g_obs[i].row, g_obs[i].col, &x, &y); // 좌표 변환
        Rectangle(hDC, x - 7, y - 7, x + 7, y + 7); // 사각형으로 그림
    }

    SelectObject(hDC, oldBrush);            // 복구
    SelectObject(hDC, oldPen);              // 복구
    DeleteObject(hBrush);                   // 삭제
    DeleteObject(hPen);                     // 삭제
}

void DrawHero(HDC hDC)                      // 주인공 그리기
{
    int drawR = g_hero.radius;              // 기본 반지름
    int x, y;                               // 주인공 중심 좌표
    HPEN hPen, oldPen;                      // 강조 테두리용 펜
    HBRUSH oldBrush;                        // 원래 브러시 저장용

    if (g_rotateEffect > 0) drawR += 6;     // t 직후에는 잠깐 더 크게 그림

    if (g_hero.shape == SHAPE_CIRCLE)       // 주인공이 원 모양이면
    {
        DrawCircleCell(hDC, g_hero.row, g_hero.col, drawR, g_hero.color); // 원으로 그림
    }
    else                                    // 주인공이 삼각형이면
    {
        DrawTriangleCell(hDC, g_hero.row, g_hero.col, drawR, g_hero.color); // 삼각형으로 그림
    }

    RCToXY(g_hero.row, g_hero.col, &x, &y); // 주인공 중심 좌표 계산

    if (g_rotateEffect > 0)                 // t 직후에는 더 두껍고 눈에 띄게
    {
        hPen = CreatePen(PS_SOLID, 4, RGB(0, 0, 0)); // 굵은 검은 펜 생성
    }
    else                                    // 평소에는 얇은 강조 테두리
    {
        hPen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0)); // 일반 검은 펜 생성
    }

    oldPen = (HPEN)SelectObject(hDC, hPen); // 펜 선택
    oldBrush = (HBRUSH)SelectObject(hDC, GetStockObject(HOLLOW_BRUSH)); // 내부 비우기

    Rectangle(hDC, x - drawR - 4, y - drawR - 4, x + drawR + 4, y + drawR + 4); // 주인공 바깥 강조 테두리

    SelectObject(hDC, oldBrush);            // 원래 브러시 복구
    SelectObject(hDC, oldPen);              // 원래 펜 복구
    DeleteObject(hPen);                     // 펜 삭제
}

void DrawTails(HDC hDC)                     // 꼬리 그리기
{
    int i;                                  // 반복 변수
    for (i = 0; i < MAX_TAIL; i++)          // 전체 꼬리 검사
    {
        if (!g_tails[i].alive) continue;    // 살아 있는 꼬리만 그림
        DrawCircleCell(hDC, g_tails[i].row, g_tails[i].col, g_tails[i].drawRadius, g_tails[i].color); // 원으로 그림
    }
}

void DrawTextInfo(HDC hDC)                  // 상태 텍스트 출력
{
    TCHAR buf[256];                         // 문자열 버퍼

    wsprintf(buf, L"s:시작  q:종료  +/-:주인공속도  j:점프  t:리더교체  a:특수모드"); // 설명 문장
    TextOut(hDC, BOARD_LEFT, BOARD_TOP + BOARD_HEIGHT + 15, buf, lstrlen(buf)); // 출력

    wsprintf(buf, L"주인공속도:%d   자유꼬리속도:%d   붙은꼬리:%d   장애물:%d   a모드:%s",
        g_speed, DEFAULT_SPEED, g_attachedCount, g_obstacleCount, g_aMode ? L"ON" : L"OFF"); // 상태 문장
    TextOut(hDC, BOARD_LEFT, BOARD_TOP + BOARD_HEIGHT + 35, buf, lstrlen(buf)); // 출력
}

void DrawScene(HDC hDC)                     // 전체 장면 그리기
{
    DrawBoard(hDC);                         // 보드
    DrawFoods(hDC);                         // 먹이
    DrawObstacles(hDC);                     // 장애물
    DrawTails(hDC);                         // 꼬리
    DrawHero(hDC);                          // 주인공
    DrawTailNumbers(hDC);                   // 주인공과 붙은 꼬리 번호 표시
    DrawTextInfo(hDC);                      // 안내 문구
}

void DrawTailNumbers(HDC hDC)               // 주인공과 붙은 꼬리에 번호를 표시하는 함수
{
    TCHAR buf[32];                          // 번호 문자열 저장용 버퍼
    int x, y;                               // 각 객체 중심 좌표
    int idx;                                // 붙은 꼬리 순회용 인덱스
    COLORREF oldTextColor;                  // 원래 글자색 저장
    int oldBkMode;                          // 원래 배경 모드 저장

    oldTextColor = SetTextColor(hDC, RGB(0, 0, 0)); // 번호를 검은색으로 설정
    oldBkMode = SetBkMode(hDC, TRANSPARENT);        // 글자 배경을 투명하게 설정

    RCToXY(g_hero.row, g_hero.col, &x, &y); // 주인공 중심 좌표 계산
    wsprintf(buf, L"%d", g_heroLabel);      // 주인공이 가진 번호 출력
    TextOut(hDC, x - 6, y - 8, buf, lstrlen(buf)); // 주인공 위에 번호 출력

    idx = g_attachedHead;                   // 첫 번째 붙은 꼬리부터 시작

    while (idx != -1)                       // 붙은 꼬리 끝까지 반복
    {
        RCToXY(g_tails[idx].row, g_tails[idx].col, &x, &y); // 꼬리 중심 좌표 계산
        wsprintf(buf, L"%d", g_tails[idx].label);            // 각 꼬리의 고정 번호 출력
        TextOut(hDC, x - 6, y - 8, buf, lstrlen(buf));       // 꼬리 위에 번호 출력

        idx = g_tails[idx].attachedNext;    // 다음 붙은 꼬리로 이동
    }

    SetTextColor(hDC, oldTextColor);        // 원래 글자색 복구
    SetBkMode(hDC, oldBkMode);              // 원래 배경 모드 복구
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow) // 프로그램 시작점
{
    HWND hWnd;                              // 윈도우 핸들
    MSG Message;                            // 메시지 구조체
    WNDCLASSEX WndClass;                    // 윈도우 클래스 구조체

    srand((unsigned int)time(NULL));        // 난수 시드 설정

    g_hInst = hInstance;                    // 인스턴스 저장
    WndClass.cbSize = sizeof(WndClass);     // 구조체 크기
    WndClass.style = CS_HREDRAW | CS_VREDRAW; // 다시 그리기 스타일
    WndClass.lpfnWndProc = (WNDPROC)WndProc; // 메시지 함수 연결
    WndClass.cbClsExtra = 0;                // 추가 메모리 없음
    WndClass.cbWndExtra = 0;                // 추가 메모리 없음
    WndClass.hInstance = hInstance;         // 인스턴스 연결
    WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION); // 기본 아이콘
    WndClass.hCursor = LoadCursor(NULL, IDC_ARROW); // 기본 커서
    WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); // 흰 배경
    WndClass.lpszMenuName = NULL;           // 메뉴 없음
    WndClass.lpszClassName = lpszClass;     // 클래스 이름
    WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION); // 작은 아이콘

    RegisterClassEx(&WndClass);             // 클래스 등록

    hWnd = CreateWindow(                    // 윈도우 생성
        lpszClass,                          // 클래스명
        lpszWindowName,                     // 제목
        WS_OVERLAPPEDWINDOW,                // 스타일
        0, 0, 800, 760,                     // 위치와 크기
        NULL, (HMENU)NULL, hInstance, NULL);// 나머지 옵션

    ShowWindow(hWnd, nCmdShow);             // 윈도우 표시
    UpdateWindow(hWnd);                     // 화면 갱신

    while (GetMessage(&Message, 0, 0, 0))   // 메시지 루프
    {
        TranslateMessage(&Message);         // 메시지 변환
        DispatchMessage(&Message);          // WndProc으로 전달
    }

    return (int)Message.wParam;             // 종료 코드 반환
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) // 메시지 처리 함수
{
    PAINTSTRUCT ps;                         // 페인트 구조체
    HDC hDC;                                // 디바이스 컨텍스트
    int mx, my;                             // 마우스 좌표
    int tailIdx;                            // 클릭한 꼬리 인덱스

    switch (uMsg)                           // 메시지 분기
    {
    case WM_CREATE:                         // 생성 시
        GetClientRect(hWnd, &g_clientRect); // 클라이언트 크기 저장
        InitGame();                         // 게임 초기화
        SetTimer(hWnd, TIMER_ID, BASE_TIMER_INTERVAL, NULL); // 짧은 간격 공통 타이머 시작
        break;                              // 종료

    case WM_SIZE:                           // 크기 변경 시
        GetClientRect(hWnd, &g_clientRect); // 클라이언트 크기 갱신
        break;                              // 종료

    case WM_TIMER:                          // 타이머 메시지
        if (wParam == TIMER_ID)             // 우리 타이머면
        {
            MoveGame(hWnd);                 // 게임 갱신
        }
        break;                              // 종료

    case WM_LBUTTONDOWN:                    // 왼쪽 클릭
        mx = LOWORD(lParam);                // x 추출
        my = HIWORD(lParam);                // y 추출

        if (HitHero(mx, my))                // 주인공 클릭이면
        {
            DetachAllTails();               // 붙은 꼬리 제거
            if (g_hero.shape == SHAPE_CIRCLE) g_hero.shape = SHAPE_TRIANGLE; // 원->삼각형
            else g_hero.shape = SHAPE_CIRCLE; // 삼각형->원
        }
        else                                // 주인공 클릭이 아니면
        {
            tailIdx = HitAttachedTail(mx, my); // 붙은 꼬리 클릭인지 검사

            if (tailIdx != -1)              // 붙은 꼬리 클릭이면
            {
                DetachTailChainFrom(tailIdx); // 그 꼬리부터 뒤를 분리
            }
            else                            // 빈 보드 클릭이면
            {
                ChangeHeroDirectionByClick(mx, my); // 방향 전환
            }
        }

        InvalidateRect(hWnd, NULL, TRUE);   // 다시 그리기
        break;                              // 종료

    case WM_RBUTTONDOWN:                    // 오른쪽 클릭
        mx = LOWORD(lParam);                // x
        my = HIWORD(lParam);                // y
        AddObstacleByMouse(mx, my);         // 장애물 추가
        InvalidateRect(hWnd, NULL, TRUE);   // 다시 그리기
        break;                              // 종료

    case WM_KEYDOWN:                        // 키 입력
        switch (wParam)                     // 어떤 키인지 분기
        {
        case 'S':                           // s
            g_started = 1;                  // 시작
            break;                          // 종료

        case VK_LEFT:                       // 왼쪽 방향키
            g_hero.dir = DIR_LEFT;          // 방향 설정
            break;                          // 종료

        case VK_RIGHT:                      // 오른쪽 방향키
            g_hero.dir = DIR_RIGHT;         // 방향 설정
            break;                          // 종료

        case VK_UP:                         // 위 방향키
            g_hero.dir = DIR_UP;            // 방향 설정
            break;                          // 종료

        case VK_DOWN:                       // 아래 방향키
            g_hero.dir = DIR_DOWN;          // 방향 설정
            break;                          // 종료

        case VK_OEM_PLUS:                   // 일반 키보드 +
        case VK_ADD:                        // 숫자패드 +
            if (g_speed > 20) g_speed -= 10; // 주인공만 빠르게
            break;                          // 종료

        case VK_OEM_MINUS:                  // 일반 키보드 -
        case VK_SUBTRACT:                   // 숫자패드 -
            if (g_speed < 500) g_speed += 10; // 주인공만 느리게
            break;                          // 종료

        case 'J':                           // j
            g_jumpPending = 1;              // 점프 예약
            break;                          // 종료

        case 'T':                           // t
            RotateLeader();                 // 리더 교체
            break;                          // 종료

        case 'A':                           // a
            if (!g_aMode) EnterAMode();     // 꺼져 있으면 시작
            else ExitAMode();               // 켜져 있으면 종료
            break;                          // 종료

        case 'Q':                           // q
            DestroyWindow(hWnd);            // 종료
            break;                          // 종료
        }

        InvalidateRect(hWnd, NULL, TRUE);   // 다시 그리기
        break;                              // 종료

    case WM_PAINT:                          // 그리기 메시지
        hDC = BeginPaint(hWnd, &ps);        // 그리기 시작
        DrawScene(hDC);                     // 전체 그리기
        EndPaint(hWnd, &ps);                // 그리기 끝
        break;                              // 종료

    case WM_DESTROY:                        // 종료 시
        KillTimer(hWnd, TIMER_ID);          // 타이머 제거
        PostQuitMessage(0);                 // 메시지 루프 종료
        break;                              // 종료
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam); // 나머지 메시지는 기본 처리
}