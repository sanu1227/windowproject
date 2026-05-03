#include <windows.h>        // 윈도우 프로그램을 만들기 위한 기본 헤더 파일입니다.
#include <tchar.h>          // TCHAR, LPCTSTR 같은 문자열 자료형을 사용하기 위한 헤더입니다.
#include <stdlib.h>         // rand, srand 함수를 사용하기 위한 헤더입니다.
#include <time.h>           // time 함수를 사용하기 위한 헤더입니다.

HINSTANCE g_hInst;          // 현재 실행 중인 프로그램의 인스턴스 핸들을 저장하는 전역 변수입니다.

LPCTSTR lpszClass = L"My Window Class";                 // 윈도우 클래스 이름입니다.
LPCTSTR lpszWindowName = L"Pie Finding Game";           // 윈도우 제목 표시줄에 나올 이름입니다.

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam); // 메시지 처리 함수 선언입니다.

// 메뉴 명령어 ID입니다.
#define ID_GAME_START  1001      // Game start 메뉴 ID입니다.
#define ID_GAME_END    1002      // Game end 메뉴 ID입니다.
#define ID_GAME_HINT   1003      // Hint 메뉴 ID입니다.
#define ID_GAME_SCORE  1004      // Score 메뉴 ID입니다.

// 타이머 ID입니다.
#define TIMER_HINT     2001      // 힌트를 잠깐 보여주기 위한 타이머 ID입니다.

// 보드 설정입니다.
#define BOARD_ROWS     10        // 보드의 행 개수입니다.
#define BOARD_COLS     10        // 보드의 열 개수입니다.
#define CELL_SIZE      45        // 보드 한 칸의 크기입니다.
#define BOARD_LEFT     50        // 보드가 시작되는 왼쪽 x좌표입니다.
#define BOARD_TOP      50        // 보드가 시작되는 위쪽 y좌표입니다.

// 보드 종류입니다.
#define CELL_EMPTY     0         // 빈 칸입니다.
#define CELL_MINE      1         // 지뢰 칸입니다.
#define CELL_ITEM      2         // 아이템 칸입니다.
#define CELL_PIE       3         // 파이 조각 칸입니다.

// 게임 데이터 개수입니다.
#define MINE_COUNT     20        // 지뢰 개수입니다.
#define ITEM_COUNT     10        // 아이템 개수입니다.
#define PIE_SET_COUNT  5         // 파이 세트 개수입니다.
#define PIE_PIECES     4         // 한 파이는 4조각입니다.
#define PIE_COUNT      20        // 파이 조각 전체 개수입니다.

struct CELL_INFO {               // 보드 한 칸의 정보를 저장하는 구조체입니다.
    int type;                    // 이 칸이 빈 칸인지, 지뢰인지, 아이템인지, 파이인지 저장합니다.
    int opened;                  // 이 칸이 열렸는지 저장합니다. 0이면 닫힘, 1이면 열림입니다.
    int pieSet;                  // 파이 조각일 경우 몇 번째 파이 세트인지 저장합니다.
    int piePiece;                // 파이 조각일 경우 0, 1, 2, 3 중 어떤 조각인지 저장합니다.
};

CELL_INFO board[BOARD_ROWS][BOARD_COLS];        // 10x10 보드 정보를 저장하는 2차원 배열입니다.

COLORREF pieColor[PIE_SET_COUNT] = {            // 파이 세트별 색상입니다.
    RGB(255, 80, 80),                            // 0번 파이 세트 색상입니다.
    RGB(80, 180, 255),                           // 1번 파이 세트 색상입니다.
    RGB(80, 220, 120),                           // 2번 파이 세트 색상입니다.
    RGB(255, 200, 70),                           // 3번 파이 세트 색상입니다.
    RGB(180, 100, 255)                           // 4번 파이 세트 색상입니다.
};

int gameStarted = 0;              // 게임이 시작되었는지 저장합니다.
int gameOver = 0;                 // 게임이 끝났는지 저장합니다.
int hintMode = 0;                 // 힌트 상태인지 저장합니다.
int score = 0;                    // 완성한 파이 개수를 저장합니다.

