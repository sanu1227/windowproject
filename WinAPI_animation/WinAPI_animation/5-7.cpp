#include <windows.h>      // 윈도우 프로그램을 만들기 위한 기본 헤더 파일
#include <tchar.h>        // TCHAR, LPCTSTR 등을 사용하기 위한 헤더 파일
#include <atlimage.h>     // CImage 클래스를 사용하기 위한 헤더 파일
#include <stdlib.h>       // rand(), srand()를 사용하기 위한 헤더 파일
#include <time.h>         // time()을 사용하기 위한 헤더 파일

// -----------------------------------------------------------------------------
// 기본 윈도우 프로그램에서 사용하는 전역 변수
// -----------------------------------------------------------------------------

HINSTANCE g_hInst;                                             // 현재 프로그램의 인스턴스 핸들
LPCTSTR lpszClass = L"My Window Class";                        // 윈도우 클래스 이름
LPCTSTR lpszWindowName = L"Jump Character Game";               // 윈도우 제목
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam); // 윈도우 메시지 처리 함수 선언

// -----------------------------------------------------------------------------
// 타이머 관련 상수
// -----------------------------------------------------------------------------

#define TIMER_GAME 1                                           // 게임 전체를 갱신하는 타이머 번호
#define TIMER_INTERVAL 40                                      // 40ms마다 갱신 = 약 25프레임

// -----------------------------------------------------------------------------
// 화면 배경 관련 상수
// -----------------------------------------------------------------------------

#define SKY_SCROLL_SPEED 8                                     // 하늘 배경 이동 속도
#define GROUND_SCROLL_SPEED 3                                  // 땅 배경 이동 속도

// -----------------------------------------------------------------------------
// 주인공 캐릭터 크기와 이동 관련 상수
// -----------------------------------------------------------------------------

#define PLAYER_WIDTH 150                                       // 일반 상태 주인공 너비
#define PLAYER_HEIGHT 180                                      // 일반 상태 주인공 높이
#define PLAYER_UNDER_HEIGHT 110                                // 엎드린 상태 주인공 높이
#define PLAYER_MOVE_STEP 20                                    // 좌우 키를 누를 때 한 번에 움직이는 거리

#define JUMP_POWER -28                                         // 점프 시작 순간의 위쪽 속도
#define GRAVITY 2                                              // 매 프레임 적용되는 중력 값

// -----------------------------------------------------------------------------
// 몬스터 크기 관련 상수
// -----------------------------------------------------------------------------

#define ATTACK_MONSTER_WIDTH 150                               // 공중 몬스터 너비
#define ATTACK_MONSTER_HEIGHT 130                              // 공중 몬스터 높이

#define SLIDING_MONSTER_WIDTH 170                              // 바닥 몬스터 너비
#define SLIDING_MONSTER_HEIGHT 100                             // 바닥 몬스터 높이

// -----------------------------------------------------------------------------
// 폭발 이미지 크기
// -----------------------------------------------------------------------------

#define BOMB_WIDTH 160                                         // 폭발 이미지 너비
#define BOMB_HEIGHT 160                                        // 폭발 이미지 높이

// -----------------------------------------------------------------------------
// 주인공 상태를 구분하기 위한 상수
// -----------------------------------------------------------------------------

#define PLAYER_RUN 0
#define PLAYER_JUMP 1
#define PLAYER_UNDER 2
#define PLAYER_BOMB 3

// -----------------------------------------------------------------------------
// 몬스터 종류를 구분하기 위한 상수
// -----------------------------------------------------------------------------

#define MONSTER_ATTACK 0                                       // 공중에서 오는 몬스터
#define MONSTER_SLIDING 1                                      // 바닥을 따라 오는 몬스터

// -----------------------------------------------------------------------------
// 이미지 객체 선언
// -----------------------------------------------------------------------------

CImage sky;                                                    // 하늘 배경 이미지
CImage ground;                                                 // 땅 배경 이미지

CImage attack[3];                                              // 몬스터1 공격 애니메이션 이미지 3장
CImage sliding[3];                                             // 몬스터2 슬라이딩 애니메이션 이미지 3장

CImage run[3];                                                 // 주인공 달리기 애니메이션 이미지 3장
CImage jump[3];                                                // 주인공 점프 애니메이션 이미지 3장
CImage under[3];                                               // 주인공 엎드리기 애니메이션 이미지 3장

CImage bomb[3];                                                // 폭발 애니메이션 이미지 3장

// -----------------------------------------------------------------------------
// 화면 크기 및 배경 스크롤 관련 변수
// -----------------------------------------------------------------------------

RECT rectView = { 0, 0, 0, 0 };                                // 현재 클라이언트 화면 크기
int groundTop = 0;                                             // 땅 배경이 시작되는 y좌표
int floorY = 0;                                                // 캐릭터 발이 닿는 바닥 y좌표

