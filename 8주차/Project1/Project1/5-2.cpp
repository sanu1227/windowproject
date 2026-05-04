#include <windows.h>                         // Windows API 함수를 사용하기 위한 기본 헤더 파일입니다.
#include <tchar.h>                           // TCHAR, LPCTSTR 같은 문자 자료형을 사용하기 위한 헤더 파일입니다.
#include <stdlib.h>                          // rand, srand, abs 함수를 사용하기 위한 헤더 파일입니다.
#include <time.h>                            // time 함수를 사용해서 랜덤 시드를 만들기 위한 헤더 파일입니다.

HINSTANCE g_hInst;                           // 현재 프로그램의 인스턴스 핸들을 저장하는 전역 변수입니다.
LPCTSTR lpszClass = L"My Window Class";      // 윈도우 클래스 이름입니다.
LPCTSTR lpszWindowName = L"Piece Puzzle";    // 윈도우 제목 표시줄에 보일 이름입니다.

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam); // 메시지 처리 함수의 원형 선언입니다.

#define ID_IMAGE_1       1001                // 메뉴 명령 번호: 그림 1 선택입니다.
#define ID_IMAGE_2       1002                // 메뉴 명령 번호: 그림 2 선택입니다.
#define ID_SPLIT_3       2003                // 메뉴 명령 번호: 3등분 선택입니다.
#define ID_SPLIT_4       2004                // 메뉴 명령 번호: 4등분 선택입니다.
#define ID_SPLIT_5       2005                // 메뉴 명령 번호: 5등분 선택입니다.
#define ID_GAME_START    3001                // 메뉴 명령 번호: 게임 시작입니다.
#define ID_GAME_FULL     3002                // 메뉴 명령 번호: 전체 그림 보기입니다.
#define ID_GAME_END      3003                // 메뉴 명령 번호: 게임 종료입니다.

#define TIMER_ANIM       1                   // 퍼즐 조각이 미끄러지는 애니메이션에 사용할 타이머 번호입니다.
#define MAX_DIV          5                   // 최대 분할 개수입니다. 문제 조건이 3, 4, 5이므로 최대 5입니다.
#define MAX_CELL         25                  // 최대 칸 수입니다. 5x5이면 25칸입니다.
#define EMPTY_TILE      -1                   // 퍼즐 칸에 그림이 없다는 뜻으로 사용할 값입니다.
#define NO_REMOVED      -1                   // 오른쪽 클릭으로 사라진 그림이 아직 없다는 뜻으로 사용할 값입니다.

#define VIEW_FULL        0                   // 화면 상태: 전체 그림 보기 상태입니다.
#define VIEW_PUZZLE      1                   // 화면 상태: 퍼즐을 보여주는 상태입니다.
#define VIEW_END         2                   // 화면 상태: 게임 종료 상태입니다.

#define SPLIT_GRID       0                   // 분할 형태: 가로와 세로를 모두 나누는 일반 퍼즐입니다.
#define SPLIT_VERTICAL   1                   // 분할 형태: 세로 방향으로만 나누는 1xN 퍼즐입니다.
#define SPLIT_HORIZONTAL 2                   // 분할 형태: 가로 방향으로만 나누는 Nx1 퍼즐입니다.
#define DRAW_TEXT(hdc, x, y, text) TextOut(hdc, x, y, text, lstrlen(text)) // 문자열 길이를 자동으로 계산해서 출력하는 매크로입니다.

HBITMAP g_hBitmap[2];                        // image.bmp와 test.bmp를 저장할 비트맵 핸들 배열입니다.
BITMAP g_bmpInfo[2];                         // 각 비트맵의 가로, 세로 크기 정보를 저장하는 배열입니다.
int g_selectedImage = 0;                     // 현재 선택된 그림 번호입니다. 0이면 image.bmp, 1이면 test.bmp입니다.

int g_div = 3;                               // 현재 그림 나누기 개수입니다. 기본값은 3입니다.
int g_rows = 3;                              // 현재 퍼즐의 행 개수입니다.
int g_cols = 3;                              // 현재 퍼즐의 열 개수입니다.
int g_totalCells = 9;                        // 현재 퍼즐의 전체 칸 개수입니다.
int g_splitMode = SPLIT_GRID;                // 현재 분할 방식입니다. 기본값은 일반 3x3 퍼즐입니다.
int g_viewMode = VIEW_FULL;                  // 현재 화면 표시 상태입니다. 처음에는 전체 그림 보기 상태입니다.
int g_gameActive = 0;                        // 게임 조작 가능 여부입니다. 1이면 조작 가능, 0이면 조작 불가능입니다.

int g_board[MAX_DIV][MAX_DIV];               // 화면에 배치된 퍼즐 조각 번호를 저장하는 2차원 배열입니다.
int g_emptyR = -1;                           // 일반 퍼즐에서 빈 칸의 행 위치입니다.
int g_emptyC = -1;                           // 일반 퍼즐에서 빈 칸의 열 위치입니다.
int g_removedTile = NO_REMOVED;              // 오른쪽 클릭으로 사라진 그림 조각 번호를 저장합니다.

int g_mouseDown = 0;                         // 왼쪽 마우스 버튼을 누르고 있는지 저장합니다.
int g_downX = 0;                             // 왼쪽 마우스를 누른 시작 x좌표입니다.
int g_downY = 0;                             // 왼쪽 마우스를 누른 시작 y좌표입니다.
int g_downR = -1;                            // 왼쪽 마우스를 누른 시작 칸의 행 번호입니다.
int g_downC = -1;                            // 왼쪽 마우스를 누른 시작 칸의 열 번호입니다.

int g_animating = 0;                         // 현재 퍼즐 조각이 애니메이션으로 이동 중인지 저장합니다.
int g_animTile = EMPTY_TILE;                 // 애니메이션으로 이동 중인 퍼즐 조각 번호입니다.
int g_animSrcR = -1;                         // 이동 중인 조각의 출발 행 번호입니다.
int g_animSrcC = -1;                         // 이동 중인 조각의 출발 열 번호입니다.
int g_animDstR = -1;                         // 이동 중인 조각의 도착 행 번호입니다.
int g_animDstC = -1;                         // 이동 중인 조각의 도착 열 번호입니다.
int g_animStep = 0;                          // 현재 애니메이션 진행 단계입니다.
int g_animMaxStep = 12;                      // 애니메이션 전체 단계 수입니다. 값이 클수록 더 천천히 움직입니다.

void MakeMenu(HWND hWnd);                    // 메뉴를 만드는 함수의 원형 선언입니다.
void UpdateMenuCheck(HWND hWnd);             // 메뉴 체크 표시를 갱신하는 함수의 원형 선언입니다.
void MakeExeFolderPath(LPCTSTR fileName, TCHAR fullPath[MAX_PATH]) // 실행 파일이 있는 폴더 기준으로 BMP 파일 전체 경로를 만드는 함수입니다.
{
    int i;                                    // 문자열을 한 글자씩 검사할 때 사용할 반복 변수입니다.
    int lastSlash;                            // 실행 파일 경로에서 마지막 '\\' 위치를 저장할 변수입니다.

    GetModuleFileName(NULL, fullPath, MAX_PATH); // 현재 실행 중인 exe 파일의 전체 경로를 얻습니다.

    lastSlash = -1;                           // 아직 '\\'를 찾지 못했다는 뜻으로 -1을 넣습니다.

    for (i = 0; fullPath[i] != '\0'; i++) {   // exe 전체 경로 문자열의 끝까지 한 글자씩 검사합니다.
        if (fullPath[i] == L'\\' || fullPath[i] == L'/') { // 폴더 구분 문자 '\\' 또는 '/'를 찾았는지 확인합니다.
            lastSlash = i;                    // 마지막으로 찾은 폴더 구분 문자의 위치를 저장합니다.
        }
    }

    if (lastSlash >= 0) {                     // 폴더 구분 문자를 하나라도 찾았다면 처리합니다.
        fullPath[lastSlash + 1] = L'\0';      // exe 파일 이름 부분을 지우고 폴더 경로만 남깁니다.
    }
    else {                                    // 폴더 구분 문자를 찾지 못한 예외 상황입니다.
        fullPath[0] = L'\0';                  // 경로를 빈 문자열로 만듭니다.
    }

    lstrcat(fullPath, fileName);              // exe 폴더 경로 뒤에 image.bmp 또는 test.bmp 파일 이름을 붙입니다.
}

HBITMAP LoadOneBitmapFile(HWND hWnd, LPCTSTR fileName, BITMAP* bmpInfo) // BMP 파일 하나를 읽어오는 함수입니다.
{
    HBITMAP hBmp;                             // 읽어온 비트맵 핸들을 저장할 변수입니다.
    TCHAR fullPath[MAX_PATH];                 // 실행 파일 폴더 기준 전체 경로를 저장할 문자 배열입니다.

    hBmp = (HBITMAP)LoadImage(NULL, fileName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION); // 먼저 현재 작업 폴더에서 BMP 파일을 찾습니다.

    if (hBmp == NULL) {                       // 현재 작업 폴더에서 BMP를 읽지 못했다면 처리합니다.
        MakeExeFolderPath(fileName, fullPath); // 실행 파일이 있는 폴더 기준의 전체 경로를 만듭니다.
        hBmp = (HBITMAP)LoadImage(NULL, fullPath, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION); // exe 파일이 있는 폴더에서 BMP 파일을 다시 찾습니다.
    }

    if (hBmp != NULL) {                       // 둘 중 한 경로에서라도 BMP 파일을 정상적으로 읽었다면 처리합니다.
        GetObject(hBmp, sizeof(BITMAP), bmpInfo); // BMP의 가로, 세로 크기 정보를 구조체에 저장합니다.
    }

    return hBmp;                              // 읽은 비트맵 핸들을 반환합니다. 실패했다면 NULL이 반환됩니다.
}

void LoadPuzzleImages(HWND hWnd)             // image.bmp와 test.bmp 파일을 읽어오는 함수입니다.
{
    TCHAR path1[MAX_PATH];                    // image.bmp의 exe 폴더 기준 경로를 저장할 배열입니다.
    TCHAR path2[MAX_PATH];                    // test.bmp의 exe 폴더 기준 경로를 저장할 배열입니다.
    TCHAR msg[1024];                          // 이미지 로딩 실패 안내문을 저장할 배열입니다.

    g_hBitmap[0] = LoadOneBitmapFile(hWnd, L"image.bmp", &g_bmpInfo[0]); // image.bmp를 현재 폴더 또는 exe 폴더에서 읽어옵니다.
    g_hBitmap[1] = LoadOneBitmapFile(hWnd, L"test.bmp", &g_bmpInfo[1]);  // test.bmp를 현재 폴더 또는 exe 폴더에서 읽어옵니다.

    if (g_hBitmap[0] == NULL || g_hBitmap[1] == NULL) { // 둘 중 하나라도 읽지 못했다면 안내 메시지를 보여줍니다.
        MakeExeFolderPath(L"image.bmp", path1); // 실행 파일 폴더 기준 image.bmp 경로를 만듭니다.
        MakeExeFolderPath(L"test.bmp", path2);  // 실행 파일 폴더 기준 test.bmp 경로를 만듭니다.

        wsprintf(msg,                          // 사용자가 어디에 파일을 넣어야 하는지 확인할 수 있도록 안내문을 만듭니다.
            L"BMP 파일을 읽지 못했습니다.\\n\\n"
            L"아래 위치에 파일이 있는지 확인하세요.\\n\\n"
            L"%s\\n"
            L"%s\\n\\n"
            L"주의: 파일 이름은 image.bmp, test.bmp 이어야 하고, JPG/PNG가 아니라 진짜 BMP 형식이어야 합니다.",
            path1, path2);                    // 안내문에 실제 exe 폴더 기준 경로 2개를 넣습니다.

        MessageBox(hWnd, msg, L"이미지 로딩 오류", MB_OK); // 이미지 로딩 실패 메시지를 보여줍니다.
    }
}

