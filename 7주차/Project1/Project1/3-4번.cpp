#include <windows.h>                                  // 윈도우 API 사용을 위한 헤더 파일
#include <tchar.h>                                    // TCHAR, LPCTSTR 같은 문자열 타입을 사용하기 위한 헤더
#include <stdlib.h>                                   // rand, srand 함수 사용을 위한 헤더
#include <time.h>                                     // time 함수 사용을 위한 헤더

// ------------------------------------------------------------
// 프로그램 기본 전역 변수
// ------------------------------------------------------------
HINSTANCE g_hInst;                                    // 현재 실행 중인 프로그램 인스턴스 핸들 저장용 변수
LPCTSTR lpszClass = L"My Window Class";               // 윈도우 클래스 이름 문자열
LPCTSTR lpszWindowName = L"Window Programming Lab";   // 실제 창 제목 문자열

// ------------------------------------------------------------
// 함수 원형 선언
// ------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam); // 메시지 처리 함수 선언

// ------------------------------------------------------------
// 게임/보드 관련 상수 정의
// ------------------------------------------------------------
#define BOARD_SIZE 30                                 // 보드 크기: 30 x 30
#define CELL_SIZE 20                                  // 한 칸의 가로/세로 픽셀 크기
#define BOARD_LEFT 40                                 // 보드가 시작되는 화면 x 좌표
#define BOARD_TOP 40                                  // 보드가 시작되는 화면 y 좌표

#define TIMER_ID 1                                    // 자동 낙하용 타이머 ID
#define TIMER_INTERVAL 250                            // 자동 낙하 속도 (250ms마다 한 번)

#define INITIAL_ITEM_COUNT 60                         // 처음 생성할 아이템 개수
#define MAX_GROUP_CELLS 900                           // 현재 떨어지는 묶음의 최대 칸 수 (30x30 전체까지 가능)

// ------------------------------------------------------------
// 칸 색상 번호 정의
// 0 = 빈칸
// 1~4 = 실제 색상 아이템
// ------------------------------------------------------------
#define EMPTY 0                                       // 빈칸을 뜻하는 값
#define ITEM_RED 1                                    // 빨간색 아이템 번호
#define ITEM_GREEN 2                                  // 초록색 아이템 번호
#define ITEM_BLUE 3                                   // 파란색 아이템 번호
#define ITEM_YELLOW 4                                 // 노란색 아이템 번호

// ------------------------------------------------------------
// 보드 배열 2개 사용
// itemBoard  : 아직 먹히지 않은 아이템 저장
// fixedBoard : 바닥에 쌓여서 더 이상 움직이지 않는 블록 저장
// ------------------------------------------------------------
int itemBoard[BOARD_SIZE][BOARD_SIZE];                // 미리 배치된 아이템 또는 마우스로 추가한 아이템 저장 배열
int fixedBoard[BOARD_SIZE][BOARD_SIZE];               // 바닥에 쌓인 꼬리 블록 저장 배열

// ------------------------------------------------------------
// 현재 떨어지는 묶음(주인공 + 흡수한 꼬리들) 정보
// groupX[i], groupY[i]   : i번째 칸의 좌표
// groupColor[i]          : i번째 칸의 색상 번호
// groupCount             : 현재 묶음 칸 수
// headIndex              : 주인공 원이 몇 번째 칸인지 저장
// ------------------------------------------------------------
int groupX[MAX_GROUP_CELLS];                          // 현재 떨어지는 묶음 각 칸의 x 좌표 저장 배열
int groupY[MAX_GROUP_CELLS];                          // 현재 떨어지는 묶음 각 칸의 y 좌표 저장 배열
int groupColor[MAX_GROUP_CELLS];                      // 현재 떨어지는 묶음 각 칸의 색상 번호 저장 배열
int groupCount = 0;                                   // 현재 묶음에 들어 있는 총 칸 수
int headIndex = 0;                                    // 현재 묶음에서 주인공 원이 들어 있는 인덱스

// ------------------------------------------------------------
// 회전 상태 저장
// 엔터를 누를 때마다 0 -> 1 -> 2 -> 3 -> 0 순으로 바뀜
// 실제 게임 진행에 꼭 필요하지는 않지만 상태 확인용으로 둠
// ------------------------------------------------------------
int rotationState = 0;                                // 현재 회전 상태 저장 변수

// ------------------------------------------------------------
// 색상 번호를 실제 RGB 색으로 바꾸는 함수
// ------------------------------------------------------------
COLORREF GetColorFromId(int colorId)                  // 색 번호를 받아 COLORREF 값으로 바꾸는 함수 시작
{                                                     // 함수 본문 시작
    switch (colorId) {                                // 색 번호에 따라 분기 처리
    case ITEM_RED:                                    // 빨간색 번호이면
        return RGB(230, 70, 70);                      // 빨간색 반환
    case ITEM_GREEN:                                  // 초록색 번호이면
        return RGB(70, 180, 90);                      // 초록색 반환
    case ITEM_BLUE:                                   // 파란색 번호이면
        return RGB(70, 120, 230);                     // 파란색 반환
    case ITEM_YELLOW:                                 // 노란색 번호이면
        return RGB(240, 200, 60);                     // 노란색 반환
    }                                                 // switch 종료

    return RGB(255, 255, 255);                        // 혹시 예외 상황이면 흰색 반환
}                                                     // 함수 끝

