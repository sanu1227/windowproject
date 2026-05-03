#include <windows.h>   // 윈도우 API 사용을 위한 헤더
#include <tchar.h>     // TCHAR (유니코드/멀티바이트 호환 문자열) 사용

// 프로그램 인스턴스 핸들 (현재 실행 중인 프로그램 자체)
HINSTANCE g_hInst;

// 윈도우 클래스 이름 (창의 종류)
LPCTSTR lpszClass = L"My Window Class";

// 실제 창 제목 (타이틀 바에 표시됨)
LPCTSTR lpszWindowName = L"Window Programming Lab";

// 윈도우 메시지 처리 함수 선언
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// 프로그램 시작점 (main 대신 사용)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpszCmdParam, int nCmdShow)
{
    HWND hWnd;            // 생성될 윈도우 핸들
    MSG Message;          // 메시지 저장 구조체
    WNDCLASSEX WndClass; // 윈도우 클래스 정보 구조체

    g_hInst = hInstance;  // 전역 변수에 인스턴스 저장

    // 윈도우 클래스 구조체 설정
    WndClass.cbSize = sizeof(WndClass);              // 구조체 크기
    WndClass.style = CS_HREDRAW | CS_VREDRAW;        // 크기 변경 시 다시 그림
    WndClass.lpfnWndProc = WndProc;                  // 메시지 처리 함수 연결
    WndClass.cbClsExtra = 0;                         // 추가 메모리 없음
    WndClass.cbWndExtra = 0;                         // 추가 메모리 없음
    WndClass.hInstance = hInstance;                  // 인스턴스 설정
    WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION); // 기본 아이콘
    WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);   // 기본 커서
    WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); // 배경색 흰색
    WndClass.lpszMenuName = NULL;                    // 메뉴 없음
    WndClass.lpszClassName = lpszClass;              // 클래스 이름
    WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION); // 작은 아이콘

    RegisterClassEx(&WndClass); // 윈도우 클래스 등록

    // 실제 윈도우 생성
    hWnd = CreateWindow(
        lpszClass, lpszWindowName, WS_OVERLAPPEDWINDOW, // 클래스, 제목, 스타일
        0, 0, 800, 600,                                 // 위치와 크기
        NULL, NULL, hInstance, NULL                     // 부모, 메뉴, 인스턴스
    );

    ShowWindow(hWnd, nCmdShow); // 창 화면에 표시
    UpdateWindow(hWnd);         // 즉시 WM_PAINT 발생

    // 메시지 루프 (프로그램이 계속 돌아가게 하는 핵심 부분)
    while (GetMessage(&Message, 0, 0, 0)) {
        TranslateMessage(&Message);   // 키보드 메시지 변환
        DispatchMessage(&Message);    // WndProc으로 전달
    }

    return (int)Message.wParam; // 종료 코드 반환
}


// 모든 이벤트 처리 함수
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;  // 그림 그릴 때 사용하는 구조체
    HDC hDC;         // 화면에 그리기 위한 핸들