void DeletePuzzleImages();                   // 읽어온 비트맵을 삭제하는 함수의 원형 선언입니다.
void StartGridGame(HWND hWnd);               // 일반 NxN 퍼즐을 시작하는 함수의 원형 선언입니다.
void StartStripGame(HWND hWnd, int mode);    // 가로 또는 세로 한 줄 퍼즐을 시작하는 함수의 원형 선언입니다.
void ShowFullImage(HWND hWnd);               // 전체 그림 보기 상태로 바꾸는 함수의 원형 선언입니다.
void EndGame(HWND hWnd);                     // 게임 종료 상태로 바꾸는 함수의 원형 선언입니다.
void ResetBoardToEmpty();                    // 퍼즐 배열을 모두 빈 칸으로 초기화하는 함수의 원형 선언입니다.
void ShuffleGridByLegalMove();               // 일반 퍼즐을 실제 이동 방식으로 섞는 함수의 원형 선언입니다.
void ShuffleStripBoard();                    // 한 줄 퍼즐의 순서를 섞는 함수의 원형 선언입니다.
void FindFirstEmpty();                       // 현재 배열에서 첫 번째 빈 칸을 찾는 함수의 원형 선언입니다.
int IsSolved();                              // 퍼즐이 맞춰졌는지 검사하는 함수의 원형 선언입니다.
void CheckSolvedAndFinish(HWND hWnd);         // 맞췄으면 메시지 박스를 띄우고 게임을 끝내는 함수의 원형 선언입니다.
void GetPuzzleRect(HWND hWnd, RECT* rc);     // 퍼즐이 그려질 화면 영역을 계산하는 함수의 원형 선언입니다.
void GetCellRect(HWND hWnd, int r, int c, RECT* rcCell); // 특정 칸의 화면 좌표를 계산하는 함수의 원형 선언입니다.
int GetCellFromPoint(HWND hWnd, int x, int y, int* r, int* c); // 마우스 좌표가 어느 칸인지 계산하는 함수의 원형 선언입니다.
void DrawScene(HWND hWnd, HDC hDC);          // 전체 화면을 그리는 함수의 원형 선언입니다.
void DrawTile(HDC hDC, HDC hImgDC, int tile, RECT rcDest); // 퍼즐 조각 하나를 그리는 함수의 원형 선언입니다.
void DrawBorderOnly(HDC hDC, int left, int top, int right, int bottom); // 사각형 내부를 덮지 않고 테두리만 그리는 함수의 원형 선언입니다.
void BeginMoveTile(HWND hWnd, int srcR, int srcC); // 조각 이동 애니메이션을 시작하는 함수의 원형 선언입니다.
void FinishMoveTile(HWND hWnd);              // 조각 이동 애니메이션을 끝내는 함수의 원형 선언입니다.
void ProcessDragMove(HWND hWnd, int dx, int dy); // 드래그 방향에 따라 빈 칸 주변 조각을 이동시키는 함수의 원형 선언입니다.
void SwapStripCells(HWND hWnd, int r1, int c1, int r2, int c2); // 한 줄 퍼즐의 두 칸을 바꾸는 함수의 원형 선언입니다.
void RemoveOrRestoreTile(HWND hWnd, int r, int c); // 오른쪽 클릭으로 조각 제거 또는 복구를 처리하는 함수의 원형 선언입니다.

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow) // Windows 프로그램의 시작 함수입니다.
{
    HWND hWnd;                               // 생성될 윈도우의 핸들을 저장할 변수입니다.
    MSG Message;                             // 메시지 루프에서 사용할 메시지 구조체 변수입니다.
    WNDCLASSEX WndClass;                     // 윈도우 클래스 정보를 저장할 구조체 변수입니다.

    g_hInst = hInstance;                     // 프로그램 인스턴스 핸들을 전역 변수에 저장합니다.
    srand((unsigned int)time(NULL));          // 현재 시간을 이용해서 랜덤 값이 매번 다르게 나오도록 설정합니다.

    WndClass.cbSize = sizeof(WndClass);       // WNDCLASSEX 구조체의 크기를 저장합니다.
    WndClass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS; // 크기 변경 시 다시 그리며 더블클릭 메시지도 받도록 설정합니다.
    WndClass.lpfnWndProc = (WNDPROC)WndProc; // 이 윈도우가 사용할 메시지 처리 함수를 지정합니다.
    WndClass.cbClsExtra = 0;                 // 추가 클래스 메모리를 사용하지 않으므로 0으로 설정합니다.
    WndClass.cbWndExtra = 0;                 // 추가 윈도우 메모리를 사용하지 않으므로 0으로 설정합니다.
    WndClass.hInstance = hInstance;          // 현재 프로그램 인스턴스를 저장합니다.
    WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION); // 기본 프로그램 아이콘을 사용합니다.
    WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);   // 기본 화살표 커서를 사용합니다.
    WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); // 기본 배경색을 흰색으로 설정합니다.
    WndClass.lpszMenuName = NULL;            // 메뉴는 코드에서 직접 만들 것이므로 NULL로 둡니다.
    WndClass.lpszClassName = lpszClass;      // 윈도우 클래스 이름을 지정합니다.
    WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION); // 작은 아이콘도 기본 아이콘으로 설정합니다.

    RegisterClassEx(&WndClass);              // 위에서 설정한 윈도우 클래스를 운영체제에 등록합니다.

    hWnd = CreateWindow(lpszClass, lpszWindowName, WS_OVERLAPPEDWINDOW, 100, 100, 900, 750, NULL, NULL, hInstance, NULL); // 실제 윈도우를 생성합니다.

    ShowWindow(hWnd, nCmdShow);              // 생성된 윈도우를 화면에 보여줍니다.
    UpdateWindow(hWnd);                      // WM_PAINT 메시지를 발생시켜 처음 화면을 그리게 합니다.

    while (GetMessage(&Message, 0, 0, 0)) {  // 프로그램이 종료되기 전까지 메시지를 계속 가져옵니다.
        TranslateMessage(&Message);          // 키보드 입력 메시지를 문자 메시지로 변환합니다.
        DispatchMessage(&Message);           // 메시지를 WndProc 함수로 전달합니다.
    }

    return (int)Message.wParam;              // 프로그램 종료 코드를 반환합니다.
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) // 윈도우 메시지를 처리하는 함수입니다.
{
    PAINTSTRUCT ps;                          // BeginPaint와 EndPaint에서 사용할 그리기 정보 구조체입니다.
    HDC hDC;                                 // 실제 화면 DC를 저장할 변수입니다.
    HDC hMemDC;                              // 더블 버퍼링용 메모리 DC를 저장할 변수입니다.
    HBITMAP hMemBmp;                         // 더블 버퍼링용 메모리 비트맵을 저장할 변수입니다.
    HBITMAP hOldBmp;                         // 메모리 DC에 원래 들어 있던 비트맵을 저장할 변수입니다.
    RECT rcClient;                           // 클라이언트 영역 크기를 저장할 변수입니다.
    int width;                               // 클라이언트 영역의 가로 크기입니다.
    int height;                              // 클라이언트 영역의 세로 크기입니다.
    int mx;                                  // 마우스 x좌표를 저장할 변수입니다.
    int my;                                  // 마우스 y좌표를 저장할 변수입니다.
    int r;                                   // 마우스가 위치한 칸의 행 번호를 저장할 변수입니다.
    int c;                                   // 마우스가 위치한 칸의 열 번호를 저장할 변수입니다.
    int dx;                                  // 드래그한 x방향 거리입니다.
    int dy;                                  // 드래그한 y방향 거리입니다.

    switch (uMsg) {                          // 어떤 메시지가 들어왔는지에 따라 처리할 내용을 나눕니다.
    case WM_CREATE:                          // 윈도우가 처음 만들어질 때 발생하는 메시지입니다.
        MakeMenu(hWnd);                      // 문제 조건에 맞는 메뉴를 생성합니다.
        LoadPuzzleImages(hWnd);              // image.bmp와 test.bmp 파일을 읽어옵니다.
        ResetBoardToEmpty();                 // 퍼즐 배열을 빈 상태로 초기화합니다.
        UpdateMenuCheck(hWnd);               // 현재 선택 상태에 맞게 메뉴 체크 표시를 갱신합니다.
        break;                               // WM_CREATE 처리를 끝냅니다.

    case WM_COMMAND:                         // 메뉴를 클릭했을 때 발생하는 메시지입니다.
        switch (LOWORD(wParam)) {            // 어떤 메뉴 명령이 선택되었는지 확인합니다.
        case ID_IMAGE_1:                     // 그림 1 메뉴를 선택한 경우입니다.
            g_selectedImage = 0;             // 현재 그림을 image.bmp로 설정합니다.
            ShowFullImage(hWnd);             // 그림을 바꾼 뒤 전체 그림 보기 상태로 전환합니다.
            break;                           // 그림 1 처리를 끝냅니다.
        case ID_IMAGE_2:                     // 그림 2 메뉴를 선택한 경우입니다.
            g_selectedImage = 1;             // 현재 그림을 test.bmp로 설정합니다.
            ShowFullImage(hWnd);             // 그림을 바꾼 뒤 전체 그림 보기 상태로 전환합니다.
            break;                           // 그림 2 처리를 끝냅니다.
        case ID_SPLIT_3:                     // 그림 나누기 3 메뉴를 선택한 경우입니다.
            g_div = 3;                       // 분할 개수를 3으로 설정합니다.
            UpdateMenuCheck(hWnd);           // 메뉴 체크 상태를 갱신합니다.
            InvalidateRect(hWnd, NULL, TRUE); // 화면을 다시 그리도록 요청합니다.
            break;                           // 3등분 처리를 끝냅니다.
        case ID_SPLIT_4:                     // 그림 나누기 4 메뉴를 선택한 경우입니다.
            g_div = 4;                       // 분할 개수를 4로 설정합니다.
            UpdateMenuCheck(hWnd);           // 메뉴 체크 상태를 갱신합니다.
            InvalidateRect(hWnd, NULL, TRUE); // 화면을 다시 그리도록 요청합니다.
            break;                           // 4등분 처리를 끝냅니다.
        case ID_SPLIT_5:                     // 그림 나누기 5 메뉴를 선택한 경우입니다.
            g_div = 5;                       // 분할 개수를 5로 설정합니다.
            UpdateMenuCheck(hWnd);           // 메뉴 체크 상태를 갱신합니다.
            InvalidateRect(hWnd, NULL, TRUE); // 화면을 다시 그리도록 요청합니다.
            break;                           // 5등분 처리를 끝냅니다.
        case ID_GAME_START:                  // 게임 시작 메뉴를 선택한 경우입니다.
            StartGridGame(hWnd);             // 현재 분할 개수에 맞는 NxN 퍼즐을 시작합니다.
            break;                           // 게임 시작 처리를 끝냅니다.
        case ID_GAME_FULL:                   // 전체 그림 보기 메뉴를 선택한 경우입니다.
            ShowFullImage(hWnd);             // 전체 그림 보기 상태로 전환합니다.
            break;                           // 전체 그림 보기 처리를 끝냅니다.
        case ID_GAME_END:                    // 게임 종료 메뉴를 선택한 경우입니다.
            EndGame(hWnd);                   // 게임 조작을 막고 종료 상태로 전환합니다.
            break;                           // 게임 종료 처리를 끝냅니다.
        }
        break;                               // WM_COMMAND 처리를 끝냅니다.

    case WM_CHAR:                            // 일반 문자 키가 눌렸을 때 발생하는 메시지입니다.
        if (wParam == '1') {                 // 1 키를 누르면 그림 1을 선택합니다.
            g_selectedImage = 0;             // 현재 그림을 image.bmp로 설정합니다.
            ShowFullImage(hWnd);             // 전체 그림 보기 상태로 전환합니다.
        }
        else if (wParam == '2') {            // 2 키를 누르면 그림 2를 선택합니다.
            g_selectedImage = 1;             // 현재 그림을 test.bmp로 설정합니다.
            ShowFullImage(hWnd);             // 전체 그림 보기 상태로 전환합니다.
        }
        else if (wParam == '3') {            // 3 키를 누르면 3등분으로 설정합니다.
            g_div = 3;                       // 분할 개수를 3으로 저장합니다.
            UpdateMenuCheck(hWnd);           // 메뉴 체크 표시를 갱신합니다.
            InvalidateRect(hWnd, NULL, TRUE); // 화면을 다시 그리도록 요청합니다.
        }
        else if (wParam == '4') {            // 4 키를 누르면 4등분으로 설정합니다.
            g_div = 4;                       // 분할 개수를 4로 저장합니다.
            UpdateMenuCheck(hWnd);           // 메뉴 체크 표시를 갱신합니다.
            InvalidateRect(hWnd, NULL, TRUE); // 화면을 다시 그리도록 요청합니다.
        }
        else if (wParam == '5') {            // 5 키를 누르면 5등분으로 설정합니다.
            g_div = 5;                       // 분할 개수를 5로 저장합니다.
            UpdateMenuCheck(hWnd);           // 메뉴 체크 표시를 갱신합니다.
            InvalidateRect(hWnd, NULL, TRUE); // 화면을 다시 그리도록 요청합니다.
        }
        else if (wParam == 's' || wParam == 'S') { // s 키를 누르면 게임을 시작합니다.
            StartGridGame(hWnd);             // NxN 일반 퍼즐을 시작합니다.
        }
        else if (wParam == 'f' || wParam == 'F') { // f 키를 누르면 전체 그림을 봅니다.
            ShowFullImage(hWnd);             // 전체 그림 보기 상태로 전환합니다.
        }
        else if (wParam == 'v' || wParam == 'V') { // v 키를 누르면 세로 방향 한 줄 퍼즐을 시작합니다.
            StartStripGame(hWnd, SPLIT_VERTICAL); // 1xN 형태로 그림을 나누어 섞습니다.
        }
        else if (wParam == 'h' || wParam == 'H') { // h 키를 누르면 가로 방향 한 줄 퍼즐을 시작합니다.
            StartStripGame(hWnd, SPLIT_HORIZONTAL); // Nx1 형태로 그림을 나누어 섞습니다.
        }
        else if (wParam == 'q' || wParam == 'Q') { // q 키를 누르면 게임을 종료합니다.
            EndGame(hWnd);                   // 조작을 막고 게임 종료 상태로 전환합니다.
        }
        break;                               // WM_CHAR 처리를 끝냅니다.

    case WM_LBUTTONDOWN:                     // 왼쪽 마우스 버튼을 눌렀을 때 발생하는 메시지입니다.
        if (g_gameActive == 1 && g_animating == 0) { // 게임 중이고 애니메이션 중이 아닐 때만 처리합니다.
            mx = LOWORD(lParam);             // lParam에서 마우스 x좌표를 꺼냅니다.
            my = HIWORD(lParam);             // lParam에서 마우스 y좌표를 꺼냅니다.
            if (GetCellFromPoint(hWnd, mx, my, &r, &c) == 1) { // 마우스 좌표가 퍼즐 칸 안에 있는지 확인합니다.
                g_mouseDown = 1;             // 왼쪽 마우스 버튼을 누른 상태로 저장합니다.
                g_downX = mx;                // 드래그 시작 x좌표를 저장합니다.
                g_downY = my;                // 드래그 시작 y좌표를 저장합니다.
                g_downR = r;                 // 드래그 시작 칸의 행 번호를 저장합니다.
                g_downC = c;                 // 드래그 시작 칸의 열 번호를 저장합니다.
            }
        }
        break;                               // WM_LBUTTONDOWN 처리를 끝냅니다.

    case WM_MOUSEMOVE:                       // 마우스가 움직일 때 발생하는 메시지입니다.
        if (g_gameActive == 1 && g_mouseDown == 1 && g_animating == 0 && g_splitMode == SPLIT_GRID) { // 일반 퍼즐에서 드래그 중일 때만 처리합니다.
            mx = LOWORD(lParam);             // 현재 마우스 x좌표를 꺼냅니다.
            my = HIWORD(lParam);             // 현재 마우스 y좌표를 꺼냅니다.
            dx = mx - g_downX;               // 처음 누른 위치에서 x방향으로 얼마나 움직였는지 계산합니다.
            dy = my - g_downY;               // 처음 누른 위치에서 y방향으로 얼마나 움직였는지 계산합니다.
            if (abs(dx) > 25 || abs(dy) > 25) { // 너무 작은 움직임은 실수로 보고, 25픽셀 이상 움직였을 때만 처리합니다.
                ProcessDragMove(hWnd, dx, dy); // 드래그 방향에 따라 빈 칸 주변 조각을 움직입니다.
                g_mouseDown = 0;             // 한 번 이동했으므로 드래그 상태를 해제합니다.
            }
        }
        break;                               // WM_MOUSEMOVE 처리를 끝냅니다.

    case WM_LBUTTONUP:                       // 왼쪽 마우스 버튼을 뗐을 때 발생하는 메시지입니다.
        if (g_gameActive == 1 && g_mouseDown == 1 && g_animating == 0 && g_splitMode != SPLIT_GRID) { // 한 줄 퍼즐에서 드래그가 끝났을 때 처리합니다.
            mx = LOWORD(lParam);             // 마우스를 놓은 x좌표를 꺼냅니다.
            my = HIWORD(lParam);             // 마우스를 놓은 y좌표를 꺼냅니다.
            if (GetCellFromPoint(hWnd, mx, my, &r, &c) == 1) { // 마우스를 놓은 위치가 퍼즐 칸인지 확인합니다.
                SwapStripCells(hWnd, g_downR, g_downC, r, c); // 시작 칸과 도착 칸의 그림을 서로 바꿉니다.
            }
        }
        g_mouseDown = 0;                     // 왼쪽 마우스 버튼을 뗐으므로 드래그 상태를 해제합니다.
        break;                               // WM_LBUTTONUP 처리를 끝냅니다.

    case WM_LBUTTONDBLCLK:                   // 왼쪽 마우스 버튼을 더블클릭했을 때 발생하는 메시지입니다.
        if (g_gameActive == 1 && g_animating == 0 && g_splitMode == SPLIT_GRID) { // 일반 퍼즐에서만 더블클릭 이동을 처리합니다.
            mx = LOWORD(lParam);             // 더블클릭한 x좌표를 꺼냅니다.
            my = HIWORD(lParam);             // 더블클릭한 y좌표를 꺼냅니다.
            if (GetCellFromPoint(hWnd, mx, my, &r, &c) == 1) { // 더블클릭한 곳이 퍼즐 칸인지 확인합니다.
                if (g_board[r][c] != EMPTY_TILE && g_emptyR != -1 && g_emptyC != -1) { // 그림이 있는 칸이고 빈 칸이 존재하는지 확인합니다.
                    BeginMoveTile(hWnd, r, c); // 더블클릭한 그림을 빈 칸으로 이동시킵니다.
                }
            }
        }
        break;                               // WM_LBUTTONDBLCLK 처리를 끝냅니다.

    case WM_RBUTTONDOWN:                     // 오른쪽 마우스 버튼을 클릭했을 때 발생하는 메시지입니다.
        if (g_gameActive == 1 && g_animating == 0) { // 게임 중이고 애니메이션 중이 아닐 때만 처리합니다.
            mx = LOWORD(lParam);             // 오른쪽 클릭한 x좌표를 꺼냅니다.
            my = HIWORD(lParam);             // 오른쪽 클릭한 y좌표를 꺼냅니다.
            if (GetCellFromPoint(hWnd, mx, my, &r, &c) == 1) { // 오른쪽 클릭한 곳이 퍼즐 칸인지 확인합니다.
                RemoveOrRestoreTile(hWnd, r, c); // 그림 제거 또는 복구를 처리합니다.
            }
        }
        break;                               // WM_RBUTTONDOWN 처리를 끝냅니다.

    case WM_TIMER:                           // 타이머가 울릴 때 발생하는 메시지입니다.
        if (wParam == TIMER_ANIM) {          // 퍼즐 조각 이동 애니메이션 타이머인지 확인합니다.
            g_animStep++;                    // 애니메이션 진행 단계를 1 증가시킵니다.
            if (g_animStep >= g_animMaxStep) { // 마지막 단계에 도착했는지 확인합니다.
                FinishMoveTile(hWnd);        // 애니메이션을 끝내고 배열 값을 확정합니다.
            }
            InvalidateRect(hWnd, NULL, FALSE); // 애니메이션 화면을 다시 그립니다.
        }
        break;                               // WM_TIMER 처리를 끝냅니다.

    case WM_ERASEBKGND:                     // 배경을 지우라는 메시지입니다.
        return 1;                            // 더블 버퍼링으로 직접 배경을 칠하므로 기본 흰색 지우기를 막습니다.

    case WM_PAINT:                           // 화면을 다시 그려야 할 때 발생하는 메시지입니다.
        hDC = BeginPaint(hWnd, &ps);          // 실제 화면에 그리기 시작합니다.
        GetClientRect(hWnd, &rcClient);       // 현재 클라이언트 영역 크기를 얻습니다.
        width = rcClient.right - rcClient.left; // 클라이언트 영역의 가로 길이를 계산합니다.
        height = rcClient.bottom - rcClient.top; // 클라이언트 영역의 세로 길이를 계산합니다.
        hMemDC = CreateCompatibleDC(hDC);     // 화면 DC와 호환되는 메모리 DC를 생성합니다.
        hMemBmp = CreateCompatibleBitmap(hDC, width, height); // 화면 크기와 같은 메모리 비트맵을 생성합니다.
        hOldBmp = (HBITMAP)SelectObject(hMemDC, hMemBmp); // 메모리 DC에 메모리 비트맵을 연결합니다.
        FillRect(hMemDC, &rcClient, (HBRUSH)GetStockObject(WHITE_BRUSH)); // 메모리 DC 배경을 흰색으로 지웁니다.
        DrawScene(hWnd, hMemDC);              // 메모리 DC에 전체 장면을 그립니다.
        BitBlt(hDC, 0, 0, width, height, hMemDC, 0, 0, SRCCOPY); // 완성된 메모리 화면을 실제 화면에 한 번에 복사합니다.
        SelectObject(hMemDC, hOldBmp);        // 메모리 DC에 원래 비트맵을 다시 연결합니다.
        DeleteObject(hMemBmp);                // 메모리 비트맵을 삭제하여 GDI 자원을 정리합니다.
        DeleteDC(hMemDC);                     // 메모리 DC를 삭제하여 GDI 자원을 정리합니다.
        EndPaint(hWnd, &ps);                  // 실제 화면 그리기를 끝냅니다.
        return 0;                            // WM_PAINT는 여기서 완전히 처리했으므로 운영체제 기본 처리로 넘기지 않습니다.

    case WM_DESTROY:                         // 윈도우가 파괴될 때 발생하는 메시지입니다.
        KillTimer(hWnd, TIMER_ANIM);          // 혹시 타이머가 켜져 있다면 꺼줍니다.
        DeletePuzzleImages();                 // 읽어온 비트맵 자원을 삭제합니다.
        PostQuitMessage(0);                  // 메시지 루프를 끝내서 프로그램을 종료합니다.
        break;                               // WM_DESTROY 처리를 끝냅니다.
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam); // 직접 처리하지 않은 메시지는 운영체제 기본 처리로 넘깁니다.
}