// ------------------------------------------------------------
// 보드 x 좌표를 실제 화면 픽셀 x 좌표로 바꾸는 함수
// ------------------------------------------------------------
int CellToPixelX(int x)                               // 보드 x를 화면 x로 바꾸는 함수
{                                                     // 함수 본문 시작
    return BOARD_LEFT + x * CELL_SIZE;                // 보드 시작 위치 + 칸 번호 * 칸 크기 반환
}                                                     // 함수 끝

// ------------------------------------------------------------
// 보드 y 좌표를 실제 화면 픽셀 y 좌표로 바꾸는 함수
// ------------------------------------------------------------
int CellToPixelY(int y)                               // 보드 y를 화면 y로 바꾸는 함수
{                                                     // 함수 본문 시작
    return BOARD_TOP + y * CELL_SIZE;                 // 보드 시작 위치 + 칸 번호 * 칸 크기 반환
}                                                     // 함수 끝

// ------------------------------------------------------------
// 현재 떨어지는 묶음 안에 특정 좌표가 이미 들어 있는지 검사
// ------------------------------------------------------------
int IsInGroup(int x, int y)                           // 좌표가 현재 묶음 안에 있는지 검사하는 함수
{                                                     // 함수 본문 시작
    int i;                                            // 반복문에서 사용할 변수 선언

    for (i = 0; i < groupCount; i++) {                // 현재 묶음의 모든 칸을 하나씩 검사
        if (groupX[i] == x && groupY[i] == y) {       // x와 y가 둘 다 같으면
            return 1;                                 // 이미 묶음 안에 있다는 뜻이므로 1 반환
        }                                             // if 종료
    }                                                 // for 종료

    return 0;                                         // 끝까지 못 찾았으면 묶음 안에 없으므로 0 반환
}                                                     // 함수 끝

// ------------------------------------------------------------
// itemBoard와 fixedBoard를 모두 빈칸으로 초기화하는 함수
// ------------------------------------------------------------
void ClearBoards(void)                                // 두 개의 보드 배열을 모두 초기화하는 함수
{                                                     // 함수 본문 시작
    int y;                                            // 행 반복용 변수
    int x;                                            // 열 반복용 변수

    for (y = 0; y < BOARD_SIZE; y++) {                // 모든 행을 반복
        for (x = 0; x < BOARD_SIZE; x++) {            // 각 행의 모든 열을 반복
            itemBoard[y][x] = EMPTY;                  // 아이템 보드를 빈칸으로 초기화
            fixedBoard[y][x] = EMPTY;                 // 고정 블록 보드도 빈칸으로 초기화
        }                                             // 안쪽 for 종료
    }                                                 // 바깥쪽 for 종료
}                                                     // 함수 끝

// ------------------------------------------------------------
// 랜덤 아이템 60개 생성
// 이미 차 있는 칸은 피해가며 itemBoard에만 생성
// ------------------------------------------------------------
void CreateInitialItems(void)                         // 처음 아이템 60개를 만드는 함수
{                                                     // 함수 본문 시작
    int created = 0;                                  // 지금까지 생성된 아이템 수를 저장할 변수

    while (created < INITIAL_ITEM_COUNT) {            // 60개가 만들어질 때까지 반복
        int x = rand() % BOARD_SIZE;                  // 0~29 중 랜덤 x 좌표 생성
        int y = rand() % BOARD_SIZE;                  // 0~29 중 랜덤 y 좌표 생성

        if (itemBoard[y][x] == EMPTY && fixedBoard[y][x] == EMPTY) { // 그 칸이 완전히 비어 있으면
            itemBoard[y][x] = 1 + rand() % 4;        // 1~4 중 랜덤 색상을 itemBoard에 저장
            created++;                                // 생성 개수 1 증가
        }                                             // if 종료
    }                                                 // while 종료
}                                                     // 함수 끝

// ------------------------------------------------------------
// 새로운 주인공 원 생성
// 맨 위 가운데에서 시작
// ------------------------------------------------------------
void SpawnHero(void)                                  // 새 주인공 원을 만드는 함수
{                                                     // 함수 본문 시작
    groupCount = 1;                                   // 새 묶음은 주인공 한 칸만 가진 상태로 시작
    headIndex = 0;                                    // 첫 번째 칸이 주인공 원이 되도록 설정
    groupX[0] = BOARD_SIZE / 2;                       // 가로 중앙 칸에서 시작
    groupY[0] = 0;                                    // 맨 위 줄에서 시작
    groupColor[0] = 1 + rand() % 4;                   // 주인공 원의 색도 1~4 중 랜덤으로 설정
    rotationState = 0;                                // 회전 상태를 초기화
}                                                     // 함수 끝

// ------------------------------------------------------------
// 게임 전체를 리셋하는 함수
// ------------------------------------------------------------
void ResetGame(void)                                  // 게임을 처음 상태로 되돌리는 함수
{                                                     // 함수 본문 시작
    ClearBoards();                                    // itemBoard, fixedBoard를 모두 비움
    CreateInitialItems();                             // 랜덤 아이템 60개 생성
    SpawnHero();                                      // 새 주인공 생성
}                                                     // 함수 끝