int skyOffset = 0;                                             // 하늘 배경 스크롤 위치
int groundOffset = 0;                                          // 땅 배경 스크롤 위치

// -----------------------------------------------------------------------------
// 애니메이션 공통 프레임 변수
// -----------------------------------------------------------------------------

int animationFrame = 0;                                        // 현재 애니메이션 프레임 번호 0~2
int animationTick = 0;                                         // 프레임 전환 속도를 조절하기 위한 카운터

// -----------------------------------------------------------------------------
// 주인공 관련 변수
// -----------------------------------------------------------------------------

int playerState = PLAYER_RUN;                                  // 현재 주인공 상태
int playerX = 0;                                               // 주인공 x좌표
int playerY = 0;                                               // 주인공 y좌표
int playerGroundY = 0;                                         // 땅 위에 서 있을 때의 y좌표
int playerUnderY = 0;                                          // 엎드렸을 때의 y좌표

int jumpVelocity = 0;                                          // 점프할 때 사용하는 세로 속도
int underRemainTick = 0;                                       // 엎드린 상태가 유지되는 시간

// -----------------------------------------------------------------------------
// 몬스터 관련 변수
// -----------------------------------------------------------------------------

int monsterType = MONSTER_ATTACK;                              // 현재 등장한 몬스터 종류
int monsterX = 0;                                              // 몬스터 x좌표
int monsterY = 0;                                              // 몬스터 y좌표
int monsterSpeed = 10;                                         // 몬스터 이동 속도

// -----------------------------------------------------------------------------
// 폭발 애니메이션 관련 변수
// -----------------------------------------------------------------------------

BOOL bombActive = FALSE;                                       // 현재 폭발 애니메이션이 실행 중인지 확인
int bombX = 0;                                                 // 폭발 이미지 x좌표
int bombY = 0;                                                 // 폭발 이미지 y좌표
int bombFrame = 0;                                             // 폭발 애니메이션 현재 프레임
int bombTick = 0;                                              // 폭발 프레임 전환 속도 조절용 카운터

// -----------------------------------------------------------------------------
// 함수 원형 선언
// -----------------------------------------------------------------------------

BOOL LoadGameImages(HWND hWnd);                                // 모든 이미지를 불러오는 함수
void UpdateLayout();                                           // 화면 크기에 맞춰 바닥과 캐릭터 위치를 계산하는 함수
void ResetMonster();                                           // 몬스터를 다시 왼쪽에서 등장시키는 함수
void UpdateGame(HWND hWnd);                                    // 게임 상태를 한 프레임 갱신하는 함수
void DrawGame(HDC hDC);                                        // 전체 게임 화면을 그리는 함수
void DrawBackground(HDC hDC);                                  // 하늘과 땅 배경을 그리는 함수
void DrawPlayer(HDC hDC);                                      // 주인공을 그리는 함수
void DrawMonster(HDC hDC);                                     // 몬스터를 그리는 함수
void DrawGuideText(HDC hDC);                                  // 조작 안내 문구를 그리는 함수
RECT GetPlayerRect();                                          // 주인공 충돌 판정 영역을 반환하는 함수
RECT GetMonsterRect();                                         // 몬스터 충돌 판정 영역을 반환하는 함수
BOOL IsCollision(RECT a, RECT b);                              // 두 사각형이 겹쳤는지 확인하는 함수