void MakeMenu(HWND hWnd)                     // 프로그램 상단 메뉴를 만드는 함수입니다.
{
    HMENU hMainMenu;                          // 전체 메뉴 막대 핸들입니다.
    HMENU hImageMenu;                         // 그림 선택 하위 메뉴 핸들입니다.
    HMENU hSplitMenu;                         // 그림 나누기 하위 메뉴 핸들입니다.
    HMENU hGameMenu;                          // 게임 하위 메뉴 핸들입니다.

    hMainMenu = CreateMenu();                 // 메뉴 막대 하나를 생성합니다.
    hImageMenu = CreatePopupMenu();           // 그림 메뉴에 들어갈 하위 메뉴를 생성합니다.
    hSplitMenu = CreatePopupMenu();           // 그림 나누기 메뉴에 들어갈 하위 메뉴를 생성합니다.
    hGameMenu = CreatePopupMenu();            // 게임 메뉴에 들어갈 하위 메뉴를 생성합니다.

    AppendMenu(hImageMenu, MF_STRING, ID_IMAGE_1, L"그림 1 - image.bmp"); // 그림 1 메뉴 항목을 추가합니다.
    AppendMenu(hImageMenu, MF_STRING, ID_IMAGE_2, L"그림 2 - test.bmp");  // 그림 2 메뉴 항목을 추가합니다.

    AppendMenu(hSplitMenu, MF_STRING, ID_SPLIT_3, L"3"); // 3등분 메뉴 항목을 추가합니다.
    AppendMenu(hSplitMenu, MF_STRING, ID_SPLIT_4, L"4"); // 4등분 메뉴 항목을 추가합니다.
    AppendMenu(hSplitMenu, MF_STRING, ID_SPLIT_5, L"5"); // 5등분 메뉴 항목을 추가합니다.

    AppendMenu(hGameMenu, MF_STRING, ID_GAME_START, L"게임 시작");       // 게임 시작 메뉴 항목을 추가합니다.
    AppendMenu(hGameMenu, MF_STRING, ID_GAME_FULL, L"전체 그림 보기");   // 전체 그림 보기 메뉴 항목을 추가합니다.
    AppendMenu(hGameMenu, MF_STRING, ID_GAME_END, L"게임 종료");         // 게임 종료 메뉴 항목을 추가합니다.

    AppendMenu(hMainMenu, MF_POPUP, (UINT_PTR)hImageMenu, L"그림");       // 그림 하위 메뉴를 메인 메뉴에 붙입니다.
    AppendMenu(hMainMenu, MF_POPUP, (UINT_PTR)hSplitMenu, L"그림 나누기"); // 그림 나누기 하위 메뉴를 메인 메뉴에 붙입니다.
    AppendMenu(hMainMenu, MF_POPUP, (UINT_PTR)hGameMenu, L"게임");       // 게임 하위 메뉴를 메인 메뉴에 붙입니다.

    SetMenu(hWnd, hMainMenu);                 // 완성된 메뉴를 윈도우에 설정합니다.
}