int mouseDown = 0;                // 마우스 왼쪽 버튼을 누르고 있는지 저장합니다.
int dragStartRow = -1;            // 드래그 시작 행을 저장합니다.
int dragStartCol = -1;            // 드래그 시작 열을 저장합니다.
int dragEndRow = -1;              // 드래그 끝 행을 저장합니다.
int dragEndCol = -1;              // 드래그 끝 열을 저장합니다.

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
    HWND hWnd;                    // 생성될 윈도우의 핸들을 저장합니다.
    MSG Message;                  // 메시지 정보를 저장하는 구조체입니다.
    WNDCLASSEX WndClass;          // 윈도우 클래스 정보를 저장하는 구조체입니다.

    g_hInst = hInstance;          // 전역 인스턴스 핸들에 현재 프로그램 인스턴스를 저장합니다.

    WndClass.cbSize = sizeof(WndClass);                          // 구조체 크기를 저장합니다.
    WndClass.style = CS_HREDRAW | CS_VREDRAW;                    // 가로 또는 세로 크기가 바뀌면 다시 그리도록 설정합니다.
    WndClass.lpfnWndProc = (WNDPROC)WndProc;                     // 메시지 처리 함수를 등록합니다.
    WndClass.cbClsExtra = 0;                                     // 추가 클래스 메모리는 사용하지 않습니다.
    WndClass.cbWndExtra = 0;                                     // 추가 윈도우 메모리는 사용하지 않습니다.
    WndClass.hInstance = hInstance;                              // 현재 프로그램 인스턴스를 저장합니다.
    WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);            // 기본 아이콘을 사용합니다.
    WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);              // 기본 화살표 커서를 사용합니다.
    WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);// 배경색을 흰색으로 설정합니다.
    WndClass.lpszMenuName = NULL;                                // 메뉴는 코드에서 직접 만들기 때문에 NULL로 둡니다.
    WndClass.lpszClassName = lpszClass;                          // 윈도우 클래스 이름을 등록합니다.
    WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);          // 작은 아이콘도 기본 아이콘을 사용합니다.

    RegisterClassEx(&WndClass);                                  // 윈도우 클래스를 등록합니다.

    hWnd = CreateWindow(                                          // 실제 윈도우를 생성합니다.
        lpszClass,                                                // 사용할 윈도우 클래스 이름입니다.
        lpszWindowName,                                           // 창 제목입니다.
        WS_OVERLAPPEDWINDOW,                                      // 일반적인 윈도우 창 스타일입니다.
        100,                                                      // 창의 시작 x좌표입니다.
        100,                                                      // 창의 시작 y좌표입니다.
        700,                                                      // 창의 너비입니다.
        600,                                                      // 창의 높이입니다.
        NULL,                                                     // 부모 윈도우는 없습니다.
        (HMENU)NULL,                                              // 메뉴는 WM_CREATE에서 직접 만듭니다.
        hInstance,                                                // 현재 프로그램 인스턴스입니다.
        NULL                                                      // 추가 전달 데이터는 없습니다.
    );

    ShowWindow(hWnd, nCmdShow);                                   // 윈도우를 화면에 보여줍니다.
    UpdateWindow(hWnd);                                           // 윈도우를 즉시 갱신합니다.

    while (GetMessage(&Message, 0, 0, 0)) {                       // 메시지 큐에서 메시지를 계속 가져옵니다.
        TranslateMessage(&Message);                               // 키보드 메시지를 문자 메시지로 변환합니다.
        DispatchMessage(&Message);                                // 메시지를 WndProc 함수로 보냅니다.
    }

    return Message.wParam;                                        // 프로그램 종료 코드를 반환합니다.
}

void CreateGameMenu(HWND hWnd)
{
    HMENU hMenu;                       // 전체 메뉴를 저장할 핸들입니다.
    HMENU hGameMenu;                   // Game 메뉴를 저장할 핸들입니다.

    hMenu = CreateMenu();              // 메뉴 바를 생성합니다.
    hGameMenu = CreatePopupMenu();     // Game 아래에 들어갈 팝업 메뉴를 생성합니다.

    AppendMenu(hGameMenu, MF_STRING, ID_GAME_START, L"Game start"); // Game start 메뉴를 추가합니다.
    AppendMenu(hGameMenu, MF_STRING, ID_GAME_END, L"Game end");     // Game end 메뉴를 추가합니다.
    AppendMenu(hGameMenu, MF_STRING, ID_GAME_HINT, L"Hint");        // Hint 메뉴를 추가합니다.
    AppendMenu(hGameMenu, MF_STRING, ID_GAME_SCORE, L"Score");      // Score 메뉴를 추가합니다.

    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hGameMenu, L"Game");      // 메뉴 바에 Game 메뉴를 추가합니다.

    SetMenu(hWnd, hMenu);               // 만든 메뉴를 윈도우에 연결합니다.
}

void ResetBoard()
{
    int r;                              // 행 반복에 사용할 변수입니다.
    int c;                              // 열 반복에 사용할 변수입니다.
    int index;                          // 0부터 99까지의 칸 번호를 저장할 변수입니다.
    int temp;                           // 섞을 때 임시 저장할 변수입니다.
    int pos[100];                       // 0부터 99까지 칸 번호를 저장하고 섞을 배열입니다.
    int current;                        // 현재 배치할 위치 인덱스입니다.
    int row;                            // 칸 번호를 행 번호로 바꾼 값입니다.
    int col;                            // 칸 번호를 열 번호로 바꾼 값입니다.
    int set;                            // 파이 세트 번호입니다.
    int piece;                          // 파이 조각 번호입니다.

    for (r = 0; r < BOARD_ROWS; r++) {                                  // 모든 행을 반복합니다.
        for (c = 0; c < BOARD_COLS; c++) {                              // 모든 열을 반복합니다.
            board[r][c].type = CELL_EMPTY;                              // 처음에는 모두 빈 칸으로 설정합니다.
            board[r][c].opened = 0;                                     // 모든 칸을 닫힌 상태로 설정합니다.
            board[r][c].pieSet = -1;                                    // 파이 세트가 없다는 의미로 -1을 넣습니다.
            board[r][c].piePiece = -1;                                  // 파이 조각이 없다는 의미로 -1을 넣습니다.
        }
    }

    for (index = 0; index < 100; index++) {                              // 100개의 칸 번호를 준비합니다.
        pos[index] = index;                                              // pos 배열에 0부터 99까지 저장합니다.
    }

    for (index = 0; index < 100; index++) {                              // 칸 번호를 랜덤하게 섞습니다.
        int randomIndex = rand() % 100;                                  // 0부터 99 사이의 랜덤 위치를 고릅니다.
        temp = pos[index];                                               // 현재 값을 temp에 저장합니다.
        pos[index] = pos[randomIndex];                                   // 현재 위치에 랜덤 위치 값을 넣습니다.
        pos[randomIndex] = temp;                                         // 랜덤 위치에 temp 값을 넣습니다.
    }

    current = 0;                                                          // 첫 번째 랜덤 위치부터 배치합니다.

    for (index = 0; index < MINE_COUNT; index++) {                        // 지뢰 20개를 배치합니다.
        row = pos[current] / BOARD_COLS;                                  // 칸 번호를 행으로 바꿉니다.
        col = pos[current] % BOARD_COLS;                                  // 칸 번호를 열로 바꿉니다.
        board[row][col].type = CELL_MINE;                                 // 해당 칸을 지뢰로 설정합니다.
        current++;                                                        // 다음 랜덤 위치로 이동합니다.
    }

    for (index = 0; index < ITEM_COUNT; index++) {                        // 아이템 10개를 배치합니다.
        row = pos[current] / BOARD_COLS;                                  // 칸 번호를 행으로 바꿉니다.
        col = pos[current] % BOARD_COLS;                                  // 칸 번호를 열로 바꿉니다.
        board[row][col].type = CELL_ITEM;                                 // 해당 칸을 아이템으로 설정합니다.
        current++;                                                        // 다음 랜덤 위치로 이동합니다.
    }

    for (set = 0; set < PIE_SET_COUNT; set++) {                           // 파이 세트 5개를 반복합니다.
        for (piece = 0; piece < PIE_PIECES; piece++) {                    // 각 세트마다 4조각을 배치합니다.
            row = pos[current] / BOARD_COLS;                              // 칸 번호를 행으로 바꿉니다.
            col = pos[current] % BOARD_COLS;                              // 칸 번호를 열로 바꿉니다.
            board[row][col].type = CELL_PIE;                              // 해당 칸을 파이 조각으로 설정합니다.
            board[row][col].pieSet = set;                                 // 몇 번째 파이 세트인지 저장합니다.
            board[row][col].piePiece = piece;                             // 몇 번째 조각인지 저장합니다.
            current++;                                                    // 다음 랜덤 위치로 이동합니다.
        }
    }

    gameStarted = 1;                                                       // 게임 시작 상태로 변경합니다.
    gameOver = 0;                                                          // 게임 오버 상태를 해제합니다.
    hintMode = 0;                                                          // 힌트 모드를 끕니다.
    score = 0;                                                             // 점수를 0으로 초기화합니다.
}