// -----------------------------------------------------------------------------
// WinMain 함수 : 프로그램의 시작점
// -----------------------------------------------------------------------------

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
    HWND hWnd;                                                  // 생성할 윈도우 핸들
    MSG Message;                                                // 윈도우 메시지를 저장할 구조체
    WNDCLASSEX WndClass;                                        // 윈도우 클래스 정보를 저장할 구조체

    g_hInst = hInstance;                                        // 현재 프로그램의 인스턴스 핸들을 전역 변수에 저장

    WndClass.cbSize = sizeof(WndClass);                         // WNDCLASSEX 구조체의 크기를 저장
    WndClass.style = CS_HREDRAW | CS_VREDRAW;                   // 가로 또는 세로 크기가 변하면 다시 그리도록 설정
    WndClass.lpfnWndProc = (WNDPROC)WndProc;                    // 메시지를 처리할 함수 연결
    WndClass.cbClsExtra = 0;                                    // 추가 클래스 메모리는 사용하지 않음
    WndClass.cbWndExtra = 0;                                    // 추가 윈도우 메모리는 사용하지 않음
    WndClass.hInstance = hInstance;                             // 현재 인스턴스 핸들을 등록
    WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);           // 기본 프로그램 아이콘 사용
    WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);             // 기본 화살표 마우스 커서 사용
    WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); // 기본 배경색은 흰색
    WndClass.lpszMenuName = NULL;                               // 메뉴는 사용하지 않음
    WndClass.lpszClassName = lpszClass;                         // 클래스 이름 지정
    WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);         // 작은 아이콘도 기본 아이콘 사용

    RegisterClassEx(&WndClass);                                 // 윈도우 클래스를 운영체제에 등록

    hWnd = CreateWindow(                                        // 실제 화면에 보일 윈도우 생성
        lpszClass,                                              // 등록한 윈도우 클래스 이름
        lpszWindowName,                                         // 윈도우 제목
        WS_OVERLAPPEDWINDOW,                                    // 일반적인 창 형태
        0,                                                      // 시작 x좌표
        0,                                                      // 시작 y좌표
        1500,                                                   // 창의 가로 크기
        650,                                                    // 창의 세로 크기
        NULL,                                                   // 부모 윈도우 없음
        (HMENU)NULL,                                            // 메뉴 없음
        hInstance,                                              // 인스턴스 핸들
        NULL                                                    // 추가 데이터 없음
    );

    ShowWindow(hWnd, nCmdShow);                                 // 윈도우를 화면에 표시
    UpdateWindow(hWnd);                                         // 처음 화면을 즉시 그림

    while (GetMessage(&Message, 0, 0, 0))                       // 종료 메시지가 올 때까지 반복
    {
        TranslateMessage(&Message);                             // 키보드 입력을 문자 메시지로 변환
        DispatchMessage(&Message);                              // 메시지를 WndProc으로 전달
    }

    return Message.wParam;                                      // 프로그램 종료 코드 반환
}

// -----------------------------------------------------------------------------
// 이미지 로드 함수
// -----------------------------------------------------------------------------

BOOL LoadGameImages(HWND hWnd)
{
    BOOL loadOK = TRUE;                                         // 모든 이미지가 잘 불러와졌는지 확인하는 변수

    if (FAILED(sky.Load(L"5-7/sky.png"))) loadOK = FALSE;       // 하늘 이미지 로드
    if (FAILED(ground.Load(L"5-7/ground.png"))) loadOK = FALSE; // 땅 이미지 로드

    if (FAILED(attack[0].Load(L"5-7/attack1.png"))) loadOK = FALSE; // 몬스터1 1프레임 로드
    if (FAILED(attack[1].Load(L"5-7/attack2.png"))) loadOK = FALSE; // 몬스터1 2프레임 로드
    if (FAILED(attack[2].Load(L"5-7/attack3.png"))) loadOK = FALSE; // 몬스터1 3프레임 로드

    if (FAILED(jump[0].Load(L"5-7/jump1.png"))) loadOK = FALSE; // 점프 1프레임 로드
    if (FAILED(jump[1].Load(L"5-7/jump2.png"))) loadOK = FALSE; // 점프 2프레임 로드
    if (FAILED(jump[2].Load(L"5-7/jump3.png"))) loadOK = FALSE; // 점프 3프레임 로드

    if (FAILED(run[0].Load(L"5-7/run1.png"))) loadOK = FALSE;   // 달리기 1프레임 로드
    if (FAILED(run[1].Load(L"5-7/run2.png"))) loadOK = FALSE;   // 달리기 2프레임 로드
    if (FAILED(run[2].Load(L"5-7/run3.png"))) loadOK = FALSE;   // 달리기 3프레임 로드

    if (FAILED(sliding[0].Load(L"5-7/sliding1.png"))) loadOK = FALSE; // 몬스터2 1프레임 로드
    if (FAILED(sliding[1].Load(L"5-7/sliding2.png"))) loadOK = FALSE; // 몬스터2 2프레임 로드
    if (FAILED(sliding[2].Load(L"5-7/sliding3.png"))) loadOK = FALSE; // 몬스터2 3프레임 로드

    if (FAILED(under[0].Load(L"5-7/under1.png"))) loadOK = FALSE; // 엎드리기 1프레임 로드
    if (FAILED(under[1].Load(L"5-7/under2.png"))) loadOK = FALSE; // 엎드리기 2프레임 로드
    if (FAILED(under[2].Load(L"5-7/under3.png"))) loadOK = FALSE; // 엎드리기 3프레임 로드

    if (FAILED(bomb[0].Load(L"5-7/bomb1.png"))) loadOK = FALSE; // 폭발 1프레임 로드
    if (FAILED(bomb[1].Load(L"5-7/bomb2.png"))) loadOK = FALSE; // 폭발 2프레임 로드
    if (FAILED(bomb[2].Load(L"5-7/bomb3.png"))) loadOK = FALSE; // 폭발 3프레임 로드

    if (loadOK == FALSE)                                        // 하나라도 불러오지 못했다면
    {
        MessageBox(                                             // 오류 메시지 출력
            hWnd,                                               // 부모 윈도우
            L"이미지를 불러오지 못했습니다.\n5-7 폴더와 파일명을 확인하세요.", // 메시지 내용
            L"이미지 로드 오류",                                // 메시지 제목
            MB_OK                                               // 확인 버튼만 표시
        );

        return FALSE;                                           // 실패 반환
    }

    return TRUE;                                                // 모든 이미지 로드 성공
}