// ------------------------------------------------------------
// 현재 묶음이 dx, dy만큼 이동 가능한지 검사
// - 보드 밖이면 이동 불가
// - fixedBoard에 이미 쌓인 블록이 있으면 이동 불가
// - itemBoard는 흡수 대상이므로 막지 않음
// ------------------------------------------------------------
int CanMoveGroup(int dx, int dy)                      // 묶음 전체가 이동 가능한지 검사하는 함수
{                                                     // 함수 본문 시작
    int i;                                            // 반복문 변수 선언

    for (i = 0; i < groupCount; i++) {                // 현재 묶음의 모든 칸을 검사
        int nx = groupX[i] + dx;                      // 이동 후의 x 좌표 계산
        int ny = groupY[i] + dy;                      // 이동 후의 y 좌표 계산

        if (nx < 0 || nx >= BOARD_SIZE || ny < 0 || ny >= BOARD_SIZE) { // 이동 후 보드 밖으로 나가면
            return 0;                                 // 이동할 수 없으므로 0 반환
        }                                             // if 종료

        if (fixedBoard[ny][nx] != EMPTY && IsInGroup(nx, ny) == 0) { // 그 칸에 이미 고정 블록이 있고, 자기 몸 안 칸도 아니면
            return 0;                                 // 이동할 수 없으므로 0 반환
        }                                             // if 종료
    }                                                 // for 종료

    return 1;                                         // 모든 칸이 안전하면 이동 가능이므로 1 반환
}                                                     // 함수 끝

// ------------------------------------------------------------
// 이동하려는 방향에 itemBoard 아이템이 있으면
// 그 아이템을 현재 묶음에 흡수하는 함수
// ------------------------------------------------------------
void AbsorbItemsOnNextStep(int dx, int dy)            // 다음 이동 칸에 있는 아이템을 흡수하는 함수
{                                                     // 함수 본문 시작
    int oldCount = groupCount;                        // 원래 묶음 개수를 저장
    int i;                                            // 반복문 변수 선언

    for (i = 0; i < oldCount; i++) {                  // 기존 묶음 칸들만 기준으로 검사
        int nx = groupX[i] + dx;                      // 다음 이동 위치 x 계산
        int ny = groupY[i] + dy;                      // 다음 이동 위치 y 계산

        if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE) { // 그 위치가 보드 안이면
            if (itemBoard[ny][nx] != EMPTY && IsInGroup(nx, ny) == 0) {  // itemBoard에 아이템이 있고 아직 묶음에 안 들어 있으면
                groupX[groupCount] = nx;              // 새 묶음 칸의 x 좌표 저장
                groupY[groupCount] = ny;              // 새 묶음 칸의 y 좌표 저장
                groupColor[groupCount] = itemBoard[ny][nx]; // 아이템 색상을 묶음 색상 배열에 저장
                itemBoard[ny][nx] = EMPTY;            // itemBoard에서는 제거해서 먹힌 상태로 만듦
                groupCount++;                          // 묶음 칸 수를 1 증가
            }                                         // 안쪽 if 종료
        }                                             // 바깥 if 종료
    }                                                 // for 종료
}                                                     // 함수 끝

// ------------------------------------------------------------
// 현재 묶음을 실제로 dx, dy만큼 이동시키는 함수
// ------------------------------------------------------------
void MoveGroup(int dx, int dy)                        // 묶음을 이동시키는 함수
{                                                     // 함수 본문 시작
    int i;                                            // 반복문 변수 선언

    AbsorbItemsOnNextStep(dx, dy);                    // 먼저 다음 칸에 있는 itemBoard 아이템을 흡수
    for (i = 0; i < groupCount; i++) {                // 현재 묶음 전체에 대해
        groupX[i] += dx;                              // x 좌표를 dx만큼 이동
        groupY[i] += dy;                              // y 좌표를 dy만큼 이동
    }                                                 // for 종료
}                                                     // 함수 끝

// ------------------------------------------------------------
// 현재 묶음 중 주인공을 제외한 꼬리만 fixedBoard에 고정
// 즉, 주인공 원은 바닥에 닿으면 사라지고 안 쌓임
// ------------------------------------------------------------
void FixTailToBoard(void)                             // 꼬리만 바닥에 고정하는 함수
{                                                     // 함수 본문 시작
    int i;                                            // 반복문 변수 선언

    for (i = 0; i < groupCount; i++) {                // 현재 묶음 전체를 검사
        if (i == headIndex) {                         // 현재 칸이 주인공 원이면
            continue;                                 // 주인공은 쌓지 않고 건너뜀
        }                                             // if 종료

        if (groupX[i] >= 0 && groupX[i] < BOARD_SIZE && groupY[i] >= 0 && groupY[i] < BOARD_SIZE) { // 좌표가 보드 안이면
            if (fixedBoard[groupY[i]][groupX[i]] == EMPTY) { // 그 칸이 비어 있을 때만
                fixedBoard[groupY[i]][groupX[i]] = groupColor[i]; // 꼬리 블록을 fixedBoard에 저장
            }                                         // 안쪽 if 종료
        }                                             // 바깥 if 종료
    }                                                 // for 종료
}                                                     // 함수 끝