RECT GetCellRect(int row, int col)
{
    RECT rc;                                                              // 칸의 사각형 좌표를 저장할 RECT 구조체입니다.

    rc.left = BOARD_LEFT + col * CELL_SIZE;                               // 칸의 왼쪽 좌표입니다.
    rc.top = BOARD_TOP + row * CELL_SIZE;                                 // 칸의 위쪽 좌표입니다.
    rc.right = rc.left + CELL_SIZE;                                       // 칸의 오른쪽 좌표입니다.
    rc.bottom = rc.top + CELL_SIZE;                                       // 칸의 아래쪽 좌표입니다.

    return rc;                                                            // 계산한 사각형 좌표를 반환합니다.
}

int GetRowFromY(int y)
{
    if (y < BOARD_TOP) {                                                   // y좌표가 보드 위쪽보다 작으면 보드 밖입니다.
        return -1;                                                         // 보드 밖이라는 의미로 -1을 반환합니다.
    }

    if (y >= BOARD_TOP + BOARD_ROWS * CELL_SIZE) {                         // y좌표가 보드 아래쪽보다 크거나 같으면 보드 밖입니다.
        return -1;                                                         // 보드 밖이라는 의미로 -1을 반환합니다.
    }

    return (y - BOARD_TOP) / CELL_SIZE;                                    // y좌표를 행 번호로 바꾸어 반환합니다.
}

int GetColFromX(int x)
{
    if (x < BOARD_LEFT) {                                                  // x좌표가 보드 왼쪽보다 작으면 보드 밖입니다.
        return -1;                                                         // 보드 밖이라는 의미로 -1을 반환합니다.
    }

    if (x >= BOARD_LEFT + BOARD_COLS * CELL_SIZE) {                        // x좌표가 보드 오른쪽보다 크거나 같으면 보드 밖입니다.
        return -1;                                                         // 보드 밖이라는 의미로 -1을 반환합니다.
    }

    return (x - BOARD_LEFT) / CELL_SIZE;                                   // x좌표를 열 번호로 바꾸어 반환합니다.
}

void CheckScore()
{
    int set;                                                               // 파이 세트 번호를 저장합니다.
    int r;                                                                 // 행 반복 변수입니다.
    int c;                                                                 // 열 반복 변수입니다.
    int openedPieces;                                                      // 해당 세트에서 열린 조각 개수를 저장합니다.
    int newScore;                                                          // 새로 계산한 점수를 저장합니다.

    newScore = 0;                                                          // 점수를 다시 계산하기 위해 0으로 초기화합니다.

    for (set = 0; set < PIE_SET_COUNT; set++) {                            // 파이 세트 5개를 모두 확인합니다.
        openedPieces = 0;                                                  // 현재 세트에서 열린 조각 개수를 0으로 초기화합니다.

        for (r = 0; r < BOARD_ROWS; r++) {                                 // 모든 행을 확인합니다.
            for (c = 0; c < BOARD_COLS; c++) {                             // 모든 열을 확인합니다.
                if (board[r][c].type == CELL_PIE &&                        // 현재 칸이 파이 조각이고,
                    board[r][c].pieSet == set &&                           // 현재 확인 중인 세트 번호와 같고,
                    board[r][c].opened == 1) {                             // 열린 상태라면,
                    openedPieces++;                                        // 열린 조각 개수를 1 증가시킵니다.
                }
            }
        }

        if (openedPieces == PIE_PIECES) {                                  // 한 세트의 4조각이 모두 열렸다면,
            newScore++;                                                    // 완성한 파이 개수를 1 증가시킵니다.
        }
    }

    score = newScore;                                                       // 계산한 점수를 전역 score에 저장합니다.
}