// -----------------------------------------------------------------------------
// 화면 크기에 맞춰 주요 위치를 계산하는 함수
// -----------------------------------------------------------------------------

void UpdateLayout()
{
    // 캐릭터와 바닥 몬스터가 서 있을 기준선
    // 화면 높이의 약 84% 지점을 바닥 위치로 사용한다.
    // 올려준 ground.png의 도로 위치와 가장 자연스럽게 맞는다.
    floorY = (int)(rectView.bottom * 0.84);

    // 일반 상태 캐릭터의 y좌표
    playerGroundY = floorY - PLAYER_HEIGHT;

    // 엎드린 상태 캐릭터의 y좌표
    playerUnderY = floorY - PLAYER_UNDER_HEIGHT;

    // 달리기 상태일 때는 기본 바닥 위치에 맞춘다.
    if (playerState == PLAYER_RUN)
    {
        playerY = playerGroundY;
    }

    // 엎드린 상태일 때는 낮아진 높이에 맞춘다.
    if (playerState == PLAYER_UNDER)
    {
        playerY = playerUnderY;
    }

    // 주인공이 화면 왼쪽으로 나가지 않도록 제한
    if (playerX < 20)
    {
        playerX = 20;
    }

    // 주인공이 화면 오른쪽으로 나가지 않도록 제한
    if (playerX > rectView.right - PLAYER_WIDTH - 20)
    {
        playerX = rectView.right - PLAYER_WIDTH - 20;
    }
}

// -----------------------------------------------------------------------------
// 몬스터를 새로 등장시키는 함수
// -----------------------------------------------------------------------------

void ResetMonster()
{
    monsterType = rand() % 2;                                   // 0 또는 1을 랜덤으로 골라 몬스터 종류를 결정
    monsterSpeed = 9 + rand() % 5;                              // 몬스터 속도를 9~13 사이에서 랜덤 결정
    monsterX = -(220 + rand() % 300);                           // 화면 왼쪽 바깥에서 약간 랜덤한 거리로 시작

    if (monsterType == MONSTER_ATTACK)                          // 공중 몬스터라면
    {
        monsterY = floorY - PLAYER_HEIGHT - 60;                 // 캐릭터 머리와 몸통 높이 쪽으로 지나가게 배치
    }
    else                                                        // 바닥 몬스터라면
    {
        monsterY = floorY - SLIDING_MONSTER_HEIGHT;             // 바닥에 붙어서 지나가게 배치
    }
}

// -----------------------------------------------------------------------------
// 게임 상태를 한 프레임 갱신하는 함수
// -----------------------------------------------------------------------------