// ------------------------------------------------------------
// fixedBoard에서 한 줄이 모두 같은 색이면 그 줄을 삭제
// 삭제 후 위쪽 줄을 아래로 한 칸씩 내림
// ------------------------------------------------------------
void DeleteSameColorLines(void)                       // 같은 색으로 꽉 찬 줄을 지우는 함수
{                                                     // 함수 본문 시작
    int y;                                            // 행 반복용 변수 선언

    for (y = 0; y < BOARD_SIZE; y++) {                // 모든 줄을 위에서 아래로 검사
        int x;                                        // 열 반복용 변수 선언
        int same = 1;                                 // 일단 같은 색 줄이라고 가정
        int color = fixedBoard[y][0];                 // 해당 줄 첫 칸의 색을 기준 색으로 저장

        if (color == EMPTY) {                         // 첫 칸이 빈칸이면
            continue;                                 // 이 줄은 절대 꽉 찬 같은 색 줄이 아니므로 넘어감
        }                                             // if 종료

        for (x = 1; x < BOARD_SIZE; x++) {            // 두 번째 칸부터 마지막 칸까지 검사
            if (fixedBoard[y][x] != color) {          // 하나라도 기준 색과 다르면
                same = 0;                             // 같은 색 줄이 아님
                break;                                // 더 검사할 필요 없으므로 반복 종료
            }                                         // if 종료
        }                                             // for 종료

        if (same == 1) {                              // 한 줄 전체가 같은 색이었다면
            int moveY;                                // 줄을 내릴 때 사용할 행 변수
            int moveX;                                // 줄을 내릴 때 사용할 열 변수

            for (moveY = y; moveY > 0; moveY--) {     // 현재 줄부터 1행까지 한 칸씩 아래로 복사
                for (moveX = 0; moveX < BOARD_SIZE; moveX++) { // 해당 줄의 모든 칸 복사
                    fixedBoard[moveY][moveX] = fixedBoard[moveY - 1][moveX]; // 윗줄을 현재 줄로 내림
                }                                     // 안쪽 for 종료
            }                                         // 바깥 for 종료

            for (moveX = 0; moveX < BOARD_SIZE; moveX++) { // 맨 윗줄은 새 빈칸으로 초기화
                fixedBoard[0][moveX] = EMPTY;         // 맨 윗줄 칸을 비움
            }                                         // for 종료

            y--;                                      // 줄이 내려왔으므로 같은 y를 한 번 더 검사
        }                                             // if 종료
    }                                                 // for 종료
}                                                     // 함수 끝

// ------------------------------------------------------------
// 한 칸 자동 낙하 처리
// - 아래로 갈 수 있으면 이동
// - 못 가면 꼬리만 고정하고 새 주인공 생성
// ------------------------------------------------------------
void DropOneStep(void)                                // 타이머마다 한 칸씩 아래로 내리는 함수
{                                                     // 함수 본문 시작
    if (CanMoveGroup(0, 1)) {                         // 아래로 한 칸 이동 가능하면
        MoveGroup(0, 1);                              // 현재 묶음을 아래로 이동
    }                                                 // if 종료
    else {                                            // 더 내려갈 수 없으면
        FixTailToBoard();                             // 주인공 제외 꼬리만 바닥에 고정
        DeleteSameColorLines();                       // 줄 삭제 규칙 검사
        SpawnHero();                                  // 새 주인공을 위에서 다시 생성
    }                                                 // else 종료
}                                                     // 함수 끝

// ------------------------------------------------------------
// 회전 가능 여부 검사
// 주인공 원(headIndex)을 중심으로 시계 방향 90도 회전
// - 보드 밖으로 나가면 회전 불가
// - fixedBoard와 겹치면 회전 불가
// - itemBoard는 흡수 대상이므로 회전 막지 않음
// ------------------------------------------------------------
int CanRotateGroup(void)                              // 묶음이 회전 가능한지 검사하는 함수
{                                                     // 함수 본문 시작
    int i;                                            // 반복문 변수 선언
    int hx = groupX[headIndex];                       // 주인공 원의 x 좌표 저장
    int hy = groupY[headIndex];                       // 주인공 원의 y 좌표 저장

    for (i = 0; i < groupCount; i++) {                // 현재 묶음의 모든 칸을 검사
        int relX = groupX[i] - hx;                    // 주인공 기준 상대 x 좌표 계산
        int relY = groupY[i] - hy;                    // 주인공 기준 상대 y 좌표 계산

        int rotX = -relY;                             // 시계 방향 90도 회전 후 상대 x
        int rotY = relX;                              // 시계 방향 90도 회전 후 상대 y

        int nx = hx + rotX;                           // 회전 후 실제 x 좌표
        int ny = hy + rotY;                           // 회전 후 실제 y 좌표

        if (nx < 0 || nx >= BOARD_SIZE || ny < 0 || ny >= BOARD_SIZE) { // 회전 후 보드 밖이면
            return 0;                                 // 회전 불가
        }                                             // if 종료

        if (fixedBoard[ny][nx] != EMPTY && IsInGroup(nx, ny) == 0) { // 회전 후 위치에 고정 블록이 있고 자기 몸 내부도 아니면
            return 0;                                 // 회전 불가
        }                                             // if 종료
    }                                                 // for 종료

    return 1;                                         // 모든 칸이 안전하면 회전 가능
}                                                     // 함수 끝