#define MAX_LINES 10   // 최대 줄 수
#define MAX_COLS 30    // 한 줄 최대 글자 수

    // 문자열 저장 배열 (10줄 × 30자 + '\0')
    static TCHAR lines[MAX_LINES][MAX_COLS + 1];

    // 각 줄의 현재 길이 저장
    static int len[MAX_LINES];

    // 현재 입력 중인 줄
    static int curLine = 0;

    // 현재 입력 중인 위치 (열)
    static int curCol = 0;

    // 현재 화면에 사용된 줄 수
    static int usedLines = 1;

    // 줄 높이 (픽셀 단위)
    static int lineHeight = 20;

    SIZE size; // 문자열의 픽셀 길이 저장용
    int i;     // 반복문용 변수

    switch (uMsg) {

    case WM_CREATE: // 윈도우 생성 시 실행
        CreateCaret(hWnd, NULL, 5, 15); // 캐럿 생성 (폭 5, 높이 15)
        ShowCaret(hWnd);                // 캐럿 표시

        // 모든 줄 초기화
        for (i = 0; i < MAX_LINES; i++) {
            lines[i][0] = '\0'; // 문자열 비움
            len[i] = 0;         // 길이 0
        }

        curLine = 0;   // 시작 줄
        curCol = 0;    // 시작 위치
        usedLines = 1; // 처음엔 1줄만 사용
        break;

    case WM_CHAR: // 키보드 입력 발생

        if (wParam == VK_ESCAPE) { // ESC 누르면 초기화
            for (i = 0; i < MAX_LINES; i++) {
                lines[i][0] = '\0'; // 문자열 비움
                len[i] = 0;         // 길이 0
            }

            curLine = 0;   // 시작 줄
            curCol = 0;    // 시작 위치
            usedLines = 1; // 처음엔 1줄만 사용
            InvalidateRect(hWnd, NULL, TRUE);
            break;
        }

        // 백스페이스 처리
        if (wParam == VK_BACK) {
            if (curCol > 0) { // 현재 줄에서 삭제 가능
                curCol--;                           // 위치 한 칸 뒤로
                lines[curLine][curCol] = '\0';      // 문자 삭제
                len[curLine] = curCol;              // 길이 갱신
            }
            else { // 줄 맨 앞일 경우
                if (curLine > 0)      // 이전 줄로 이동
                    curLine--;
                else
                    curLine = MAX_LINES - 1; // 첫 줄이면 마지막 줄로

                curCol = len[curLine]; // 이전 줄 끝으로 이동

                if (curCol > 0) {
                    curCol--;
                    lines[curLine][curCol] = '\0';
                    len[curLine] = curCol;
                }
            }

            InvalidateRect(hWnd, NULL, TRUE); // 화면 다시 그리기 요청
            break;
        }

        // 엔터 처리
        if (wParam == VK_RETURN) {
            curLine++; // 다음 줄로 이동

            if (curLine >= MAX_LINES)
                curLine = 0; // 10줄 넘으면 0으로

            curCol = 0; // ⭐ 항상 처음부터 입력

            // 사용 줄 수 증가 처리
            if (usedLines < MAX_LINES) {
                if (curLine + 1 > usedLines)
                    usedLines = curLine + 1;
            }

            InvalidateRect(hWnd, NULL, TRUE);
            break;
        }

        // 일반 문자 입력 (출력 가능한 ASCII)
        if (wParam >= 32 && wParam <= 126) {

            // 한 줄이 꽉 찼으면 자동 줄바꿈
            if (curCol >= MAX_COLS) {
                curLine++; // 다음 줄

                if (curLine >= MAX_LINES)
                    curLine = 0; // 다시 처음으로

                curCol = 0; // ⭐ 처음부터 덮어쓰기

                if (usedLines < MAX_LINES) {
                    if (curLine + 1 > usedLines)
                        usedLines = curLine + 1;
                }
            }

            // 문자 저장
            lines[curLine][curCol] = (TCHAR)wParam;
            curCol++; // 다음 칸으로 이동

            // 길이 갱신
            if (curCol > len[curLine])
                len[curLine] = curCol;

            // 문자열 끝 표시
            lines[curLine][len[curLine]] = '\0';

            // 사용 줄 수 갱신
            if (usedLines < MAX_LINES) {
                if (curLine + 1 > usedLines)
                    usedLines = curLine + 1;
            }

            InvalidateRect(hWnd, NULL, TRUE); // 다시 그리기
        }

        break;

    case WM_PAINT: // 화면 다시 그리기
        hDC = BeginPaint(hWnd, &ps); // 그리기 시작

        // 폰트 높이 가져오기
        {
            TEXTMETRIC tm;
            GetTextMetrics(hDC, &tm);
            lineHeight = tm.tmHeight + 4; // 줄 간격 설정
        }

        // 모든 줄 출력
        for (i = 0; i < usedLines; i++) {
            TextOut(hDC, 0, i * lineHeight, lines[i], len[i]);
        }

        // 현재 커서 위치 계산
        GetTextExtentPoint32(hDC, lines[curLine], curCol, &size);

        // 캐럿 위치 설정
        SetCaretPos(size.cx, curLine * lineHeight);

        EndPaint(hWnd, &ps); // 그리기 종료
        break;

    case WM_DESTROY: // 프로그램 종료 시
        HideCaret(hWnd);        // 캐럿 숨김
        DestroyCaret();         // 캐럿 제거
        PostQuitMessage(0);     // 메시지 루프 종료
        return 0;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam); // 기본 처리
}