void UpdateGame(HWND hWnd)
{
    skyOffset += SKY_SCROLL_SPEED;                              // 하늘 배경을 오른쪽으로 조금 이동
    groundOffset += GROUND_SCROLL_SPEED;                        // 땅 배경을 오른쪽으로 조금 이동

    if (skyOffset >= rectView.right)                            // 하늘 배경 이동값이 화면 너비를 넘으면
    {
        skyOffset = 0;                                          // 처음 위치부터 다시 반복
    }

    if (groundOffset >= rectView.right)                         // 땅 배경 이동값이 화면 너비를 넘으면
    {
        groundOffset = 0;                                       // 처음 위치부터 다시 반복
    }

    animationTick++;                                           // 애니메이션 전환용 카운터 증가

    if (animationTick >= 3)                                     // 3번 갱신될 때마다
    {
        animationTick = 0;                                      // 카운터 초기화
        animationFrame++;                                       // 다음 애니메이션 프레임으로 이동

        if (animationFrame >= 3)                                // 프레임이 0,1,2를 넘어가면
        {
            animationFrame = 0;                                 // 다시 첫 프레임으로 돌아감
        }
    }

    if (playerState == PLAYER_JUMP)                             // 주인공이 점프 중이라면
    {
        playerY += jumpVelocity;                                // 현재 점프 속도만큼 y좌표 이동
        jumpVelocity += GRAVITY;                                // 중력을 적용해 아래쪽 속도를 점점 증가

        if (playerY >= playerGroundY)                           // 다시 바닥 위치까지 내려왔다면
        {
            playerY = playerGroundY;                            // 정확히 바닥 위치에 맞춤
            jumpVelocity = 0;                                   // 점프 속도 초기화
            playerState = PLAYER_RUN;                           // 다시 달리기 상태로 변경
        }
    }

    if (playerState == PLAYER_UNDER)                            // 주인공이 엎드린 상태라면
    {
        underRemainTick--;                                      // 엎드린 상태 유지 시간을 1 감소

        if (underRemainTick <= 0)                               // 유지 시간이 끝났다면
        {
            underRemainTick = 0;                                // 음수가 되지 않도록 0으로 보정
            playerState = PLAYER_RUN;                           // 달리기 상태로 복귀
            playerY = playerGroundY;                            // 캐릭터 높이도 원래 위치로 복귀
        }
    }

    monsterX += monsterSpeed;                                   // 몬스터를 오른쪽 방향으로 이동

    if (monsterX > rectView.right + 100)                        // 몬스터가 오른쪽 화면 밖으로 나갔다면
    {
        ResetMonster();                                         // 새로운 몬스터를 왼쪽에서 다시 등장시킴
    }

    RECT playerRect = GetPlayerRect();                          // 현재 주인공 충돌 영역 구하기
    RECT monsterRect = GetMonsterRect();                        // 현재 몬스터 충돌 영역 구하기

    if (playerState != PLAYER_BOMB &&
        IsCollision(playerRect, monsterRect) == TRUE)
    {
        int currentPlayerHeight = PLAYER_HEIGHT;

        // 엎드린 상태에서 충돌했다면
        // 현재 캐릭터 높이는 PLAYER_UNDER_HEIGHT를 사용한다.
        if (playerState == PLAYER_UNDER)
        {
            currentPlayerHeight = PLAYER_UNDER_HEIGHT;
        }

        // 주인공 상태를 폭발 상태로 변경한다.
        playerState = PLAYER_BOMB;

        // 폭발 애니메이션 시작
        bombActive = TRUE;
        bombFrame = 0;
        bombTick = 0;

        // 폭발 이미지가 기존 주인공 중심에 나오도록 위치를 잡는다.
        bombX = playerX + PLAYER_WIDTH / 2 - BOMB_WIDTH / 2;
        bombY = playerY + currentPlayerHeight / 2 - BOMB_HEIGHT / 2;

        // 부딪힌 몬스터는 제거하고 다음 몬스터를 준비한다.
        ResetMonster();
    }

    if (bombActive == TRUE)
    {
        bombTick++;

        if (bombTick >= 4)
        {
            bombTick = 0;
            bombFrame++;

            if (bombFrame >= 3)
            {
                bombFrame = 0;
                bombActive = FALSE;

                // 폭발 애니메이션이 끝나면
                // 주인공을 다시 기본 달리기 상태로 되돌린다.
                playerState = PLAYER_RUN;
                playerY = playerGroundY;
            }
        }
    }

    InvalidateRect(hWnd, NULL, FALSE);                          // 화면을 다시 그려 달라고 요청
}

// -----------------------------------------------------------------------------
// 배경을 그리는 함수
// -----------------------------------------------------------------------------

void DrawBackground(HDC hDC)
{
    int viewWidth = rectView.right;      // 현재 창의 가로 크기
    int viewHeight = rectView.bottom;    // 현재 창의 세로 크기

    // -----------------------------------------------------------------
    // 1. 하늘 배경
    // 화면 전체에 하늘을 그리고,
    // 두 장을 이어 붙여서 자연스럽게 스크롤되도록 한다.
    // -----------------------------------------------------------------

    sky.Draw(
        hDC,
        skyOffset - viewWidth,
        0,
        viewWidth,
        viewHeight
    );

    sky.Draw(
        hDC,
        skyOffset,
        0,
        viewWidth,
        viewHeight
    );

    // -----------------------------------------------------------------
    // 2. 땅 배경
    // ground.png는 위쪽이 투명한 이미지이므로
    // sky 위에 화면 전체 크기로 겹쳐 그려야 자연스럽다.
    // -----------------------------------------------------------------

    ground.Draw(
        hDC,
        groundOffset - viewWidth,
        0,
        viewWidth,
        viewHeight
    );

    ground.Draw(
        hDC,
        groundOffset,
        0,
        viewWidth,
        viewHeight
    );
}

// -----------------------------------------------------------------------------
// 주인공 캐릭터를 그리는 함수
// -----------------------------------------------------------------------------