void UpdateMenuCheck(HWND hWnd)              // 현재 선택된 그림과 분할 개수를 메뉴 체크 표시로 보여주는 함수입니다.
{
    HMENU hMenu;                              // 현재 윈도우의 메뉴 핸들을 저장할 변수입니다.

    hMenu = GetMenu(hWnd);                    // 윈도우에 연결된 메뉴 핸들을 얻습니다.

    CheckMenuItem(hMenu, ID_IMAGE_1, MF_BYCOMMAND | (g_selectedImage == 0 ? MF_CHECKED : MF_UNCHECKED)); // 그림 1 선택 여부를 표시합니다.
    CheckMenuItem(hMenu, ID_IMAGE_2, MF_BYCOMMAND | (g_selectedImage == 1 ? MF_CHECKED : MF_UNCHECKED)); // 그림 2 선택 여부를 표시합니다.
    CheckMenuItem(hMenu, ID_SPLIT_3, MF_BYCOMMAND | (g_div == 3 ? MF_CHECKED : MF_UNCHECKED)); // 3등분 선택 여부를 표시합니다.
    CheckMenuItem(hMenu, ID_SPLIT_4, MF_BYCOMMAND | (g_div == 4 ? MF_CHECKED : MF_UNCHECKED)); // 4등분 선택 여부를 표시합니다.
    CheckMenuItem(hMenu, ID_SPLIT_5, MF_BYCOMMAND | (g_div == 5 ? MF_CHECKED : MF_UNCHECKED)); // 5등분 선택 여부를 표시합니다.
}

void DeletePuzzleImages()                    // 읽어온 비트맵 자원을 삭제하는 함수입니다.
{
    if (g_hBitmap[0] != NULL) {               // image.bmp 비트맵이 존재하는지 확인합니다.
        DeleteObject(g_hBitmap[0]);           // image.bmp 비트맵 GDI 자원을 삭제합니다.
        g_hBitmap[0] = NULL;                  // 삭제 후 핸들을 NULL로 바꿔 중복 삭제를 막습니다.
    }
    if (g_hBitmap[1] != NULL) {               // test.bmp 비트맵이 존재하는지 확인합니다.
        DeleteObject(g_hBitmap[1]);           // test.bmp 비트맵 GDI 자원을 삭제합니다.
        g_hBitmap[1] = NULL;                  // 삭제 후 핸들을 NULL로 바꿔 중복 삭제를 막습니다.
    }
}

void ResetBoardToEmpty()                    // 퍼즐 배열을 모두 빈 칸으로 초기화하는 함수입니다.
{
    int r;                                    // 행 반복에 사용할 변수입니다.
    int c;                                    // 열 반복에 사용할 변수입니다.

    for (r = 0; r < MAX_DIV; r++) {           // 최대 행 개수만큼 반복합니다.
        for (c = 0; c < MAX_DIV; c++) {       // 최대 열 개수만큼 반복합니다.
            g_board[r][c] = EMPTY_TILE;       // 모든 칸을 빈 칸으로 설정합니다.
        }
    }
}

void StartGridGame(HWND hWnd)                // 일반 NxN 조각 퍼즐을 시작하는 함수입니다.
{
    int r;                                    // 행 반복에 사용할 변수입니다.
    int c;                                    // 열 반복에 사용할 변수입니다.
    int tile;                                 // 퍼즐 조각 번호를 저장할 변수입니다.

    if (g_hBitmap[g_selectedImage] == NULL) { // 현재 선택한 그림을 읽지 못했다면 시작하지 않습니다.
        MessageBox(hWnd, L"선택한 BMP 파일을 읽을 수 없습니다.", L"오류", MB_OK); // 오류 메시지를 보여줍니다.
        return;                               // 함수를 끝냅니다.
    }

    KillTimer(hWnd, TIMER_ANIM);              // 이전 애니메이션 타이머가 남아있을 수 있으므로 꺼줍니다.
    ResetBoardToEmpty();                      // 기존 퍼즐 배열을 모두 지웁니다.

    g_splitMode = SPLIT_GRID;                 // 현재 분할 방식을 일반 NxN 퍼즐로 설정합니다.
    g_rows = g_div;                           // 행 개수를 현재 분할 개수로 설정합니다.
    g_cols = g_div;                           // 열 개수를 현재 분할 개수로 설정합니다.
    g_totalCells = g_rows * g_cols;           // 전체 칸 개수를 계산합니다.
    g_removedTile = NO_REMOVED;               // 오른쪽 클릭으로 사라진 조각 정보를 초기화합니다.
    g_animating = 0;                          // 애니메이션 상태를 초기화합니다.

    tile = 0;                                 // 첫 번째 조각 번호를 0으로 시작합니다.
    for (r = 0; r < g_rows; r++) {            // 현재 행 개수만큼 반복합니다.
        for (c = 0; c < g_cols; c++) {        // 현재 열 개수만큼 반복합니다.
            if (tile == g_totalCells - 1) {   // 마지막 칸은 빈 칸으로 두어야 합니다.
                g_board[r][c] = EMPTY_TILE;   // 마지막 칸을 빈 칸으로 설정합니다.
                g_emptyR = r;                 // 빈 칸 행 위치를 저장합니다.
                g_emptyC = c;                 // 빈 칸 열 위치를 저장합니다.
            }
            else {                            // 마지막 칸이 아니라면 그림 조각을 넣습니다.
                g_board[r][c] = tile;         // 현재 칸에 조각 번호를 저장합니다.
            }
            tile++;                           // 다음 칸에서 사용할 조각 번호를 1 증가시킵니다.
        }
    }

    ShuffleGridByLegalMove();                 // 실제 슬라이딩 퍼즐 이동 방식으로 조각을 랜덤하게 섞습니다.

    g_viewMode = VIEW_PUZZLE;                 // 화면을 퍼즐 보기 상태로 바꿉니다.
    g_gameActive = 1;                         // 게임 조작을 가능하게 만듭니다.
    UpdateMenuCheck(hWnd);                    // 메뉴 체크 상태를 갱신합니다.
    InvalidateRect(hWnd, NULL, TRUE);          // 화면을 다시 그리도록 요청합니다.
}