// ------------------------------------------------------------
// 실제 회전 수행
// 회전 전에 CanRotateGroup으로 먼저 검사
// 회전 후 그 위치에 itemBoard 아이템이 있으면 흡수
// ------------------------------------------------------------
void RotateGroupClockwise(void)                       // 현재 묶음을 시계 방향 90도 회전시키는 함수
{                                                     // 함수 본문 시작
    int i;                                            // 반복문 변수 선언
    int hx;                                           // 주인공 x 좌표 저장용 변수
    int hy;                                           // 주인공 y 좌표 저장용 변수

    if (CanRotateGroup() == 0) {                      // 회전 불가능하면
        return;                                       // 함수 종료
    }                                                 // if 종료

    hx = groupX[headIndex];                           // 주인공 x 좌표 저장
    hy = groupY[headIndex];                           // 주인공 y 좌표 저장

    for (i = 0; i < groupCount; i++) {                // 묶음의 모든 칸을 실제로 회전
        int relX = groupX[i] - hx;                    // 상대 x 좌표 계산
        int relY = groupY[i] - hy;                    // 상대 y 좌표 계산

        int rotX = -relY;                             // 회전 후 상대 x 좌표 계산
        int rotY = relX;                              // 회전 후 상대 y 좌표 계산

        groupX[i] = hx + rotX;                        // 회전 후 실제 x 좌표로 갱신
        groupY[i] = hy + rotY;                        // 회전 후 실제 y 좌표로 갱신
    }                                                 // for 종료

    for (i = 0; i < groupCount; i++) {                // 회전 결과 위치에 itemBoard 아이템이 있는지 검사
        int x = groupX[i];                            // 현재 칸 x 좌표 저장
        int y = groupY[i];                            // 현재 칸 y 좌표 저장

        if (itemBoard[y][x] != EMPTY && IsInGroup(x, y) == 1) { // 자기 칸 위치에 itemBoard 아이템이 있다면
            itemBoard[y][x] = EMPTY;                  // 그 칸의 아이템을 제거
        }                                             // if 종료
    }                                                 // for 종료

    rotationState = (rotationState + 1) % 4;          // 회전 상태를 0~3 범위에서 갱신
}                                                     // 함수 끝

// ------------------------------------------------------------
// 한 칸을 화면에 그리는 함수
// isCircle = 1 이면 원
// isCircle = 0 이면 사각형
// ------------------------------------------------------------
void DrawSingleCell(HDC hDC, int x, int y, COLORREF color, int isCircle) // 한 칸을 그리는 함수
{                                                     // 함수 본문 시작
    HBRUSH hBrush = CreateSolidBrush(color);          // 지정한 색상의 브러시 생성
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hDC, hBrush); // 기존 브러시를 저장하고 새 브러시 선택

    if (isCircle == 1) {                              // 원으로 그려야 하면
        Ellipse(hDC,                                  // 원(정확히는 타원) 그리기
            CellToPixelX(x) + 2,                      // 왼쪽 위 x 좌표
            CellToPixelY(y) + 2,                      // 왼쪽 위 y 좌표
            CellToPixelX(x) + CELL_SIZE - 2,          // 오른쪽 아래 x 좌표
            CellToPixelY(y) + CELL_SIZE - 2);         // 오른쪽 아래 y 좌표
    }                                                 // if 종료
    else {                                            // 사각형으로 그려야 하면
        Rectangle(hDC,                                // 사각형 그리기
            CellToPixelX(x) + 2,                      // 왼쪽 위 x 좌표
            CellToPixelY(y) + 2,                      // 왼쪽 위 y 좌표
            CellToPixelX(x) + CELL_SIZE - 2,          // 오른쪽 아래 x 좌표
            CellToPixelY(y) + CELL_SIZE - 2);         // 오른쪽 아래 y 좌표
    }                                                 // else 종료

    SelectObject(hDC, hOldBrush);                     // 원래 사용하던 브러시를 다시 복구
    DeleteObject(hBrush);                             // 새로 만든 브러시 객체 삭제
}                                                     // 함수 끝

// ------------------------------------------------------------
// 30x30 격자선을 그리는 함수
// ------------------------------------------------------------
void DrawGrid(HDC hDC)                                // 보드 격자를 그리는 함수
{                                                     // 함수 본문 시작
    int i;                                            // 반복문 변수 선언

    for (i = 0; i <= BOARD_SIZE; i++) {               // 0부터 30까지 선을 그림
        MoveToEx(hDC, BOARD_LEFT, BOARD_TOP + i * CELL_SIZE, NULL); // 가로선 시작점 이동
        LineTo(hDC, BOARD_LEFT + BOARD_SIZE * CELL_SIZE, BOARD_TOP + i * CELL_SIZE); // 가로선 끝까지 그림

        MoveToEx(hDC, BOARD_LEFT + i * CELL_SIZE, BOARD_TOP, NULL); // 세로선 시작점 이동
        LineTo(hDC, BOARD_LEFT + i * CELL_SIZE, BOARD_TOP + BOARD_SIZE * CELL_SIZE); // 세로선 끝까지 그림
    }                                                 // for 종료
}                                                     // 함수 끝

// ------------------------------------------------------------
// itemBoard에 있는 아이템들을 그리는 함수
// ------------------------------------------------------------
void DrawItemBoard(HDC hDC)                           // 아직 먹히지 않은 아이템을 그리는 함수
{                                                     // 함수 본문 시작
    int y;                                            // 행 반복용 변수
    int x;                                            // 열 반복용 변수

    for (y = 0; y < BOARD_SIZE; y++) {                // 모든 행 반복
        for (x = 0; x < BOARD_SIZE; x++) {            // 모든 열 반복
            if (itemBoard[y][x] != EMPTY) {           // 해당 칸에 아이템이 있으면
                DrawSingleCell(hDC, x, y, GetColorFromId(itemBoard[y][x]), 0); // 사각형으로 그림
            }                                         // if 종료
        }                                             // 안쪽 for 종료
    }                                                 // 바깥 for 종료
}                                                     // 함수 끝