void DrawPlayer(HDC hDC)
{
    if (playerState == PLAYER_RUN)
    {
        run[animationFrame].Draw(
            hDC,
            playerX,
            playerY,
            PLAYER_WIDTH,
            PLAYER_HEIGHT
        );
    }
    else if (playerState == PLAYER_JUMP)
    {
        jump[animationFrame].Draw(
            hDC,
            playerX,
            playerY,
            PLAYER_WIDTH,
            PLAYER_HEIGHT
        );
    }
    else if (playerState == PLAYER_UNDER)
    {
        under[animationFrame].Draw(
            hDC,
            playerX,
            playerY,
            PLAYER_WIDTH,
            PLAYER_UNDER_HEIGHT
        );
    }
    else if (playerState == PLAYER_BOMB)
    {
        // 충돌 중에는 주인공 이미지 대신 폭발 애니메이션을 출력한다.
        bomb[bombFrame].Draw(
            hDC,
            bombX,
            bombY,
            BOMB_WIDTH,
            BOMB_HEIGHT
        );
    }
}

// -----------------------------------------------------------------------------
// 몬스터를 그리는 함수
// -----------------------------------------------------------------------------

void DrawMonster(HDC hDC)
{
    if (monsterType == MONSTER_ATTACK)                          // 현재 몬스터가 공중 몬스터라면
    {
        attack[animationFrame].Draw(                            // attack 애니메이션 출력
            hDC,                                                 // 그릴 대상 HDC
            monsterX,                                            // x좌표
            monsterY,                                            // y좌표
            ATTACK_MONSTER_WIDTH,                                // 출력 너비
            ATTACK_MONSTER_HEIGHT                                // 출력 높이
        );
    }
    else                                                        // 현재 몬스터가 바닥 몬스터라면
    {
        sliding[animationFrame].Draw(                           // sliding 애니메이션 출력
            hDC,                                                 // 그릴 대상 HDC
            monsterX,                                            // x좌표
            monsterY,                                            // y좌표
            SLIDING_MONSTER_WIDTH,                               // 출력 너비
            SLIDING_MONSTER_HEIGHT                               // 출력 높이
        );
    }
}



// -----------------------------------------------------------------------------
// 안내 문구를 그리는 함수
// -----------------------------------------------------------------------------

void DrawGuideText(HDC hDC)
{
    SetBkMode(hDC, TRANSPARENT);                                // 글자 배경을 투명하게 설정
    SetTextColor(hDC, RGB(20, 20, 20));                         // 글자색을 어두운 회색으로 설정

    TextOut(                                                     // 첫 번째 안내 문구 출력
        hDC,                                                     // 그릴 대상 HDC
        20,                                                      // x좌표
        20,                                                      // y좌표
        L"← / → : 이동",                                         // 출력할 글자
        lstrlen(L"← / → : 이동")                                 // 글자 길이
    );

    TextOut(                                                     // 두 번째 안내 문구 출력
        hDC,                                                     // 그릴 대상 HDC
        20,                                                      // x좌표
        45,                                                      // y좌표
        L"j : 점프",                                             // 출력할 글자
        lstrlen(L"j : 점프")                                     // 글자 길이
    );

    TextOut(                                                     // 세 번째 안내 문구 출력
        hDC,                                                     // 그릴 대상 HDC
        20,                                                      // x좌표
        70,                                                      // y좌표
        L"Shift + J : 엎드리기",                                  // 출력할 글자
        lstrlen(L"Shift + J : 엎드리기")                          // 글자 길이
    );
}

// -----------------------------------------------------------------------------
// 전체 게임 화면을 그리는 함수
// -----------------------------------------------------------------------------

void DrawGame(HDC hDC)
{
    DrawBackground(hDC);                                        // 먼저 하늘과 땅 배경을 그림
    DrawMonster(hDC);                                           // 그 위에 몬스터를 그림
    DrawPlayer(hDC);                                            // 그 위에 주인공을 그림
    DrawGuideText(hDC);                                         // 마지막으로 조작 안내 문구를 그림
}

// -----------------------------------------------------------------------------
// 주인공 충돌 영역을 계산하는 함수
// -----------------------------------------------------------------------------

RECT GetPlayerRect()
{
    RECT r;

    // ------------------------------------------------------------
    // 엎드린 상태의 주인공 충돌 판정
    // 실제 이미지보다 판정 범위를 작게 잡아서
    // 공중 몬스터가 조금 스쳐도 맞지 않도록 완화한다.
    // ------------------------------------------------------------
    if (playerState == PLAYER_UNDER)
    {
        r.left = playerX + 40;                      // 왼쪽 판정을 안쪽으로 줄임
        r.top = playerY + 40;                       // 윗부분 판정을 크게 줄임
        r.right = playerX + PLAYER_WIDTH - 40;      // 오른쪽 판정을 안쪽으로 줄임
        r.bottom = playerY + PLAYER_UNDER_HEIGHT - 10; // 아래쪽은 약간만 줄임
    }

    // ------------------------------------------------------------
    // 달리기 / 점프 상태의 주인공 충돌 판정
    // 몸 전체가 아니라 중심 몸통 위주로 충돌하게 한다.
    // ------------------------------------------------------------
    else
    {
        r.left = playerX + 45;                      // 왼쪽 팔이나 머리카락 쪽 판정 제거
        r.top = playerY + 35;                       // 머리 윗부분 판정 제거
        r.right = playerX + PLAYER_WIDTH - 45;      // 오른쪽 여백 제거
        r.bottom = playerY + PLAYER_HEIGHT - 20;    // 발 아래쪽 판정 약간 제거
    }

    return r;
}