void OpenRemainingPiePieces()
{
    int set;                                                               // 파이 세트 번호입니다.
    int r;                                                                 // 행 반복 변수입니다.
    int c;                                                                 // 열 반복 변수입니다.
    int hasOpenedPiece[PIE_SET_COUNT];                                     // 각 세트가 이미 하나라도 열렸는지 저장합니다.

    for (set = 0; set < PIE_SET_COUNT; set++) {                            // 모든 파이 세트를 반복합니다.
        hasOpenedPiece[set] = 0;                                           // 처음에는 열린 조각이 없다고 설정합니다.
    }

    for (r = 0; r < BOARD_ROWS; r++) {                                     // 모든 행을 반복합니다.
        for (c = 0; c < BOARD_COLS; c++) {                                 // 모든 열을 반복합니다.
            if (board[r][c].type == CELL_PIE &&                            // 현재 칸이 파이 조각이고,
                board[r][c].opened == 1 &&                                 // 현재 칸이 이미 열려 있고,
                board[r][c].pieSet >= 0) {                                 // 정상적인 파이 세트 번호가 있다면,
                hasOpenedPiece[board[r][c].pieSet] = 1;                    // 해당 파이 세트는 열린 조각이 있다고 표시합니다.
            }
        }
    }

    for (r = 0; r < BOARD_ROWS; r++) {                                     // 모든 행을 반복합니다.
        for (c = 0; c < BOARD_COLS; c++) {                                 // 모든 열을 반복합니다.
            if (board[r][c].type == CELL_PIE &&                            // 현재 칸이 파이 조각이고,
                board[r][c].pieSet >= 0 &&                                 // 정상적인 파이 세트 번호가 있고,
                hasOpenedPiece[board[r][c].pieSet] == 1) {                 // 그 세트가 이미 하나라도 열려 있었다면,
                board[r][c].opened = 1;                                    // 같은 세트의 모든 파이 조각을 엽니다.
            }
        }
    }

    CheckScore();                                                          // 파이 조각이 더 열렸으므로 점수를 다시 계산합니다.
}

void OpenCell(HWND hWnd, int row, int col)
{
    if (gameStarted == 0) {                                                // 게임이 시작되지 않았다면,
        return;                                                            // 아무것도 하지 않습니다.
    }

    if (gameOver == 1) {                                                   // 게임이 끝난 상태라면,
        return;                                                            // 아무것도 하지 않습니다.
    }

    if (row < 0 || row >= BOARD_ROWS || col < 0 || col >= BOARD_COLS) {     // 보드 범위를 벗어난 칸이면,
        return;                                                            // 아무것도 하지 않습니다.
    }

    if (board[row][col].opened == 1) {                                     // 이미 열린 칸이면,
        return;                                                            // 다시 열 필요가 없으므로 종료합니다.
    }

    board[row][col].opened = 1;                                            // 해당 칸을 열린 상태로 바꿉니다.

    if (board[row][col].type == CELL_MINE) {                               // 열린 칸이 지뢰라면,
        gameOver = 1;                                                      // 게임 오버 상태로 바꿉니다.
        MessageBox(hWnd, L"지뢰를 눌렀습니다. 게임 종료!", L"Game Over", MB_OK); // 게임 오버 메시지를 출력합니다.
    }
    else if (board[row][col].type == CELL_ITEM) {                          // 열린 칸이 아이템이라면,
        OpenRemainingPiePieces();                                          // 이미 열린 파이와 같은 색상의 나머지 조각들을 모두 엽니다.
    }
    else if (board[row][col].type == CELL_PIE) {                           // 열린 칸이 파이 조각이라면,
        CheckScore();                                                      // 완성된 파이가 있는지 점수를 다시 계산합니다.
    }
}

void OpenDragArea(HWND hWnd, int row1, int col1, int row2, int col2)
{
    int startRow;                                                          // 드래그 영역의 시작 행입니다.
    int endRow;                                                            // 드래그 영역의 끝 행입니다.
    int startCol;                                                          // 드래그 영역의 시작 열입니다.
    int endCol;                                                            // 드래그 영역의 끝 열입니다.
    int r;                                                                 // 행 반복 변수입니다.
    int c;                                                                 // 열 반복 변수입니다.
    int temp;                                                              // 값을 바꾸기 위한 임시 변수입니다.

    if (row1 == -1 || col1 == -1 || row2 == -1 || col2 == -1) {             // 시작 또는 끝 좌표가 보드 밖이면,
        return;                                                            // 아무것도 하지 않습니다.
    }

    startRow = row1;                                                       // 시작 행을 저장합니다.
    endRow = row2;                                                         // 끝 행을 저장합니다.
    startCol = col1;                                                       // 시작 열을 저장합니다.
    endCol = col2;                                                         // 끝 열을 저장합니다.

    if (startRow > endRow) {                                               // 시작 행이 끝 행보다 크면,
        temp = startRow;                                                   // 두 값을 바꾸기 위해 temp에 저장합니다.
        startRow = endRow;                                                 // 작은 값을 시작 행으로 만듭니다.
        endRow = temp;                                                     // 큰 값을 끝 행으로 만듭니다.
    }

    if (startCol > endCol) {                                               // 시작 열이 끝 열보다 크면,
        temp = startCol;                                                   // 두 값을 바꾸기 위해 temp에 저장합니다.
        startCol = endCol;                                                 // 작은 값을 시작 열로 만듭니다.
        endCol = temp;                                                     // 큰 값을 끝 열로 만듭니다.
    }

    for (r = startRow; r <= endRow; r++) {                                 // 드래그 영역의 모든 행을 반복합니다.
        for (c = startCol; c <= endCol; c++) {                             // 드래그 영역의 모든 열을 반복합니다.
            OpenCell(hWnd, r, c);                                          // 해당 칸을 엽니다.
        }
    }
}