// ------------------------------------------------------------
// fixedBoard에 쌓인 블록들을 그리는 함수
// ------------------------------------------------------------
void DrawFixedBoard(HDC hDC)                          // 바닥에 쌓인 블록을 그리는 함수
{                                                     // 함수 본문 시작
    int y;                                            // 행 반복용 변수
    int x;                                            // 열 반복용 변수

    for (y = 0; y < BOARD_SIZE; y++) {                // 모든 행 반복
        for (x = 0; x < BOARD_SIZE; x++) {            // 모든 열 반복
            if (fixedBoard[y][x] != EMPTY) {          // 해당 칸에 고정 블록이 있으면
                DrawSingleCell(hDC, x, y, GetColorFromId(fixedBoard[y][x]), 0); // 사각형으로 그림
            }                                         // if 종료
        }                                             // 안쪽 for 종료
    }                                                 // 바깥 for 종료
}                                                     // 함수 끝

// ------------------------------------------------------------
// 현재 떨어지는 묶음을 그리는 함수
// 주인공은 원, 꼬리는 사각형으로 그림
// ------------------------------------------------------------
void DrawCurrentGroup(HDC hDC)                        // 현재 움직이는 묶음을 그리는 함수
{                                                     // 함수 본문 시작
    int i;                                            // 반복문 변수 선언

    for (i = 0; i < groupCount; i++) {                // 현재 묶음의 모든 칸을 그림
        int isHead = 0;                               // 주인공 여부 저장 변수

        if (i == headIndex) {                         // 현재 칸이 주인공이면
            isHead = 1;                               // 원으로 그리기 위해 1 저장
        }                                             // if 종료

        DrawSingleCell(hDC, groupX[i], groupY[i], GetColorFromId(groupColor[i]), isHead); // 한 칸 그리기
    }                                                 // for 종료
}                                                     // 함수 끝

// ------------------------------------------------------------
// 오른쪽 설명 글자를 그리는 함수
// ------------------------------------------------------------
void DrawInfoText(HDC hDC)                            // 조작 설명을 출력하는 함수
{                                                     // 함수 본문 시작
    TCHAR text1[] = L"방향키 : 이동";                  // 안내 문구 1
    TCHAR text2[] = L"ENTER : 90도 회전";             // 안내 문구 2
    TCHAR text3[] = L"R : 리셋";                      // 안내 문구 3
    TCHAR text4[] = L"Q : 종료";                      // 안내 문구 4
    TCHAR text5[] = L"좌클릭 : 아이템 추가";           // 안내 문구 5
    TCHAR text6[] = L"우클릭 : 아이템 삭제";           // 안내 문구 6
    TCHAR text7[100];                                 // 현재 꼬리 수 표시용 문자열 버퍼

    wsprintf(text7, L"현재 꼬리 수 : %d", groupCount - 1); // 꼬리 개수를 문자열로 저장

    TextOut(hDC, BOARD_LEFT + BOARD_SIZE * CELL_SIZE + 30, 60, text1, lstrlen(text1));  // 문구 1 출력
    TextOut(hDC, BOARD_LEFT + BOARD_SIZE * CELL_SIZE + 30, 90, text2, lstrlen(text2));  // 문구 2 출력
    TextOut(hDC, BOARD_LEFT + BOARD_SIZE * CELL_SIZE + 30, 120, text3, lstrlen(text3)); // 문구 3 출력
    TextOut(hDC, BOARD_LEFT + BOARD_SIZE * CELL_SIZE + 30, 150, text4, lstrlen(text4)); // 문구 4 출력
    TextOut(hDC, BOARD_LEFT + BOARD_SIZE * CELL_SIZE + 30, 180, text5, lstrlen(text5)); // 문구 5 출력
    TextOut(hDC, BOARD_LEFT + BOARD_SIZE * CELL_SIZE + 30, 210, text6, lstrlen(text6)); // 문구 6 출력
    TextOut(hDC, BOARD_LEFT + BOARD_SIZE * CELL_SIZE + 30, 250, text7, lstrlen(text7)); // 꼬리 수 출력
}                                                     // 함수 끝

// ------------------------------------------------------------
// 화면 픽셀 좌표를 보드 칸 좌표로 변환하는 함수
// 성공하면 1, 실패하면 0 반환
// ------------------------------------------------------------
int ScreenToBoard(int sx, int sy, int* bx, int* by)  // 화면 좌표를 보드 좌표로 바꾸는 함수
{                                                     // 함수 본문 시작
    if (sx < BOARD_LEFT || sy < BOARD_TOP) {          // 보드 시작점보다 왼쪽 또는 위쪽이면
        return 0;                                     // 보드 밖이므로 실패
    }                                                 // if 종료

    *bx = (sx - BOARD_LEFT) / CELL_SIZE;              // 화면 x를 보드 x로 변환
    *by = (sy - BOARD_TOP) / CELL_SIZE;               // 화면 y를 보드 y로 변환

    if (*bx < 0 || *bx >= BOARD_SIZE || *by < 0 || *by >= BOARD_SIZE) { // 변환 결과가 범위를 벗어나면
        return 0;                                     // 실패 반환
    }                                                 // if 종료

    return 1;                                         // 정상적으로 보드 안 좌표로 변환되었으면 성공 반환
}                                                     // 함수 끝