// -----------------------------------------------------------------------------
// 몬스터 충돌 영역을 계산하는 함수
// -----------------------------------------------------------------------------

RECT GetMonsterRect()
{
    RECT r;

    // ------------------------------------------------------------
    // 공중 몬스터 attack
    // 엎드리기로 피하기 쉬워지도록
    // 몬스터의 아래쪽 충돌 판정을 많이 줄인다.
    // ------------------------------------------------------------
    if (monsterType == MONSTER_ATTACK)
    {
        r.left = monsterX + 35;
        r.top = monsterY + 25;
        r.right = monsterX + ATTACK_MONSTER_WIDTH - 35;
        r.bottom = monsterY + ATTACK_MONSTER_HEIGHT - 35;
    }

    // ------------------------------------------------------------
    // 바닥 몬스터 sliding
    // 점프했을 때 더 잘 피할 수 있도록
    // 몬스터의 위쪽 충돌 판정을 줄인다.
    // ------------------------------------------------------------
    else
    {
        r.left = monsterX + 35;
        r.top = monsterY + 30;
        r.right = monsterX + SLIDING_MONSTER_WIDTH - 35;
        r.bottom = monsterY + SLIDING_MONSTER_HEIGHT - 15;
    }

    return r;
}

// -----------------------------------------------------------------------------
// 두 사각형이 충돌했는지 확인하는 함수
// -----------------------------------------------------------------------------

BOOL IsCollision(RECT a, RECT b)
{
    if (a.right < b.left) return FALSE;                         // 주인공이 몬스터보다 완전히 왼쪽이면 충돌 아님
    if (a.left > b.right) return FALSE;                         // 주인공이 몬스터보다 완전히 오른쪽이면 충돌 아님
    if (a.bottom < b.top) return FALSE;                         // 주인공이 몬스터보다 완전히 위쪽이면 충돌 아님
    if (a.top > b.bottom) return FALSE;                         // 주인공이 몬스터보다 완전히 아래쪽이면 충돌 아님

    return TRUE;                                                // 위 조건에 걸리지 않으면 서로 겹친 상태
}