void DrawQuarterPie(HDC hDC, RECT rc, int piece, COLORREF color)
{
    HBRUSH hBrush;                                                         // 파이 조각을 칠할 브러시입니다.
    HBRUSH oldBrush;                                                       // 기존 브러시를 저장합니다.
    HPEN hPen;                                                             // 파이 조각 테두리를 그릴 펜입니다.
    HPEN oldPen;                                                           // 기존 펜을 저장합니다.
    POINT pt[3];                                                           // 파이 조각을 삼각형처럼 그리기 위한 점 배열입니다.
    int cx;                                                                // 칸 중앙 x좌표입니다.
    int cy;                                                                // 칸 중앙 y좌표입니다.
    int left;                                                              // 파이 영역 왼쪽 좌표입니다.
    int top;                                                               // 파이 영역 위쪽 좌표입니다.
    int right;                                                             // 파이 영역 오른쪽 좌표입니다.
    int bottom;                                                            // 파이 영역 아래쪽 좌표입니다.

    left = rc.left + 6;                                                    // 칸보다 조금 안쪽에서 파이를 그리기 위한 왼쪽 좌표입니다.
    top = rc.top + 6;                                                      // 칸보다 조금 안쪽에서 파이를 그리기 위한 위쪽 좌표입니다.
    right = rc.right - 6;                                                  // 칸보다 조금 안쪽에서 파이를 그리기 위한 오른쪽 좌표입니다.
    bottom = rc.bottom - 6;                                                // 칸보다 조금 안쪽에서 파이를 그리기 위한 아래쪽 좌표입니다.

    cx = (left + right) / 2;                                               // 파이 중심 x좌표입니다.
    cy = (top + bottom) / 2;                                               // 파이 중심 y좌표입니다.

    hBrush = CreateSolidBrush(color);                                      // 파이 색상 브러시를 만듭니다.
    oldBrush = (HBRUSH)SelectObject(hDC, hBrush);                          // 새 브러시를 선택하고 기존 브러시를 저장합니다.

    hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));                            // 검은색 테두리 펜을 만듭니다.
    oldPen = (HPEN)SelectObject(hDC, hPen);                                // 새 펜을 선택하고 기존 펜을 저장합니다.

    Ellipse(hDC, left, top, right, bottom);                                // 먼저 전체 원을 그립니다.

    SelectObject(hDC, oldBrush);                                           // 기존 브러시로 되돌립니다.
    DeleteObject(hBrush);                                                  // 전체 원에 사용한 브러시를 삭제합니다.

    hBrush = CreateSolidBrush(RGB(240, 240, 240));                         // 가릴 부분을 칠할 밝은 회색 브러시를 만듭니다.
    oldBrush = (HBRUSH)SelectObject(hDC, hBrush);                          // 밝은 회색 브러시를 선택합니다.

    if (piece != 0) {                                                       // 0번 조각이 아니라면 왼쪽 위 조각을 가립니다.
        pt[0].x = cx; pt[0].y = cy;                                        // 첫 번째 점은 중심입니다.
        pt[1].x = left; pt[1].y = top;                                     // 두 번째 점은 왼쪽 위입니다.
        pt[2].x = cx; pt[2].y = top;                                       // 세 번째 점은 위쪽 중앙입니다.
        Pie(hDC, left, top, right, bottom, cx, top, left, cy);             // 원의 왼쪽 위 1/4 부분을 가립니다.
    }

    if (piece != 1) {                                                       // 1번 조각이 아니라면 오른쪽 위 조각을 가립니다.
        Pie(hDC, left, top, right, bottom, right, cy, cx, top);            // 원의 오른쪽 위 1/4 부분을 가립니다.
    }

    if (piece != 2) {                                                       // 2번 조각이 아니라면 오른쪽 아래 조각을 가립니다.
        Pie(hDC, left, top, right, bottom, cx, bottom, right, cy);         // 원의 오른쪽 아래 1/4 부분을 가립니다.
    }

    if (piece != 3) {                                                       // 3번 조각이 아니라면 왼쪽 아래 조각을 가립니다.
        Pie(hDC, left, top, right, bottom, left, cy, cx, bottom);          // 원의 왼쪽 아래 1/4 부분을 가립니다.
    }

    SelectObject(hDC, oldBrush);                                           // 기존 브러시로 되돌립니다.
    SelectObject(hDC, oldPen);                                             // 기존 펜으로 되돌립니다.
    DeleteObject(hBrush);                                                  // 밝은 회색 브러시를 삭제합니다.
    DeleteObject(hPen);                                                    // 펜을 삭제합니다.
}