// ------------------------------------------------------------
// 프로그램 시작점
// 기본 윈도우 생성 코드
// ------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow) // 프로그램 시작 함수
{                                                     // 함수 본문 시작
    HWND hWnd;                                        // 생성될 윈도우 핸들 변수
    MSG Message;                                      // 메시지 저장 구조체 변수
    WNDCLASSEX WndClass;                              // 윈도우 클래스 등록용 구조체 변수

    g_hInst = hInstance;                              // 현재 인스턴스 핸들을 전역 변수에 저장

    WndClass.cbSize = sizeof(WndClass);               // 구조체 크기 설정
    WndClass.style = CS_HREDRAW | CS_VREDRAW;         // 가로/세로 크기 변경 시 다시 그리기 설정
    WndClass.lpfnWndProc = (WNDPROC)WndProc;          // 메시지 처리 함수를 WndProc으로 연결
    WndClass.cbClsExtra = 0;                          // 클래스 추가 메모리 없음
    WndClass.cbWndExtra = 0;                          // 윈도우 추가 메모리 없음
    WndClass.hInstance = hInstance;                   // 인스턴스 핸들 저장
    WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION); // 기본 아이콘 사용
    WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);   // 기본 화살표 커서 사용
    WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); // 흰색 배경 브러시 사용
    WndClass.lpszMenuName = NULL;                     // 메뉴 사용 안 함
    WndClass.lpszClassName = lpszClass;               // 클래스 이름 설정
    WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION); // 작은 아이콘도 기본 아이콘 사용

    RegisterClassEx(&WndClass);                       // 위에서 설정한 클래스를 윈도우 시스템에 등록

    hWnd = CreateWindow(                              // 실제 윈도우를 생성
        lpszClass,                                    // 등록한 클래스 이름 사용
        lpszWindowName,                               // 창 제목 문자열 사용
        WS_OVERLAPPEDWINDOW,                          // 일반적인 윈도우 스타일 사용
        50,                                           // 창 시작 x 좌표
        50,                                           // 창 시작 y 좌표
        950,                                          // 창 너비
        720,                                          // 창 높이
        NULL,                                         // 부모 윈도우 없음
        (HMENU)NULL,                                  // 메뉴 없음
        hInstance,                                    // 인스턴스 핸들 전달
        NULL);                                        // 추가 데이터 없음

    ShowWindow(hWnd, nCmdShow);                       // 생성한 창을 화면에 표시
    UpdateWindow(hWnd);                               // 창을 한 번 즉시 갱신

    while (GetMessage(&Message, 0, 0, 0)) {           // 메시지 루프 시작
        TranslateMessage(&Message);                   // 키보드 메시지를 문자 메시지 등으로 변환
        DispatchMessage(&Message);                    // 메시지를 WndProc으로 전달
    }                                                 // while 종료

    return (int)Message.wParam;                       // 프로그램 종료 코드를 반환
}                                                     // 함수 끝