// -----------------------------------------------------------------------------
// 윈도우 메시지를 처리하는 함수
// -----------------------------------------------------------------------------

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;                                             // WM_PAINT에서 사용하는 구조체
    HDC hDC;                                                    // 실제 윈도우에 그릴 때 사용하는 HDC

    switch (uMsg)                                               // 전달받은 메시지 종류에 따라 처리
    {
    case WM_CREATE:                                             // 윈도우가 처음 만들어질 때
        srand((unsigned int)time(NULL));                        // 매 실행마다 다른 랜덤 결과가 나오도록 초기화
        GetClientRect(hWnd, &rectView);                         // 현재 윈도우 내부 화면 크기 저장
        LoadGameImages(hWnd);                                   // 게임에서 사용할 모든 이미지 로드
        playerX = rectView.right - PLAYER_WIDTH - 100;          // 주인공은 화면 오른쪽 근처에서 시작
        UpdateLayout();                                         // 바닥 위치와 캐릭터 기본 위치 계산
        playerY = playerGroundY;                                // 주인공을 바닥 위에 배치
        ResetMonster();                                         // 첫 번째 몬스터를 랜덤하게 생성
        SetTimer(hWnd, TIMER_GAME, TIMER_INTERVAL, NULL);       // 게임 갱신 타이머 시작
        return 0;                                               // WM_CREATE 처리 완료

    case WM_SIZE:                                               // 윈도우 크기가 바뀔 때
        GetClientRect(hWnd, &rectView);                         // 변경된 화면 크기 다시 저장
        UpdateLayout();                                         // 바뀐 크기에 맞춰 위치 다시 계산
        return 0;                                               // WM_SIZE 처리 완료

    case WM_TIMER:                                              // 타이머가 울릴 때
        if (wParam == TIMER_GAME)                               // 게임 타이머라면
        {
            UpdateGame(hWnd);                                   // 게임을 한 프레임 진행
        }
        return 0;                                               // WM_TIMER 처리 완료

    case WM_KEYDOWN:                                            // 키보드 특수키가 눌렸을 때
        if (wParam == VK_LEFT)                                  // 왼쪽 방향키라면
        {
            playerX -= PLAYER_MOVE_STEP;                        // 주인공을 왼쪽으로 이동

            if (playerX < 20)                                   // 화면 밖으로 나가지 못하게 검사
            {
                playerX = 20;                                   // 왼쪽 경계 안으로 보정
            }
        }

        if (wParam == VK_RIGHT)                                 // 오른쪽 방향키라면
        {
            playerX += PLAYER_MOVE_STEP;                        // 주인공을 오른쪽으로 이동

            if (playerX > rectView.right - PLAYER_WIDTH - 20)   // 오른쪽 화면 밖으로 나가지 못하게 검사
            {
                playerX = rectView.right - PLAYER_WIDTH - 20;   // 오른쪽 경계 안으로 보정
            }
        }

        InvalidateRect(hWnd, NULL, FALSE);                      // 키 입력 결과를 바로 화면에 반영
        return 0;                                               // WM_KEYDOWN 처리 완료

    case WM_CHAR:                                               // 일반 문자 키가 입력되었을 때
        if (wParam == 'j')                                      // 소문자 j를 눌렀다면
        {
            if (playerState == PLAYER_RUN)                      // 달리는 중일 때만 점프 가능
            {
                playerState = PLAYER_JUMP;                      // 점프 상태로 전환
                jumpVelocity = JUMP_POWER;                      // 위로 튀어오르는 초기 속도 적용
            }
        }

        if (wParam == 'J')                                      // 대문자 J, 즉 Shift + J를 눌렀다면
        {
            if (playerState == PLAYER_RUN)                      // 달리는 중일 때만 엎드리기 가능
            {
                playerState = PLAYER_UNDER;                     // 엎드리기 상태로 전환
                playerY = playerUnderY;                         // 엎드린 높이에 맞춰 y좌표 변경
                underRemainTick = 22;                           // 일정 시간 동안 엎드린 상태 유지
            }
        }

        InvalidateRect(hWnd, NULL, FALSE);                      // 키 입력 후 즉시 화면 갱신 요청
        return 0;                                               // WM_CHAR 처리 완료

    case WM_PAINT:                                              // 화면을 다시 그려야 할 때
    {
        hDC = BeginPaint(hWnd, &ps);                            // 그림 그리기 시작

        if (rectView.right <= 0 || rectView.bottom <= 0)        // 화면 크기가 비정상적이면
        {
            EndPaint(hWnd, &ps);                                // 그림 그리기를 종료
            return 0;                                           // 처리 끝
        }

        HDC memDC = CreateCompatibleDC(hDC);                    // 더블 버퍼링용 메모리 DC 생성
        HBITMAP memBitmap = CreateCompatibleBitmap(             // 화면 크기만큼의 비트맵 생성
            hDC,                                                 // 기준 HDC
            rectView.right,                                     // 비트맵 너비
            rectView.bottom                                     // 비트맵 높이
        );

        HBITMAP oldBitmap = (HBITMAP)SelectObject(              // 메모리 DC에 비트맵 연결
            memDC,                                               // 메모리 DC
            memBitmap                                            // 새로 만든 비트맵
        );

        FillRect(                                                // 메모리 화면 전체를 흰색으로 초기화
            memDC,                                               // 메모리 DC
            &rectView,                                           // 칠할 영역
            (HBRUSH)GetStockObject(WHITE_BRUSH)                  // 흰색 브러시
        );

        DrawGame(memDC);                                        // 실제 게임 장면을 메모리 DC에 그림

        BitBlt(                                                  // 완성된 메모리 화면을 실제 창으로 복사
            hDC,                                                 // 목적지 HDC
            0,                                                   // 목적지 x좌표
            0,                                                   // 목적지 y좌표
            rectView.right,                                     // 복사 너비
            rectView.bottom,                                    // 복사 높이
            memDC,                                               // 원본 메모리 DC
            0,                                                   // 원본 x좌표
            0,                                                   // 원본 y좌표
            SRCCOPY                                              // 그대로 복사
        );

        SelectObject(memDC, oldBitmap);                         // 메모리 DC의 원래 비트맵 복구
        DeleteObject(memBitmap);                                // 직접 만든 비트맵 메모리 해제
        DeleteDC(memDC);                                        // 메모리 DC 해제

        EndPaint(hWnd, &ps);                                    // 그림 그리기 종료
        return 0;                                               // WM_PAINT 처리 완료
    }

    case WM_DESTROY:                                            // 창이 닫힐 때
        KillTimer(hWnd, TIMER_GAME);                            // 게임 타이머 정지
        PostQuitMessage(0);                                     // 프로그램 종료 메시지 전송
        return 0;                                               // WM_DESTROY 처리 완료
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);           // 처리하지 않은 메시지는 운영체제 기본 처리로 넘김
}