void ShuffleGridByLegalMove()                // 일반 퍼즐을 실제 이동 방식으로 섞는 함수입니다.
{
    int i;                                    // 섞는 횟수를 세는 반복 변수입니다.
    int dir;                                  // 랜덤 방향 번호입니다.
    int nr;                                   // 빈 칸 주변 조각의 행 번호입니다.
    int nc;                                   // 빈 칸 주변 조각의 열 번호입니다.
    int temp;                                 // 조각 번호를 임시 저장할 변수입니다.

    for (i = 0; i < 300; i++) {               // 300번 정도 움직이면 충분히 섞입니다.
        dir = rand() % 4;                     // 0, 1, 2, 3 중 하나의 방향을 랜덤으로 선택합니다.
        nr = g_emptyR;                        // 우선 주변 조각 행 번호를 빈 칸 행 번호로 시작합니다.
        nc = g_emptyC;                        // 우선 주변 조각 열 번호를 빈 칸 열 번호로 시작합니다.

        if (dir == 0) {                       // 0이면 빈 칸 위쪽 조각을 선택합니다.
            nr = g_emptyR - 1;                // 위쪽 조각의 행 번호입니다.
        }
        else if (dir == 1) {                  // 1이면 빈 칸 아래쪽 조각을 선택합니다.
            nr = g_emptyR + 1;                // 아래쪽 조각의 행 번호입니다.
        }
        else if (dir == 2) {                  // 2이면 빈 칸 왼쪽 조각을 선택합니다.
            nc = g_emptyC - 1;                // 왼쪽 조각의 열 번호입니다.
        }
        else {                                // 3이면 빈 칸 오른쪽 조각을 선택합니다.
            nc = g_emptyC + 1;                // 오른쪽 조각의 열 번호입니다.
        }

        if (nr >= 0 && nr < g_rows && nc >= 0 && nc < g_cols) { // 선택한 주변 칸이 보드 안에 있는지 확인합니다.
            temp = g_board[nr][nc];           // 주변 조각 번호를 임시 저장합니다.
            g_board[g_emptyR][g_emptyC] = temp; // 주변 조각을 빈 칸 위치로 옮깁니다.
            g_board[nr][nc] = EMPTY_TILE;     // 원래 주변 조각이 있던 곳을 빈 칸으로 만듭니다.
            g_emptyR = nr;                    // 새로운 빈 칸의 행 위치를 저장합니다.
            g_emptyC = nc;                    // 새로운 빈 칸의 열 위치를 저장합니다.
        }
    }

    if (IsSolved() == 1) {                    // 아주 낮은 확률로 섞은 뒤에도 맞춰진 상태일 수 있습니다.
        if (g_emptyR > 0) {                   // 빈 칸 위에 조각이 있으면 위쪽 조각과 바꿉니다.
            g_board[g_emptyR][g_emptyC] = g_board[g_emptyR - 1][g_emptyC]; // 위쪽 조각을 빈 칸으로 내립니다.
            g_board[g_emptyR - 1][g_emptyC] = EMPTY_TILE; // 위쪽 칸을 빈 칸으로 만듭니다.
            g_emptyR--;                       // 빈 칸 위치를 위로 이동시킵니다.
        }
        else if (g_emptyC > 0) {              // 위쪽 조각이 없고 왼쪽 조각이 있으면 왼쪽 조각과 바꿉니다.
            g_board[g_emptyR][g_emptyC] = g_board[g_emptyR][g_emptyC - 1]; // 왼쪽 조각을 빈 칸으로 옮깁니다.
            g_board[g_emptyR][g_emptyC - 1] = EMPTY_TILE; // 왼쪽 칸을 빈 칸으로 만듭니다.
            g_emptyC--;                       // 빈 칸 위치를 왼쪽으로 이동시킵니다.
        }
    }
}

void StartStripGame(HWND hWnd, int mode)     // 가로 또는 세로 한 줄 퍼즐을 시작하는 함수입니다.
{
    int i;                                    // 조각 번호를 넣을 때 사용할 반복 변수입니다.

    if (g_hBitmap[g_selectedImage] == NULL) { // 현재 선택한 그림을 읽지 못했다면 시작하지 않습니다.
        MessageBox(hWnd, L"선택한 BMP 파일을 읽을 수 없습니다.", L"오류", MB_OK); // 오류 메시지를 보여줍니다.
        return;                               // 함수를 끝냅니다.
    }

    KillTimer(hWnd, TIMER_ANIM);              // 일반 퍼즐 애니메이션 타이머가 켜져 있을 수 있으므로 꺼줍니다.
    ResetBoardToEmpty();                      // 기존 퍼즐 배열을 모두 지웁니다.

    g_splitMode = mode;                       // 현재 분할 방식을 세로 또는 가로 한 줄 퍼즐로 저장합니다.
    g_removedTile = NO_REMOVED;               // 오른쪽 클릭으로 사라진 조각 정보를 초기화합니다.
    g_animating = 0;                          // 애니메이션 상태를 초기화합니다.
    g_emptyR = -1;                            // 한 줄 퍼즐은 기본 빈 칸이 없으므로 -1로 설정합니다.
    g_emptyC = -1;                            // 한 줄 퍼즐은 기본 빈 칸이 없으므로 -1로 설정합니다.

    if (mode == SPLIT_VERTICAL) {             // v 명령처럼 세로 방향으로 나눌 때입니다.
        g_rows = g_div;                       // 행 개수는 분할 개수만큼 둡니다.
        g_cols = 1;                           // 열 개수는 1개만 둡니다.
    }
    else {                                    // h 명령처럼 가로 방향으로 나눌 때입니다.
        g_rows = 1;                           // 행 개수는 1개만 둡니다.
        g_cols = g_div;                       // 열 개수는 분할 개수만큼 둡니다.
    }

    g_totalCells = g_rows * g_cols;           // 전체 칸 개수를 계산합니다.

    for (i = 0; i < g_totalCells; i++) {      // 현재 칸 개수만큼 반복합니다.
        g_board[i / g_cols][i % g_cols] = i;  // 순서대로 조각 번호를 넣습니다.
    }

    ShuffleStripBoard();                      // 한 줄 퍼즐의 조각 순서를 랜덤하게 바꿉니다.

    g_viewMode = VIEW_PUZZLE;                 // 화면을 퍼즐 보기 상태로 바꿉니다.
    g_gameActive = 1;                         // 게임 조작을 가능하게 만듭니다.
    UpdateMenuCheck(hWnd);                    // 메뉴 체크 상태를 갱신합니다.
    InvalidateRect(hWnd, NULL, TRUE);          // 화면을 다시 그리도록 요청합니다.
}

void ShuffleStripBoard()                     // 한 줄 퍼즐 조각을 랜덤하게 섞는 함수입니다.
{
    int i;                                    // 섞는 횟수를 세는 반복 변수입니다.
    int a;                                    // 첫 번째 랜덤 칸 번호입니다.
    int b;                                    // 두 번째 랜덤 칸 번호입니다.
    int ar;                                   // 첫 번째 칸의 행 번호입니다.
    int ac;                                   // 첫 번째 칸의 열 번호입니다.
    int br;                                   // 두 번째 칸의 행 번호입니다.
    int bc;                                   // 두 번째 칸의 열 번호입니다.
    int temp;                                 // 조각 번호를 임시 저장할 변수입니다.

    for (i = 0; i < 50; i++) {                // 50번 정도 두 칸을 서로 바꾸어 섞습니다.
        a = rand() % g_totalCells;            // 첫 번째 칸 번호를 랜덤으로 선택합니다.
        b = rand() % g_totalCells;            // 두 번째 칸 번호를 랜덤으로 선택합니다.
        ar = a / g_cols;                      // 첫 번째 칸 번호를 행 번호로 바꿉니다.
        ac = a % g_cols;                      // 첫 번째 칸 번호를 열 번호로 바꿉니다.
        br = b / g_cols;                      // 두 번째 칸 번호를 행 번호로 바꿉니다.
        bc = b % g_cols;                      // 두 번째 칸 번호를 열 번호로 바꿉니다.
        temp = g_board[ar][ac];               // 첫 번째 칸의 조각을 임시 저장합니다.
        g_board[ar][ac] = g_board[br][bc];    // 두 번째 칸의 조각을 첫 번째 칸에 넣습니다.
        g_board[br][bc] = temp;               // 임시 저장한 조각을 두 번째 칸에 넣습니다.
    }

    if (IsSolved() == 1 && g_totalCells > 1) { // 섞었는데도 정답 상태라면 첫 두 조각을 강제로 바꿉니다.
        temp = g_board[0][0];                 // 첫 번째 조각을 임시 저장합니다.
        if (g_cols > 1) {                     // 가로 퍼즐이면 두 번째 열과 바꿉니다.
            g_board[0][0] = g_board[0][1];    // 두 번째 조각을 첫 번째 위치에 넣습니다.
            g_board[0][1] = temp;             // 첫 번째 조각을 두 번째 위치에 넣습니다.
        }
        else {                                // 세로 퍼즐이면 두 번째 행과 바꿉니다.
            g_board[0][0] = g_board[1][0];    // 두 번째 조각을 첫 번째 위치에 넣습니다.
            g_board[1][0] = temp;             // 첫 번째 조각을 두 번째 위치에 넣습니다.
        }
    }
}

void ShowFullImage(HWND hWnd)                // 전체 그림 보기 상태로 전환하는 함수입니다.
{
    KillTimer(hWnd, TIMER_ANIM);              // 이동 애니메이션 타이머가 켜져 있다면 꺼줍니다.
    g_viewMode = VIEW_FULL;                   // 화면 상태를 전체 그림 보기로 설정합니다.
    g_gameActive = 0;                         // 전체 그림 보기에서는 퍼즐 조작을 막습니다.
    g_animating = 0;                          // 애니메이션 상태를 초기화합니다.
    UpdateMenuCheck(hWnd);                    // 메뉴 체크 표시를 갱신합니다.
    InvalidateRect(hWnd, NULL, TRUE);          // 화면을 다시 그리도록 요청합니다.
}

void EndGame(HWND hWnd)                      // 게임 종료 상태로 전환하는 함수입니다.
{
    KillTimer(hWnd, TIMER_ANIM);              // 이동 애니메이션 타이머가 켜져 있다면 꺼줍니다.
    g_gameActive = 0;                         // 퍼즐 이동을 막습니다.
    g_animating = 0;                          // 애니메이션 상태를 초기화합니다.
    g_viewMode = VIEW_END;                    // 화면 상태를 게임 종료 상태로 설정합니다.
    InvalidateRect(hWnd, NULL, TRUE);          // 화면을 다시 그리도록 요청합니다.
}