void DrawCellContent(HDC hDC, int row, int col, RECT rc)
{
    HBRUSH hBrush;                                                         // 칸 내부를 칠할 브러시입니다.
    HBRUSH oldBrush;                                                       // 기존 브러시를 저장합니다.
    HPEN hPen;                                                             // 테두리 펜입니다.
    HPEN oldPen;                                                           // 기존 펜을 저장합니다.
    int cx;                                                                // 칸 중앙 x좌표입니다.
    int cy;                                                                // 칸 중앙 y좌표입니다.

    cx = (rc.left + rc.right) / 2;                                         // 칸 중앙 x좌표를 계산합니다.
    cy = (rc.top + rc.bottom) / 2;                                         // 칸 중앙 y좌표를 계산합니다.

    if (board[row][col].type == CELL_EMPTY) {                              // 빈 칸이면,
        hBrush = CreateSolidBrush(RGB(230, 230, 230));                     // 열린 빈 칸 색상 브러시를 만듭니다.
        oldBrush = (HBRUSH)SelectObject(hDC, hBrush);                      // 브러시를 선택합니다.
        Rectangle(hDC, rc.left + 2, rc.top + 2, rc.right - 2, rc.bottom - 2); // 빈 칸을 회색으로 표시합니다.
        SelectObject(hDC, oldBrush);                                       // 기존 브러시로 되돌립니다.
        DeleteObject(hBrush);                                              // 브러시를 삭제합니다.
    }
    else if (board[row][col].type == CELL_MINE) {                          // 지뢰 칸이면,
        hBrush = CreateSolidBrush(RGB(30, 30, 30));                        // 검은색 브러시를 만듭니다.
        oldBrush = (HBRUSH)SelectObject(hDC, hBrush);                      // 브러시를 선택합니다.
        Ellipse(hDC, cx - 13, cy - 13, cx + 13, cy + 13);                  // 지뢰를 원으로 그립니다.
        SelectObject(hDC, oldBrush);                                       // 기존 브러시로 되돌립니다.
        DeleteObject(hBrush);                                              // 브러시를 삭제합니다.
        TextOut(hDC, cx - 5, cy - 8, L"!", 1);                             // 지뢰 표시 느낌으로 느낌표를 출력합니다.
    }
    else if (board[row][col].type == CELL_ITEM) {                          // 아이템 칸이면,
        hBrush = CreateSolidBrush(RGB(100, 220, 255));                     // 하늘색 브러시를 만듭니다.
        oldBrush = (HBRUSH)SelectObject(hDC, hBrush);                      // 브러시를 선택합니다.
        RoundRect(hDC, rc.left + 8, rc.top + 8, rc.right - 8, rc.bottom - 8, 8, 8); // 아이템을 둥근 사각형으로 그립니다.
        SelectObject(hDC, oldBrush);                                       // 기존 브러시로 되돌립니다.
        DeleteObject(hBrush);                                              // 브러시를 삭제합니다.
        TextOut(hDC, cx - 6, cy - 8, L"I", 1);                             // 아이템이라는 의미로 I를 출력합니다.
    }
    else if (board[row][col].type == CELL_PIE) {                           // 파이 조각 칸이면,
        DrawQuarterPie(hDC, rc, board[row][col].piePiece, pieColor[board[row][col].pieSet]); // 파이 1/4 조각을 그립니다.
    }
}

void DrawBoard(HDC hDC)
{
    int r;                                                                 // 행 반복 변수입니다.
    int c;                                                                 // 열 반복 변수입니다.
    RECT rc;                                                               // 각 칸의 사각형 좌표입니다.
    HBRUSH hBrush;                                                         // 닫힌 칸을 칠할 브러시입니다.
    HBRUSH oldBrush;                                                       // 기존 브러시를 저장합니다.
    HPEN hPen;                                                             // 칸 테두리 펜입니다.
    HPEN oldPen;                                                           // 기존 펜을 저장합니다.

    hPen = CreatePen(PS_SOLID, 1, RGB(80, 80, 80));                         // 보드 선을 그릴 회색 펜을 만듭니다.
    oldPen = (HPEN)SelectObject(hDC, hPen);                                // 새 펜을 선택하고 기존 펜을 저장합니다.

    for (r = 0; r < BOARD_ROWS; r++) {                                     // 모든 행을 반복합니다.
        for (c = 0; c < BOARD_COLS; c++) {                                 // 모든 열을 반복합니다.
            rc = GetCellRect(r, c);                                        // 현재 칸의 좌표를 가져옵니다.

            Rectangle(hDC, rc.left, rc.top, rc.right, rc.bottom);          // 보드 칸 테두리를 그립니다.

            if (board[r][c].opened == 1 || hintMode == 1) {                // 칸이 열렸거나 힌트 모드라면,
                DrawCellContent(hDC, r, c, rc);                            // 칸의 실제 내용을 그립니다.
            }
            else {                                                         // 닫힌 칸이라면,
                hBrush = CreateSolidBrush(RGB(120, 120, 120));             // 닫힌 칸 색상 브러시를 만듭니다.
                oldBrush = (HBRUSH)SelectObject(hDC, hBrush);              // 새 브러시를 선택합니다.
                Rectangle(hDC, rc.left + 2, rc.top + 2, rc.right - 2, rc.bottom - 2); // 닫힌 칸을 회색으로 칠합니다.
                SelectObject(hDC, oldBrush);                               // 기존 브러시로 되돌립니다.
                DeleteObject(hBrush);                                      // 브러시를 삭제합니다.
            }
        }
    }

    SelectObject(hDC, oldPen);                                             // 기존 펜으로 되돌립니다.
    DeleteObject(hPen);                                                    // 만든 펜을 삭제합니다.
}

void DrawDragRect(HDC hDC)
{
    RECT rc1;                                                              // 드래그 시작 칸 좌표입니다.
    RECT rc2;                                                              // 드래그 끝 칸 좌표입니다.
    RECT dragRc;                                                           // 드래그 영역 전체 좌표입니다.
    HPEN hPen;                                                             // 드래그 영역 테두리 펜입니다.
    HPEN oldPen;                                                           // 기존 펜입니다.
    HBRUSH oldBrush;                                                       // 기존 브러시입니다.

    if (mouseDown == 0) {                                                  // 마우스를 누르고 있지 않다면,
        return;                                                            // 드래그 영역을 그리지 않습니다.
    }

    if (dragStartRow == -1 || dragStartCol == -1 || dragEndRow == -1 || dragEndCol == -1) { // 좌표가 잘못되었다면,
        return;                                                            // 드래그 영역을 그리지 않습니다.
    }

    rc1 = GetCellRect(dragStartRow, dragStartCol);                         // 시작 칸 좌표를 구합니다.
    rc2 = GetCellRect(dragEndRow, dragEndCol);                             // 끝 칸 좌표를 구합니다.

    dragRc.left = min(rc1.left, rc2.left);                                 // 드래그 영역의 왼쪽 좌표입니다.
    dragRc.top = min(rc1.top, rc2.top);                                    // 드래그 영역의 위쪽 좌표입니다.
    dragRc.right = max(rc1.right, rc2.right);                              // 드래그 영역의 오른쪽 좌표입니다.
    dragRc.bottom = max(rc1.bottom, rc2.bottom);                           // 드래그 영역의 아래쪽 좌표입니다.

    hPen = CreatePen(PS_SOLID, 3, RGB(255, 0, 0));                          // 빨간색 굵은 펜을 만듭니다.
    oldPen = (HPEN)SelectObject(hDC, hPen);                                // 새 펜을 선택합니다.
    oldBrush = (HBRUSH)SelectObject(hDC, GetStockObject(NULL_BRUSH));       // 내부를 칠하지 않도록 NULL_BRUSH를 선택합니다.

    Rectangle(hDC, dragRc.left, dragRc.top, dragRc.right, dragRc.bottom);   // 드래그 영역 테두리를 그립니다.

    SelectObject(hDC, oldBrush);                                           // 기존 브러시로 되돌립니다.
    SelectObject(hDC, oldPen);                                             // 기존 펜으로 되돌립니다.
    DeleteObject(hPen);                                                    // 만든 펜을 삭제합니다.
}