// ------------------------------------------------------------
// 메시지 처리 함수
// ------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) // 윈도우 메시지 처리 함수
{                                                     // 함수 본문 시작
    PAINTSTRUCT ps;                                   // WM_PAINT에서 사용할 구조체 변수
    HDC hDC;                                          // 그림을 그릴 때 사용할 DC 핸들 변수

    switch (iMessage) {                               // 메시지 종류에 따라 분기
    case WM_CREATE:                                   // 창이 처음 생성될 때
        srand((unsigned int)time(NULL));              // 랜덤 시드 초기화
        ResetGame();                                  // 게임 전체 초기화
        SetTimer(hWnd, TIMER_ID, TIMER_INTERVAL, NULL); // 자동 낙하 타이머 시작
        break;                                        // WM_CREATE 처리 종료

    case WM_TIMER:                                    // 타이머 메시지가 들어오면
        if (wParam == TIMER_ID) {                     // 우리가 설정한 타이머라면
            DropOneStep();                            // 한 칸 자동 낙하 처리
            InvalidateRect(hWnd, NULL, TRUE);         // 화면 전체 다시 그리기 요청
        }                                             // if 종료
        break;                                        // WM_TIMER 처리 종료

    case WM_KEYDOWN:                                  // 키보드가 눌렸을 때
        switch (wParam) {                             // 어떤 키가 눌렸는지 판별
        case VK_LEFT:                                 // 왼쪽 화살표 키이면
            if (CanMoveGroup(-1, 0)) {                // 왼쪽으로 이동 가능하면
                MoveGroup(-1, 0);                     // 묶음을 왼쪽으로 한 칸 이동
            }                                         // if 종료
            InvalidateRect(hWnd, NULL, TRUE);         // 다시 그리기 요청
            break;                                    // 내부 switch 종료

        case VK_RIGHT:                                // 오른쪽 화살표 키이면
            if (CanMoveGroup(1, 0)) {                 // 오른쪽으로 이동 가능하면
                MoveGroup(1, 0);                      // 묶음을 오른쪽으로 한 칸 이동
            }                                         // if 종료
            InvalidateRect(hWnd, NULL, TRUE);         // 다시 그리기 요청
            break;                                    // 내부 switch 종료

        case VK_UP:                                   // 위쪽 화살표 키이면
            if (CanMoveGroup(0, -1)) {                // 위로 이동 가능하면
                MoveGroup(0, -1);                     // 묶음을 위로 한 칸 이동
            }                                         // if 종료
            InvalidateRect(hWnd, NULL, TRUE);         // 다시 그리기 요청
            break;                                    // 내부 switch 종료

        case VK_DOWN:                                 // 아래쪽 화살표 키이면
            if (CanMoveGroup(0, 1)) {                 // 아래로 이동 가능하면
                MoveGroup(0, 1);                      // 묶음을 아래로 한 칸 이동
            }                                         // if 종료
            else {                                    // 더 내려갈 수 없으면
                FixTailToBoard();                     // 꼬리만 바닥에 고정
                DeleteSameColorLines();               // 줄 삭제 검사
                SpawnHero();                          // 새 주인공 생성
            }                                         // else 종료
            InvalidateRect(hWnd, NULL, TRUE);         // 다시 그리기 요청
            break;                                    // 내부 switch 종료

        case VK_RETURN:                               // Enter 키이면
            RotateGroupClockwise();                   // 묶음을 시계 방향 90도 회전
            InvalidateRect(hWnd, NULL, TRUE);         // 다시 그리기 요청
            break;                                    // 내부 switch 종료

        case 'R':                                     // 대문자 R이면
        case 'r':                                     // 소문자 r이면
            ResetGame();                              // 게임 전체를 리셋
            InvalidateRect(hWnd, NULL, TRUE);         // 다시 그리기 요청
            break;                                    // 내부 switch 종료

        case 'Q':                                     // 대문자 Q이면
        case 'q':                                     // 소문자 q이면
            DestroyWindow(hWnd);                      // 창을 닫아서 프로그램 종료
            break;                                    // 내부 switch 종료
        }                                             // 내부 switch 종료
        break;                                        // WM_KEYDOWN 처리 종료

    case WM_LBUTTONDOWN:                              // 마우스 왼쪽 버튼 클릭 시
    {                                             // 지역 변수 사용을 위한 블록 시작
        int bx;                                   // 변환된 보드 x 좌표 저장 변수
        int by;                                   // 변환된 보드 y 좌표 저장 변수

        if (ScreenToBoard(LOWORD(lParam), HIWORD(lParam), &bx, &by)) { // 클릭 위치가 보드 안이면
            if (itemBoard[by][bx] == EMPTY && fixedBoard[by][bx] == EMPTY && IsInGroup(bx, by) == 0) { // 완전히 빈칸이면
                itemBoard[by][bx] = 1 + rand() % 4; // itemBoard에 랜덤 색 아이템 생성
                InvalidateRect(hWnd, NULL, TRUE); // 다시 그리기 요청
            }                                     // if 종료
        }                                         // if 종료
    }                                             // 블록 종료
    break;                                        // WM_LBUTTONDOWN 처리 종료

    case WM_RBUTTONDOWN:                              // 마우스 오른쪽 버튼 클릭 시
    {                                             // 지역 변수 사용을 위한 블록 시작
        int bx;                                   // 보드 x 좌표 저장 변수
        int by;                                   // 보드 y 좌표 저장 변수

        if (ScreenToBoard(LOWORD(lParam), HIWORD(lParam), &bx, &by)) { // 클릭 위치가 보드 안이면
            if (itemBoard[by][bx] != EMPTY) {     // 아직 먹히지 않은 아이템이 있으면
                itemBoard[by][bx] = EMPTY;        // 그 아이템 삭제
                InvalidateRect(hWnd, NULL, TRUE); // 다시 그리기 요청
            }                                     // if 종료
            else if (fixedBoard[by][bx] != EMPTY) { // 고정 블록이 있으면
                fixedBoard[by][bx] = EMPTY;       // 그 블록 삭제
                InvalidateRect(hWnd, NULL, TRUE); // 다시 그리기 요청
            }                                     // else if 종료
        }                                         // if 종료
    }                                             // 블록 종료
    break;                                        // WM_RBUTTONDOWN 처리 종료

    case WM_PAINT:                                    // 화면을 다시 그려야 할 때
        hDC = BeginPaint(hWnd, &ps);                  // 그리기 시작
        DrawGrid(hDC);                                // 격자선 먼저 그림
        DrawItemBoard(hDC);                           // 먹지 않은 아이템 그리기
        DrawFixedBoard(hDC);                          // 바닥에 쌓인 블록 그리기
        DrawCurrentGroup(hDC);                        // 현재 떨어지는 묶음 그리기
        DrawInfoText(hDC);                            // 오른쪽 안내 글자 출력
        EndPaint(hWnd, &ps);                          // 그리기 종료
        break;                                        // WM_PAINT 처리 종료

    case WM_DESTROY:                                  // 창이 닫힐 때
        KillTimer(hWnd, TIMER_ID);                    // 타이머 제거
        PostQuitMessage(0);                           // 메시지 루프 종료 요청
        break;                                        // WM_DESTROY 처리 종료
    }                                                 // switch 종료

    return DefWindowProc(hWnd, iMessage, wParam, lParam); // 처리하지 않은 메시지는 운영체제 기본 처리에 맡김
}                                                     // 함수 끝