void FindFirstEmpty()                        // 현재 퍼즐 배열에서 첫 번째 빈 칸을 찾는 함수입니다.
{
    int r;                                    // 행 반복에 사용할 변수입니다.
    int c;                                    // 열 반복에 사용할 변수입니다.

    g_emptyR = -1;                            // 우선 빈 칸이 없다고 설정합니다.
    g_emptyC = -1;                            // 우선 빈 칸이 없다고 설정합니다.

    for (r = 0; r < g_rows; r++) {            // 현재 행 개수만큼 반복합니다.
        for (c = 0; c < g_cols; c++) {        // 현재 열 개수만큼 반복합니다.
            if (g_board[r][c] == EMPTY_TILE) { // 현재 칸이 빈 칸인지 확인합니다.
                g_emptyR = r;                 // 빈 칸의 행 위치를 저장합니다.
                g_emptyC = c;                 // 빈 칸의 열 위치를 저장합니다.
                return;                       // 첫 번째 빈 칸을 찾았으므로 함수를 끝냅니다.
            }
        }
    }
}

int IsSolved()                               // 현재 퍼즐이 정답 상태인지 검사하는 함수입니다.
{
    int r;                                    // 행 반복에 사용할 변수입니다.
    int c;                                    // 열 반복에 사용할 변수입니다.
    int index;                                // 현재 칸이 원래 가져야 하는 조각 번호입니다.

    index = 0;                                // 첫 번째 칸의 정답 조각 번호는 0입니다.

    for (r = 0; r < g_rows; r++) {            // 현재 행 개수만큼 반복합니다.
        for (c = 0; c < g_cols; c++) {        // 현재 열 개수만큼 반복합니다.
            if (g_splitMode == SPLIT_GRID && index == g_totalCells - 1) { // 일반 퍼즐의 마지막 칸은 빈 칸이어야 합니다.
                if (g_board[r][c] != EMPTY_TILE) { // 마지막 칸이 빈 칸이 아니면 정답이 아닙니다.
                    return 0;                 // 정답이 아니므로 0을 반환합니다.
                }
            }
            else {                            // 마지막 빈 칸이 아닌 일반 조각 칸입니다.
                if (g_board[r][c] != index) { // 현재 칸의 조각 번호가 정답 번호와 다르면 틀린 상태입니다.
                    return 0;                 // 정답이 아니므로 0을 반환합니다.
                }
            }
            index++;                          // 다음 칸의 정답 조각 번호로 넘어갑니다.
        }
    }

    return 1;                                 // 모든 칸이 맞았으므로 정답 상태입니다.
}

void CheckSolvedAndFinish(HWND hWnd)          // 퍼즐을 맞췄는지 확인하고 맞췄다면 게임을 끝내는 함수입니다.
{
    if (g_gameActive == 1 && IsSolved() == 1) { // 게임 중이고 정답 상태인지 확인합니다.
        g_gameActive = 0;                     // 더 이상 조작하지 못하게 게임을 멈춥니다.
        MessageBox(hWnd, L"퍼즐을 모두 맞췄습니다!", L"게임 종료", MB_OK); // 성공 메시지를 보여줍니다.
    }
}

void GetPuzzleRect(HWND hWnd, RECT* rc)       // 현재 창 크기에 맞게 퍼즐 출력 영역을 계산하는 함수입니다.
{
    RECT client;                              // 클라이언트 영역을 저장할 변수입니다.
    int availW;                               // 퍼즐을 그릴 수 있는 최대 가로 공간입니다.
    int availH;                               // 퍼즐을 그릴 수 있는 최대 세로 공간입니다.
    int bmpW;                                 // 현재 그림의 가로 크기입니다.
    int bmpH;                                 // 현재 그림의 세로 크기입니다.
    int drawW;                                // 실제로 그릴 퍼즐 영역의 가로 크기입니다.
    int drawH;                                // 실제로 그릴 퍼즐 영역의 세로 크기입니다.

    GetClientRect(hWnd, &client);             // 현재 창의 클라이언트 영역을 얻습니다.
    availW = client.right - client.left - 80; // 좌우 여백 40씩을 뺀 가로 공간을 계산합니다.
    availH = client.bottom - client.top - 130; // 위쪽 안내문과 아래쪽 여백을 뺀 세로 공간을 계산합니다.

    if (availW < 100) {                       // 창이 너무 작아도 최소 크기를 보장합니다.
        availW = 100;                         // 가로 최소 공간을 100으로 설정합니다.
    }
    if (availH < 100) {                       // 창이 너무 작아도 최소 크기를 보장합니다.
        availH = 100;                         // 세로 최소 공간을 100으로 설정합니다.
    }

    if (g_hBitmap[g_selectedImage] != NULL) { // 현재 그림을 정상적으로 읽었다면 그림 크기를 사용합니다.
        bmpW = g_bmpInfo[g_selectedImage].bmWidth;  // 현재 그림의 실제 가로 크기를 얻습니다.
        bmpH = g_bmpInfo[g_selectedImage].bmHeight; // 현재 그림의 실제 세로 크기를 얻습니다.
    }
    else {                                    // 그림을 읽지 못했다면 기본 비율을 사용합니다.
        bmpW = 600;                           // 기본 가로 크기입니다.
        bmpH = 400;                           // 기본 세로 크기입니다.
    }

    drawW = availW;                           // 우선 가능한 가로 공간을 모두 사용한다고 가정합니다.
    drawH = drawW * bmpH / bmpW;              // 그림 비율에 맞춰 세로 크기를 계산합니다.

    if (drawH > availH) {                     // 계산된 세로 크기가 가능한 세로 공간보다 크면 조정합니다.
        drawH = availH;                       // 세로 크기를 가능한 세로 공간에 맞춥니다.
        drawW = drawH * bmpW / bmpH;          // 그림 비율에 맞춰 가로 크기를 다시 계산합니다.
    }

    rc->left = client.left + ((client.right - client.left) - drawW) / 2; // 퍼즐 영역을 가로 중앙에 배치합니다.
    rc->top = client.top + 80;                // 위쪽 안내문 아래에서 시작하도록 y좌표를 정합니다.
    rc->right = rc->left + drawW;             // 퍼즐 영역의 오른쪽 좌표를 계산합니다.
    rc->bottom = rc->top + drawH;             // 퍼즐 영역의 아래쪽 좌표를 계산합니다.
}

void GetCellRect(HWND hWnd, int r, int c, RECT* rcCell) // 특정 퍼즐 칸의 화면 좌표를 계산하는 함수입니다.
{
    RECT rcPuzzle;                            // 전체 퍼즐 영역을 저장할 변수입니다.
    int left;                                 // 칸의 왼쪽 좌표입니다.
    int right;                                // 칸의 오른쪽 좌표입니다.
    int top;                                  // 칸의 위쪽 좌표입니다.
    int bottom;                               // 칸의 아래쪽 좌표입니다.

    GetPuzzleRect(hWnd, &rcPuzzle);           // 전체 퍼즐 영역을 계산합니다.

    left = rcPuzzle.left + (rcPuzzle.right - rcPuzzle.left) * c / g_cols; // 열 번호에 따라 칸의 왼쪽 좌표를 계산합니다.
    right = rcPuzzle.left + (rcPuzzle.right - rcPuzzle.left) * (c + 1) / g_cols; // 열 번호에 따라 칸의 오른쪽 좌표를 계산합니다.
    top = rcPuzzle.top + (rcPuzzle.bottom - rcPuzzle.top) * r / g_rows; // 행 번호에 따라 칸의 위쪽 좌표를 계산합니다.
    bottom = rcPuzzle.top + (rcPuzzle.bottom - rcPuzzle.top) * (r + 1) / g_rows; // 행 번호에 따라 칸의 아래쪽 좌표를 계산합니다.

    rcCell->left = left;                      // 계산한 왼쪽 좌표를 저장합니다.
    rcCell->right = right;                    // 계산한 오른쪽 좌표를 저장합니다.
    rcCell->top = top;                        // 계산한 위쪽 좌표를 저장합니다.
    rcCell->bottom = bottom;                  // 계산한 아래쪽 좌표를 저장합니다.
}

int GetCellFromPoint(HWND hWnd, int x, int y, int* r, int* c) // 마우스 좌표가 어느 퍼즐 칸인지 찾는 함수입니다.
{
    RECT rcPuzzle;                            // 전체 퍼즐 영역을 저장할 변수입니다.
    int width;                                // 전체 퍼즐 영역의 가로 길이입니다.
    int height;                               // 전체 퍼즐 영역의 세로 길이입니다.

    GetPuzzleRect(hWnd, &rcPuzzle);           // 전체 퍼즐 영역을 계산합니다.

    if (x < rcPuzzle.left || x >= rcPuzzle.right || y < rcPuzzle.top || y >= rcPuzzle.bottom) { // 마우스가 퍼즐 영역 밖인지 확인합니다.
        return 0;                             // 퍼즐 영역 밖이면 0을 반환합니다.
    }

    width = rcPuzzle.right - rcPuzzle.left;   // 전체 퍼즐 영역의 가로 길이를 계산합니다.
    height = rcPuzzle.bottom - rcPuzzle.top;  // 전체 퍼즐 영역의 세로 길이를 계산합니다.

    *c = (x - rcPuzzle.left) * g_cols / width; // x좌표를 열 번호로 변환합니다.
    *r = (y - rcPuzzle.top) * g_rows / height; // y좌표를 행 번호로 변환합니다.

    if (*r < 0 || *r >= g_rows || *c < 0 || *c >= g_cols) { // 계산된 행과 열이 정상 범위인지 확인합니다.
        return 0;                             // 범위 밖이면 0을 반환합니다.
    }

    return 1;                                 // 정상적으로 칸을 찾았으므로 1을 반환합니다.
}