void DrawInfo(HDC hDC)
{
    TCHAR text[100];                                                       // 출력할 문자열을 저장할 배열입니다.

    TextOut(hDC, 510, 60, L"파이찾기", 4);                                // 게임 제목을 출력합니다.
    TextOut(hDC, 510, 95, L"목표: 같은 색 파이 4조각 완성", 16);           // 게임 목표를 출력합니다.
    TextOut(hDC, 510, 130, L"닫힌 칸 클릭/드래그로 열기", 14);             // 조작 방법을 출력합니다.
    TextOut(hDC, 510, 165, L"지뢰: 게임 종료", 8);                        // 지뢰 설명을 출력합니다.
    TextOut(hDC, 510, 200, L"아이템: 열린 파이 세트 완성", 14);            // 아이템 설명을 출력합니다.

    wsprintf(text, L"Score: %d / 5", score);                              // 점수 문자열을 만듭니다.
    TextOut(hDC, 510, 250, text, lstrlen(text));                           // 점수를 출력합니다.

    if (gameStarted == 0) {                                                // 게임이 아직 시작되지 않았다면,
        TextOut(hDC, 510, 300, L"Game start를 누르세요.", 14);             // 시작 안내 문구를 출력합니다.
    }

    if (gameOver == 1) {                                                   // 게임 오버라면,
        TextOut(hDC, 510, 330, L"게임 종료 상태입니다.", 11);              // 게임 종료 문구를 출력합니다.
    }

    if (score == 5 && gameStarted == 1 && gameOver == 0) {                 // 모든 파이를 완성했다면,
        TextOut(hDC, 510, 365, L"모든 파이 완성!", 8);                     // 승리 문구를 출력합니다.
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;                                                        // WM_PAINT에서 그림 그리기 정보를 저장합니다.
    HDC hDC;                                                               // 그림을 그릴 DC 핸들입니다.
    int x;                                                                 // 마우스 x좌표입니다.
    int y;                                                                 // 마우스 y좌표입니다.
    int row;                                                               // 마우스 좌표를 보드 행으로 바꾼 값입니다.
    int col;                                                               // 마우스 좌표를 보드 열로 바꾼 값입니다.
    TCHAR scoreText[100];                                                  // Score 메뉴에서 출력할 문자열입니다.

    switch (uMsg) {                                                        // 들어온 메시지 종류에 따라 처리합니다.

    case WM_CREATE:                                                        // 윈도우가 생성될 때 실행됩니다.
        srand((unsigned int)time(NULL));                                   // 랜덤값이 매번 다르게 나오도록 초기화합니다.
        CreateGameMenu(hWnd);                                              // 게임 메뉴를 생성합니다.
        ResetBoard();                                                      // 처음 실행했을 때도 보드를 한 번 준비합니다.
        gameStarted = 0;                                                   // 하지만 메뉴의 Game start를 누르기 전까지는 시작하지 않은 상태로 둡니다.
        break;                                                             // WM_CREATE 처리를 끝냅니다.

    case WM_COMMAND:                                                       // 메뉴 명령어가 들어왔을 때 실행됩니다.
        switch (LOWORD(wParam)) {                                          // 어떤 메뉴가 눌렸는지 확인합니다.

        case ID_GAME_START:                                                // Game start 메뉴가 눌린 경우입니다.
            ResetBoard();                                                  // 보드를 새로 랜덤 배치합니다.
            InvalidateRect(hWnd, NULL, TRUE);                              // 화면을 다시 그리도록 요청합니다.
            break;                                                         // Game start 처리를 끝냅니다.

        case ID_GAME_END:                                                  // Game end 메뉴가 눌린 경우입니다.
            gameOver = 1;                                                  // 게임 종료 상태로 만듭니다.
            MessageBox(hWnd, L"게임을 종료했습니다.", L"Game end", MB_OK); // 종료 메시지를 보여줍니다.
            InvalidateRect(hWnd, NULL, TRUE);                              // 화면을 다시 그립니다.
            break;                                                         // Game end 처리를 끝냅니다.

        case ID_GAME_HINT:                                                 // Hint 메뉴가 눌린 경우입니다.
            if (gameStarted == 1 && gameOver == 0) {                       // 게임 중일 때만 힌트를 사용할 수 있습니다.
                hintMode = 1;                                              // 모든 칸이 보이도록 힌트 모드를 켭니다.
                SetTimer(hWnd, TIMER_HINT, 1500, NULL);                    // 1.5초 뒤 힌트 모드를 끄기 위한 타이머를 설정합니다.
                InvalidateRect(hWnd, NULL, TRUE);                          // 화면을 다시 그립니다.
            }
            break;                                                         // Hint 처리를 끝냅니다.

        case ID_GAME_SCORE:                                                // Score 메뉴가 눌린 경우입니다.
            wsprintf(scoreText, L"현재 완성한 파이 개수: %d개 / 5개", score); // 점수 메시지를 만듭니다.
            MessageBox(hWnd, scoreText, L"Score", MB_OK);                 // 점수 메시지를 출력합니다.
            break;                                                         // Score 처리를 끝냅니다.
        }
        break;                                                             // WM_COMMAND 처리를 끝냅니다.

    case WM_TIMER:                                                         // 타이머 메시지가 들어왔을 때 실행됩니다.
        if (wParam == TIMER_HINT) {                                        // 힌트 타이머라면,
            hintMode = 0;                                                  // 힌트 모드를 끕니다.
            KillTimer(hWnd, TIMER_HINT);                                  // 힌트 타이머를 제거합니다.
            InvalidateRect(hWnd, NULL, TRUE);                              // 화면을 다시 그립니다.
        }
        break;                                                             // WM_TIMER 처리를 끝냅니다.

    case WM_LBUTTONDOWN:                                                   // 마우스 왼쪽 버튼을 눌렀을 때 실행됩니다.
        x = LOWORD(lParam);                                                // 마우스 x좌표를 얻습니다.
        y = HIWORD(lParam);                                                // 마우스 y좌표를 얻습니다.
        row = GetRowFromY(y);                                              // y좌표를 보드 행으로 바꿉니다.
        col = GetColFromX(x);                                              // x좌표를 보드 열로 바꿉니다.

        if (row != -1 && col != -1) {                                      // 마우스가 보드 안에 있다면,
            mouseDown = 1;                                                 // 마우스를 누르는 중이라고 저장합니다.
            dragStartRow = row;                                            // 드래그 시작 행을 저장합니다.
            dragStartCol = col;                                            // 드래그 시작 열을 저장합니다.
            dragEndRow = row;                                              // 현재 드래그 끝 행도 시작 행과 같게 둡니다.
            dragEndCol = col;                                              // 현재 드래그 끝 열도 시작 열과 같게 둡니다.
        }
        break;                                                             // WM_LBUTTONDOWN 처리를 끝냅니다.

    case WM_MOUSEMOVE:                                                     // 마우스가 움직일 때 실행됩니다.
        if (mouseDown == 1) {                                              // 왼쪽 버튼을 누른 채 움직이는 중이라면,
            x = LOWORD(lParam);                                            // 현재 마우스 x좌표를 얻습니다.
            y = HIWORD(lParam);                                            // 현재 마우스 y좌표를 얻습니다.
            row = GetRowFromY(y);                                          // 현재 y좌표를 보드 행으로 바꿉니다.
            col = GetColFromX(x);                                          // 현재 x좌표를 보드 열로 바꿉니다.

            if (row != -1 && col != -1) {                                  // 현재 마우스가 보드 안에 있다면,
                dragEndRow = row;                                          // 드래그 끝 행을 갱신합니다.
                dragEndCol = col;                                          // 드래그 끝 열을 갱신합니다.
                InvalidateRect(hWnd, NULL, TRUE);                          // 드래그 표시를 다시 그립니다.
            }
        }
        break;                                                             // WM_MOUSEMOVE 처리를 끝냅니다.

    case WM_LBUTTONUP:                                                     // 마우스 왼쪽 버튼을 뗐을 때 실행됩니다.
        if (mouseDown == 1) {                                              // 이전에 마우스를 누른 상태였다면,
            x = LOWORD(lParam);                                            // 마우스를 뗀 x좌표를 얻습니다.
            y = HIWORD(lParam);                                            // 마우스를 뗀 y좌표를 얻습니다.
            row = GetRowFromY(y);                                          // y좌표를 보드 행으로 바꿉니다.
            col = GetColFromX(x);                                          // x좌표를 보드 열로 바꿉니다.

            if (row != -1 && col != -1) {                                  // 마우스를 보드 안에서 뗐다면,
                dragEndRow = row;                                          // 드래그 끝 행을 저장합니다.
                dragEndCol = col;                                          // 드래그 끝 열을 저장합니다.
                OpenDragArea(hWnd, dragStartRow, dragStartCol, dragEndRow, dragEndCol); // 드래그 영역의 칸들을 모두 엽니다.
            }

            mouseDown = 0;                                                 // 마우스 누름 상태를 해제합니다.
            dragStartRow = -1;                                             // 드래그 시작 행을 초기화합니다.
            dragStartCol = -1;                                             // 드래그 시작 열을 초기화합니다.
            dragEndRow = -1;                                               // 드래그 끝 행을 초기화합니다.
            dragEndCol = -1;                                               // 드래그 끝 열을 초기화합니다.

            InvalidateRect(hWnd, NULL, TRUE);                              // 화면을 다시 그립니다.
        }
        break;                                                             // WM_LBUTTONUP 처리를 끝냅니다.

    case WM_PAINT:                                                         // 화면을 다시 그려야 할 때 실행됩니다.
        hDC = BeginPaint(hWnd, &ps);                                       // 그림 그리기를 시작합니다.
        DrawBoard(hDC);                                                    // 보드를 그립니다.
        DrawDragRect(hDC);                                                 // 드래그 중이면 드래그 영역을 그립니다.
        DrawInfo(hDC);                                                     // 오른쪽 안내 문구와 점수를 그립니다.
        EndPaint(hWnd, &ps);                                               // 그림 그리기를 끝냅니다.
        break;                                                             // WM_PAINT 처리를 끝냅니다.

    case WM_DESTROY:                                                       // 윈도우가 종료될 때 실행됩니다.
        PostQuitMessage(0);                                                // 프로그램 종료 메시지를 보냅니다.
        break;                                                             // WM_DESTROY 처리를 끝냅니다.
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);                      // 직접 처리하지 않은 메시지는 기본 처리 함수로 넘깁니다.
}