void DrawScene(HWND hWnd, HDC hDC)           // 화면 전체를 그리는 함수입니다.
{
    HDC hImgDC;                               // 그림 비트맵을 선택할 메모리 DC입니다.
    HBITMAP hOldImg;                          // 이미지 DC에 원래 들어 있던 비트맵을 저장합니다.
    RECT rcClient;                            // 클라이언트 영역을 저장할 변수입니다.
    RECT rcPuzzle;                            // 전체 퍼즐 영역을 저장할 변수입니다.
    RECT rcCell;                              // 각 퍼즐 칸 영역을 저장할 변수입니다.
    RECT rcAnimStart;                         // 애니메이션 시작 칸 좌표입니다.
    RECT rcAnimEnd;                           // 애니메이션 도착 칸 좌표입니다.
    RECT rcAnim;                              // 애니메이션 중인 현재 좌표입니다.
    HBRUSH hEmptyBrush;                       // 빈 칸을 칠할 브러시입니다.
    int r;                                    // 행 반복에 사용할 변수입니다.
    int c;                                    // 열 반복에 사용할 변수입니다.
    int bmpW;                                 // 현재 그림의 가로 크기입니다.
    int bmpH;                                 // 현재 그림의 세로 크기입니다.
    WCHAR info[256];                          // 화면 위쪽 안내 문장을 만들 문자 배열입니다.

    GetClientRect(hWnd, &rcClient);           // 현재 창의 클라이언트 영역을 얻습니다.
    SetBkMode(hDC, TRANSPARENT);              // 글자 배경을 투명하게 그리도록 설정합니다.

    wsprintf(info, L"그림: %s   나누기: %d   키: 1/2 그림, 3/4/5 나누기, s 시작, f 전체, v 세로, h 가로, q 종료", // 안내 문자열 형식을 만듭니다.
        g_selectedImage == 0 ? L"image.bmp" : L"test.bmp", g_div); // 현재 그림 이름과 분할 개수를 문자열에 넣습니다.
    TextOut(hDC, 20, 15, info, lstrlen(info)); // 안내 문자열을 화면 위쪽에 출력합니다.

    if (g_viewMode == VIEW_FULL) {            // 전체 그림 보기 상태인지 확인합니다.
        DRAW_TEXT(hDC, 20, 40, L"전체 그림 보기 상태입니다. s를 누르거나 [게임]-[게임 시작]을 선택하면 퍼즐이 시작됩니다."); // 상태 안내 문장입니다.
    }
    else if (g_viewMode == VIEW_END) {        // 게임 종료 상태인지 확인합니다.
        DRAW_TEXT(hDC, 20, 40, L"게임 종료 상태입니다. 퍼즐 이동이 안 됩니다. 다시 시작하려면 s를 누르세요."); // 상태 안내 문장입니다.
    }
    else if (g_splitMode == SPLIT_GRID) {     // 일반 NxN 퍼즐 상태인지 확인합니다.
        DRAW_TEXT(hDC, 20, 40, L"드래그: 빈 칸 주변 조각 이동 / 더블클릭: 해당 조각을 빈 칸으로 이동 / 오른쪽 클릭: 조각 제거 또는 복구"); // 조작 안내 문장입니다.
    }
    else {                                    // 가로 또는 세로 한 줄 퍼즐 상태입니다.
        DRAW_TEXT(hDC, 20, 40, L"한 줄 퍼즐: 조각을 누른 채 다른 칸에 놓으면 두 조각의 위치가 바뀝니다."); // 조작 안내 문장입니다.
    }

    if (g_hBitmap[g_selectedImage] == NULL) { // 현재 선택한 그림을 읽지 못했다면 그림 대신 오류 문장을 보여줍니다.
        DRAW_TEXT(hDC, 20, 90, L"BMP 파일을 읽지 못했습니다. image.bmp와 test.bmp를 exe 파일과 같은 폴더에 넣으세요."); // 파일 위치 안내 문장입니다.
        return;                               // 그림을 그릴 수 없으므로 함수를 끝냅니다.
    }

    GetPuzzleRect(hWnd, &rcPuzzle);           // 퍼즐 또는 전체 그림을 그릴 영역을 계산합니다.
    hImgDC = CreateCompatibleDC(NULL);        // 비트맵 이미지를 그리기 위한 화면 호환 메모리 DC를 만듭니다.
    hOldImg = (HBITMAP)SelectObject(hImgDC, g_hBitmap[g_selectedImage]); // 현재 선택한 비트맵을 이미지 DC에 선택합니다.

    if (hOldImg == NULL) {                    // SelectObject가 실패하면 비트맵을 DC에 연결하지 못한 것입니다.
        DRAW_TEXT(hDC, 20, 90, L"비트맵을 메모리 DC에 연결하지 못했습니다."); // 실패 원인을 화면에 출력합니다.
        DeleteDC(hImgDC);                     // 생성한 이미지 DC를 삭제합니다.
        return;                               // 더 이상 이미지를 그릴 수 없으므로 함수를 끝냅니다.
    }

    SetStretchBltMode(hDC, COLORONCOLOR);     // BMP 확대/축소 시 색이 이상하게 섞이지 않도록 기본 복사 모드로 설정합니다.
    bmpW = g_bmpInfo[g_selectedImage].bmWidth; // 현재 비트맵의 가로 크기를 얻습니다.
    bmpH = g_bmpInfo[g_selectedImage].bmHeight; // 현재 비트맵의 세로 크기를 얻습니다.

    if (bmpW <= 0 || bmpH <= 0) {             // 이미지 크기가 비정상이라면 그릴 수 없습니다.
        DRAW_TEXT(hDC, 20, 90, L"BMP 크기 정보가 비정상입니다."); // 크기 오류를 화면에 출력합니다.
        SelectObject(hImgDC, hOldImg);        // 이미지 DC에 원래 비트맵을 되돌립니다.
        DeleteDC(hImgDC);                     // 이미지 DC를 삭제합니다.
        return;                               // 함수를 끝냅니다.
    }

    if (g_viewMode == VIEW_FULL) {            // 전체 그림 보기 상태라면 한 장의 그림으로 출력합니다.
        if (StretchBlt(hDC, rcPuzzle.left, rcPuzzle.top, rcPuzzle.right - rcPuzzle.left, rcPuzzle.bottom - rcPuzzle.top, hImgDC, 0, 0, bmpW, bmpH, SRCCOPY) == FALSE) { // 전체 이미지를 화면 영역에 맞게 확대/축소해서 그리며 성공 여부를 확인합니다.
            DRAW_TEXT(hDC, 20, 90, L"StretchBlt 실패: 이미지는 로딩됐지만 화면 복사에 실패했습니다."); // 실패하면 화면에 원인을 출력합니다.
            BitBlt(hDC, rcPuzzle.left, rcPuzzle.top, bmpW, bmpH, hImgDC, 0, 0, SRCCOPY); // 테스트용으로 원본 크기 그대로 한 번 더 그려봅니다.
        }
        DrawBorderOnly(hDC, rcPuzzle.left, rcPuzzle.top, rcPuzzle.right, rcPuzzle.bottom); // 전체 그림 주변에 테두리만 그립니다.
    }
    else {                                    // 퍼즐 보기 또는 게임 종료 상태라면 조각별로 그립니다.
        hEmptyBrush = CreateSolidBrush(RGB(230, 230, 230)); // 빈 칸을 연한 회색으로 칠할 브러시를 만듭니다.

        for (r = 0; r < g_rows; r++) {        // 현재 행 개수만큼 반복합니다.
            for (c = 0; c < g_cols; c++) {    // 현재 열 개수만큼 반복합니다.
                GetCellRect(hWnd, r, c, &rcCell); // 현재 칸의 화면 좌표를 계산합니다.
                if (g_board[r][c] == EMPTY_TILE) { // 현재 칸이 빈 칸인지 확인합니다.
                    FillRect(hDC, &rcCell, hEmptyBrush); // 빈 칸을 회색으로 칠합니다.
                    DrawBorderOnly(hDC, rcCell.left, rcCell.top, rcCell.right, rcCell.bottom); // 빈 칸 테두리만 그립니다.
                }
                else {                        // 현재 칸에 그림 조각이 있는 경우입니다.
                    DrawTile(hDC, hImgDC, g_board[r][c], rcCell); // 현재 칸에 해당 조각 이미지를 그립니다.
                    DrawBorderOnly(hDC, rcCell.left, rcCell.top, rcCell.right, rcCell.bottom); // 조각 테두리만 그립니다.
                }
            }
        }

        DeleteObject(hEmptyBrush);            // 빈 칸 브러시를 삭제하여 GDI 자원을 정리합니다.

        if (g_animating == 1) {               // 조각이 이동 중이라면 이동 중인 조각을 따로 그립니다.
            GetCellRect(hWnd, g_animSrcR, g_animSrcC, &rcAnimStart); // 출발 칸의 좌표를 계산합니다.
            GetCellRect(hWnd, g_animDstR, g_animDstC, &rcAnimEnd); // 도착 칸의 좌표를 계산합니다.
            rcAnim.left = rcAnimStart.left + (rcAnimEnd.left - rcAnimStart.left) * g_animStep / g_animMaxStep; // 현재 왼쪽 좌표를 보간 계산합니다.
            rcAnim.top = rcAnimStart.top + (rcAnimEnd.top - rcAnimStart.top) * g_animStep / g_animMaxStep; // 현재 위쪽 좌표를 보간 계산합니다.
            rcAnim.right = rcAnimStart.right + (rcAnimEnd.right - rcAnimStart.right) * g_animStep / g_animMaxStep; // 현재 오른쪽 좌표를 보간 계산합니다.
            rcAnim.bottom = rcAnimStart.bottom + (rcAnimEnd.bottom - rcAnimStart.bottom) * g_animStep / g_animMaxStep; // 현재 아래쪽 좌표를 보간 계산합니다.
            DrawTile(hDC, hImgDC, g_animTile, rcAnim); // 계산된 현재 위치에 이동 중인 조각을 그립니다.
            DrawBorderOnly(hDC, rcAnim.left, rcAnim.top, rcAnim.right, rcAnim.bottom); // 이동 중인 조각의 테두리만 그립니다.
        }
    }

    SelectObject(hImgDC, hOldImg);            // 이미지 DC에 원래 비트맵을 다시 선택합니다.
    DeleteDC(hImgDC);                         // 이미지 DC를 삭제하여 GDI 자원을 정리합니다.
}

void DrawBorderOnly(HDC hDC, int left, int top, int right, int bottom) // 사각형의 안쪽을 흰색으로 덮지 않고 테두리만 그리는 함수입니다.
{
    HBRUSH hOldBrush;                         // 기존 브러시를 저장할 변수입니다.

    hOldBrush = (HBRUSH)SelectObject(hDC, GetStockObject(NULL_BRUSH)); // NULL_BRUSH를 선택하면 Rectangle이 내부를 채우지 않고 테두리만 그립니다.
    Rectangle(hDC, left, top, right, bottom);  // 현재 펜으로 사각형 테두리만 그립니다.
    SelectObject(hDC, hOldBrush);             // DC의 브러시를 원래 브러시로 되돌립니다.
}

void DrawTile(HDC hDC, HDC hImgDC, int tile, RECT rcDest) // 퍼즐 조각 하나를 원본 이미지에서 잘라 그리는 함수입니다.
{
    int srcR;                                 // 원본 이미지에서 조각이 있던 행 번호입니다.
    int srcC;                                 // 원본 이미지에서 조각이 있던 열 번호입니다.
    int srcLeft;                              // 원본 이미지에서 잘라낼 왼쪽 좌표입니다.
    int srcRight;                             // 원본 이미지에서 잘라낼 오른쪽 좌표입니다.
    int srcTop;                               // 원본 이미지에서 잘라낼 위쪽 좌표입니다.
    int srcBottom;                            // 원본 이미지에서 잘라낼 아래쪽 좌표입니다.
    int bmpW;                                 // 원본 이미지의 가로 크기입니다.
    int bmpH;                                 // 원본 이미지의 세로 크기입니다.

    if (tile == EMPTY_TILE) {                 // 빈 칸이면 그릴 이미지가 없습니다.
        return;                               // 함수를 끝냅니다.
    }

    bmpW = g_bmpInfo[g_selectedImage].bmWidth; // 현재 원본 이미지의 가로 크기를 얻습니다.
    bmpH = g_bmpInfo[g_selectedImage].bmHeight; // 현재 원본 이미지의 세로 크기를 얻습니다.
    srcR = tile / g_cols;                     // 조각 번호를 원래 행 번호로 변환합니다.
    srcC = tile % g_cols;                     // 조각 번호를 원래 열 번호로 변환합니다.

    srcLeft = bmpW * srcC / g_cols;           // 원본 이미지에서 조각의 왼쪽 좌표를 계산합니다.
    srcRight = bmpW * (srcC + 1) / g_cols;    // 원본 이미지에서 조각의 오른쪽 좌표를 계산합니다.
    srcTop = bmpH * srcR / g_rows;            // 원본 이미지에서 조각의 위쪽 좌표를 계산합니다.
    srcBottom = bmpH * (srcR + 1) / g_rows;   // 원본 이미지에서 조각의 아래쪽 좌표를 계산합니다.

    SetStretchBltMode(hDC, COLORONCOLOR);       // 조각 이미지를 확대/축소할 때 기본 복사 모드를 사용합니다.

    if (StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right - rcDest.left, rcDest.bottom - rcDest.top, // 목적지 화면 좌표와 크기입니다.
        hImgDC, srcLeft, srcTop, srcRight - srcLeft, srcBottom - srcTop, SRCCOPY) == FALSE) { // 원본 이미지의 일부를 목적지 칸에 맞게 그리며 성공 여부를 확인합니다.
        DrawBorderOnly(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom); // 실패하면 최소한 칸 테두리만 보이게 합니다.
    }
}

void ProcessDragMove(HWND hWnd, int dx, int dy) // 드래그 방향에 따라 빈 칸 주변 조각을 이동시키는 함수입니다.
{
    int srcR;                                 // 빈 칸으로 이동할 조각의 행 번호입니다.
    int srcC;                                 // 빈 칸으로 이동할 조각의 열 번호입니다.

    if (g_emptyR == -1 || g_emptyC == -1) {    // 빈 칸이 없다면 이동할 수 없습니다.
        FindFirstEmpty();                     // 혹시 배열에 빈 칸이 있는지 다시 찾아봅니다.
    }
    if (g_emptyR == -1 || g_emptyC == -1) {    // 다시 찾아도 빈 칸이 없다면 처리하지 않습니다.
        return;                               // 함수를 끝냅니다.
    }

    srcR = g_emptyR;                          // 우선 이동할 조각 위치를 빈 칸 위치로 시작합니다.
    srcC = g_emptyC;                          // 우선 이동할 조각 위치를 빈 칸 위치로 시작합니다.

    if (abs(dx) > abs(dy)) {                  // 가로 방향 드래그가 세로 방향보다 더 큰 경우입니다.
        if (dx > 0) {                         // 오른쪽으로 드래그한 경우입니다.
            srcC = g_emptyC - 1;              // 빈 칸 왼쪽 조각이 오른쪽으로 이동해야 합니다.
        }
        else {                                // 왼쪽으로 드래그한 경우입니다.
            srcC = g_emptyC + 1;              // 빈 칸 오른쪽 조각이 왼쪽으로 이동해야 합니다.
        }
    }
    else {                                    // 세로 방향 드래그가 더 큰 경우입니다.
        if (dy > 0) {                         // 아래쪽으로 드래그한 경우입니다.
            srcR = g_emptyR - 1;              // 빈 칸 위쪽 조각이 아래로 이동해야 합니다.
        }
        else {                                // 위쪽으로 드래그한 경우입니다.
            srcR = g_emptyR + 1;              // 빈 칸 아래쪽 조각이 위로 이동해야 합니다.
        }
    }

    if (srcR < 0 || srcR >= g_rows || srcC < 0 || srcC >= g_cols) { // 이동할 조각 위치가 보드 밖인지 확인합니다.
        return;                               // 보드 밖이면 이동하지 않습니다.
    }
    if (g_board[srcR][srcC] == EMPTY_TILE) {  // 이동하려는 위치에 조각이 없으면 이동할 수 없습니다.
        return;                               // 함수를 끝냅니다.
    }

    BeginMoveTile(hWnd, srcR, srcC);          // 선택된 주변 조각을 빈 칸으로 미끄러지듯 이동시킵니다.
}

void BeginMoveTile(HWND hWnd, int srcR, int srcC) // 조각 하나를 빈 칸으로 이동시키는 애니메이션을 시작하는 함수입니다.
{
    if (g_animating == 1) {                   // 이미 애니메이션 중이면 새 이동을 시작하지 않습니다.
        return;                               // 함수를 끝냅니다.
    }
    if (g_emptyR == -1 || g_emptyC == -1) {    // 빈 칸 위치가 저장되어 있지 않다면 확인합니다.
        FindFirstEmpty();                     // 배열에서 첫 번째 빈 칸을 찾습니다.
    }
    if (g_emptyR == -1 || g_emptyC == -1) {    // 빈 칸이 없다면 이동할 수 없습니다.
        MessageBox(hWnd, L"빈 칸이 없어서 이동할 수 없습니다.", L"오류", MB_OK); // 오류 메시지를 보여줍니다.
        return;                               // 함수를 끝냅니다.
    }
    if (srcR < 0 || srcR >= g_rows || srcC < 0 || srcC >= g_cols) { // 출발 위치가 정상 범위인지 확인합니다.
        return;                               // 범위 밖이면 함수를 끝냅니다.
    }
    if (g_board[srcR][srcC] == EMPTY_TILE) {  // 출발 칸에 그림이 없으면 이동할 수 없습니다.
        return;                               // 함수를 끝냅니다.
    }

    g_animating = 1;                          // 애니메이션 중 상태로 설정합니다.
    g_animStep = 0;                           // 애니메이션을 첫 단계부터 시작합니다.
    g_animSrcR = srcR;                        // 출발 행 번호를 저장합니다.
    g_animSrcC = srcC;                        // 출발 열 번호를 저장합니다.
    g_animDstR = g_emptyR;                    // 도착 행 번호는 현재 빈 칸 행 번호입니다.
    g_animDstC = g_emptyC;                    // 도착 열 번호는 현재 빈 칸 열 번호입니다.
    g_animTile = g_board[srcR][srcC];         // 이동할 조각 번호를 저장합니다.
    g_board[srcR][srcC] = EMPTY_TILE;         // 이동 중에는 출발 칸을 빈 칸으로 보이게 만듭니다.

    SetTimer(hWnd, TIMER_ANIM, 15, NULL);     // 15ms마다 WM_TIMER가 오도록 설정해서 부드럽게 움직입니다.
}

void FinishMoveTile(HWND hWnd)               // 조각 이동 애니메이션을 끝내는 함수입니다.
{
    KillTimer(hWnd, TIMER_ANIM);              // 애니메이션 타이머를 끕니다.

    g_board[g_animDstR][g_animDstC] = g_animTile; // 이동하던 조각을 도착 칸에 실제로 넣습니다.
    g_emptyR = g_animSrcR;                    // 조각이 떠난 출발 칸이 새로운 빈 칸이 됩니다.
    g_emptyC = g_animSrcC;                    // 조각이 떠난 출발 칸이 새로운 빈 칸이 됩니다.

    g_animating = 0;                          // 애니메이션 상태를 끝냅니다.
    g_animTile = EMPTY_TILE;                  // 이동 중인 조각 번호를 초기화합니다.

    InvalidateRect(hWnd, NULL, TRUE);          // 이동이 끝난 화면을 다시 그리도록 요청합니다.
    CheckSolvedAndFinish(hWnd);               // 이동 후 퍼즐을 맞췄는지 검사합니다.
}

void SwapStripCells(HWND hWnd, int r1, int c1, int r2, int c2) // 한 줄 퍼즐에서 두 칸의 조각을 서로 바꾸는 함수입니다.
{
    int temp;                                 // 조각 번호를 임시 저장할 변수입니다.

    if (r1 < 0 || r1 >= g_rows || c1 < 0 || c1 >= g_cols) { // 첫 번째 칸이 정상 범위인지 확인합니다.
        return;                               // 범위 밖이면 함수를 끝냅니다.
    }
    if (r2 < 0 || r2 >= g_rows || c2 < 0 || c2 >= g_cols) { // 두 번째 칸이 정상 범위인지 확인합니다.
        return;                               // 범위 밖이면 함수를 끝냅니다.
    }
    if (r1 == r2 && c1 == c2) {               // 같은 칸에 놓았다면 바꿀 필요가 없습니다.
        return;                               // 함수를 끝냅니다.
    }

    temp = g_board[r1][c1];                   // 첫 번째 칸의 조각 번호를 임시 저장합니다.
    g_board[r1][c1] = g_board[r2][c2];        // 두 번째 칸의 조각을 첫 번째 칸에 넣습니다.
    g_board[r2][c2] = temp;                   // 임시 저장한 첫 번째 조각을 두 번째 칸에 넣습니다.

    InvalidateRect(hWnd, NULL, TRUE);          // 바뀐 화면을 다시 그리도록 요청합니다.
    CheckSolvedAndFinish(hWnd);               // 위치를 바꾼 뒤 정답인지 검사합니다.
}

void RemoveOrRestoreTile(HWND hWnd, int r, int c) // 오른쪽 클릭으로 그림을 제거하거나 복구하는 함수입니다.
{
    if (r < 0 || r >= g_rows || c < 0 || c >= g_cols) { // 클릭한 칸이 정상 범위인지 확인합니다.
        return;                               // 범위 밖이면 함수를 끝냅니다.
    }

    if (g_board[r][c] != EMPTY_TILE) {        // 클릭한 칸에 그림이 있는 경우입니다.
        if (g_removedTile != NO_REMOVED) {    // 이미 사라진 그림을 하나 보관 중인지 확인합니다.
            MessageBox(hWnd, L"이미 사라진 그림이 있습니다. 먼저 빈 칸에 추가하세요.", L"오류", MB_OK); // 중복 제거를 막는 메시지입니다.
            return;                           // 함수를 끝냅니다.
        }
        g_removedTile = g_board[r][c];        // 클릭한 칸의 조각 번호를 보관합니다.
        g_board[r][c] = EMPTY_TILE;           // 클릭한 칸의 그림을 사라지게 만듭니다.
        if (g_splitMode == SPLIT_GRID) {      // 일반 퍼즐이면 빈 칸 위치를 다시 확인해야 합니다.
            FindFirstEmpty();                 // 배열에서 첫 번째 빈 칸을 찾습니다.
        }
    }
    else {                                    // 클릭한 칸에 그림이 없는 경우입니다.
        if (g_removedTile == NO_REMOVED) {    // 추가할 사라진 그림이 없는지 확인합니다.
            MessageBox(hWnd, L"추가할 그림이 없습니다.", L"오류", MB_OK); // 추가할 조각이 없다는 오류 메시지입니다.
            return;                           // 함수를 끝냅니다.
        }
        g_board[r][c] = g_removedTile;        // 보관 중이던 그림 조각을 클릭한 빈 칸에 넣습니다.
        g_removedTile = NO_REMOVED;           // 보관 중인 사라진 그림 정보를 초기화합니다.
        if (g_splitMode == SPLIT_GRID) {      // 일반 퍼즐이면 빈 칸 위치를 다시 확인해야 합니다.
            FindFirstEmpty();                 // 배열에서 첫 번째 빈 칸을 찾습니다.
        }
    }

    InvalidateRect(hWnd, NULL, TRUE);          // 변경된 퍼즐 화면을 다시 그리도록 요청합니다.
    CheckSolvedAndFinish(hWnd);               // 제거 또는 복구 후 정답 상태인지 검사합니다.
}
