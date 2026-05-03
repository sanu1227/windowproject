#include <windows.h>                                       // 윈도우 API 헤더
#include <tchar.h>                                         // TCHAR 사용 헤더

#define MAX_LINES 10                                       // 최대 줄 수
#define MAX_COLS 30                                        // 한 줄 최대 글자 수
#define TAB_SIZE 4                                         // Tab 키 입력 시 공백 4칸

HINSTANCE g_hInst;                                         // 프로그램 인스턴스 핸들
LPCTSTR lpszClass = L"My Window Class";                    // 윈도우 클래스 이름
LPCTSTR lpszWindowName = L"Window Programming Lab";        // 창 제목

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);      // 메시지 처리 함수 선언

int FindPrevValidLine(int curLine, int len[]);             // 이전에 문자가 있는 줄 찾기
void MoveToNextLineStart(int* curLine, int* curCol, int len[], int* usedLines); // 다음 줄 맨 앞으로 이동
void OverwriteCharAt(TCHAR lines[][MAX_COLS + 1], int len[], int line, int col, TCHAR ch); // 덮어쓰기
void DeleteWordAtCaret(TCHAR lines[][MAX_COLS + 1], int len[], int line, int* col); // 단어 삭제
void InsertWithWrap(TCHAR lines[][MAX_COLS + 1], int len[], int line, int col, TCHAR ch, int* usedLines); // 삽입 후 넘치면 다음 줄로 넘기기

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow) // 프로그램 시작점
{
    HWND hWnd;                                             // 윈도우 핸들
    MSG Message;                                           // 메시지 구조체
    WNDCLASSEX WndClass;                                   // 윈도우 클래스 구조체

    g_hInst = hInstance;                                   // 전역 변수에 인스턴스 저장

    WndClass.cbSize = sizeof(WndClass);                    // 구조체 크기 설정
    WndClass.style = CS_HREDRAW | CS_VREDRAW;              // 크기 변경 시 다시 그림
    WndClass.lpfnWndProc = WndProc;                        // 메시지 처리 함수 지정
    WndClass.cbClsExtra = 0;                               // 클래스 추가 메모리 없음
    WndClass.cbWndExtra = 0;                               // 윈도우 추가 메모리 없음
    WndClass.hInstance = hInstance;                        // 인스턴스 핸들 저장
    WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);      // 기본 아이콘 사용
    WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);        // 기본 커서 사용
    WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); // 흰색 배경
    WndClass.lpszMenuName = NULL;                          // 메뉴 없음
    WndClass.lpszClassName = lpszClass;                    // 클래스 이름
    WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);    // 작은 아이콘

    RegisterClassEx(&WndClass);                            // 윈도우 클래스 등록

    hWnd = CreateWindow(                                   // 실제 창 생성
        lpszClass,                                         // 클래스 이름
        lpszWindowName,                                    // 창 제목
        WS_OVERLAPPEDWINDOW,                               // 창 스타일
        0, 0, 800, 600,                                    // 위치와 크기
        NULL, NULL, hInstance, NULL                        // 부모, 메뉴, 인스턴스, 추가 데이터
    );

    ShowWindow(hWnd, nCmdShow);                            // 창 보이기
    UpdateWindow(hWnd);                                    // 즉시 그리기

    while (GetMessage(&Message, NULL, 0, 0)) {             // 메시지 루프
        TranslateMessage(&Message);                        // 키보드 메시지 변환
        DispatchMessage(&Message);                         // WndProc으로 전달
    }

    return (int)Message.wParam;                            // 종료 코드 반환
}

int FindPrevValidLine(int curLine, int len[])              // 이전에 문자가 있는 줄 찾기
{
    int i;                                                 // 반복문 변수
    int line;                                              // 검사할 줄 번호

    for (i = 1; i <= MAX_LINES; i++) {                     // 최대 10줄 검사
        line = curLine - i;                                // 이전 줄 계산
        if (line < 0) {                                    // 음수가 되면
            line += MAX_LINES;                             // 마지막 줄로 순환
        }
        if (len[line] > 0) {                               // 해당 줄에 문자가 있으면
            return line;                                   // 그 줄 반환
        }
    }

    return curLine;                                        // 없으면 현재 줄 반환
}

void MoveToNextLineStart(int* curLine, int* curCol, int len[], int* usedLines) // 다음 줄 맨 앞으로 이동
{
    *curLine = *curLine + 1;                               // 줄 번호 1 증가
    if (*curLine >= MAX_LINES) {                           // 마지막 줄을 넘으면
        *curLine = 0;                                      // 0번째 줄로 순환
    }

    *curCol = 0;                                           // 열은 맨 앞

    if (*usedLines < MAX_LINES) {                          // 아직 최대 줄 수 미만이면
        if (*curLine + 1 > *usedLines) {                   // 새 줄을 처음 쓰는 경우
            *usedLines = *curLine + 1;                     // 사용 줄 수 갱신
        }
    }
}

void OverwriteCharAt(TCHAR lines[][MAX_COLS + 1], int len[], int line, int col, TCHAR ch) // 덮어쓰기 함수
{
    if (col >= MAX_COLS) {                                 // 범위를 넘으면
        return;                                            // 종료
    }

    lines[line][col] = ch;                                 // 현재 위치에 문자 저장

    if (col >= len[line]) {                                // 문자열 끝 뒤에 쓴 경우
        len[line] = col + 1;                               // 길이 갱신
        lines[line][len[line]] = '\0';                     // 문자열 끝 표시
    }
}

void DeleteWordAtCaret(TCHAR lines[][MAX_COLS + 1], int len[], int line, int* col) // 현재 단어 삭제
{
    int start;                                             // 삭제 시작 위치
    int end;                                               // 삭제 끝 위치
    int i;                                                 // 반복문 변수

    if (len[line] == 0) {                                  // 빈 줄이면
        return;                                            // 종료
    }

    if (*col >= len[line]) {                               // 캐럿이 끝 뒤에 있으면
        if (len[line] > 0) {                               // 문자가 있으면
            *col = len[line] - 1;                          // 마지막 문자 위치로 이동
        }
        else {                                             // 비어 있으면
            *col = 0;                                      // 0으로 맞춤
            return;                                        // 종료
        }
    }

    if (lines[line][*col] == ' ') {                        // 현재 위치가 공백이면
        while (*col < len[line] && lines[line][*col] == ' ') { // 공백 건너뜀
            (*col)++;                                      // 다음 칸으로 이동
        }
        if (*col >= len[line]) {                           // 끝까지 갔으면
            *col = len[line];                              // 줄 끝 위치
            return;                                        // 종료
        }
    }

    start = *col;                                          // 시작 위치 지정
    while (start > 0 && lines[line][start - 1] != ' ') {   // 단어 시작 찾기
        start--;                                           // 왼쪽으로 이동
    }

    end = *col;                                            // 끝 위치 지정
    while (end < len[line] && lines[line][end] != ' ') {   // 단어 끝 찾기
        end++;                                             // 오른쪽으로 이동
    }

    while (end < len[line] && lines[line][end] == ' ') {   // 뒤 공백도 정리
        end++;                                             // 계속 이동
    }

    for (i = start; i + (end - start) <= len[line]; i++) { // 뒤 문자 앞으로 당기기
        lines[line][i] = lines[line][i + (end - start)];   // 문자 복사
    }

    len[line] -= (end - start);                            // 길이 감소
    if (len[line] < 0) {                                   // 음수 방지
        len[line] = 0;                                     // 0으로 보정
    }

    lines[line][len[line]] = '\0';                         // 문자열 끝 표시

    if (start > len[line]) {                               // 시작 위치가 끝보다 뒤면
        *col = len[line];                                  // 줄 끝으로 이동
    }
    else {                                                 // 아니면
        *col = start;                                      // 삭제 시작 위치로 이동
    }
}

void InsertWithWrap(TCHAR lines[][MAX_COLS + 1], int len[], int line, int col, TCHAR ch, int* usedLines) // 삽입 후 넘치면 다음 줄로 넘김
{
    int currentLine;                                       // 현재 처리 중인 줄
    int currentCol;                                        // 현재 처리 중인 열
    int i;                                                 // 반복문 변수
    TCHAR carry;                                           // 다음 줄로 넘길 문자
    TCHAR temp;                                            // 임시 문자

    currentLine = line;                                    // 시작 줄 저장
    currentCol = col;                                      // 시작 열 저장
    carry = ch;                                            // 처음 삽입할 문자 저장

    if (currentCol > len[currentLine]) {                   // 캐럿이 문자열 끝 뒤면
        currentCol = len[currentLine];                     // 줄 끝으로 맞춤
    }

    while (1) {                                            // 넘김이 끝날 때까지 반복
        if (len[currentLine] < MAX_COLS) {                 // 현재 줄에 여유가 있으면
            for (i = len[currentLine]; i >= currentCol; i--) { // 뒤 문자 한 칸씩 밀기
                lines[currentLine][i + 1] = lines[currentLine][i]; // 오른쪽으로 이동
            }

            lines[currentLine][currentCol] = carry;        // 현재 위치에 문자 삽입
            len[currentLine]++;                            // 줄 길이 증가
            lines[currentLine][len[currentLine]] = '\0';   // 문자열 끝 표시
            break;                                         // 작업 완료
        }
        else {                                             // 현재 줄이 가득 찬 경우
            temp = lines[currentLine][MAX_COLS - 1];       // 맨 끝 문자를 임시 저장

            for (i = MAX_COLS - 1; i > currentCol; i--) {  // 삽입 위치 뒤 문자 오른쪽으로 밀기
                lines[currentLine][i] = lines[currentLine][i - 1]; // 한 칸씩 이동
            }

            lines[currentLine][currentCol] = carry;        // 삽입 위치에 문자 저장
            lines[currentLine][MAX_COLS] = '\0';           // 문자열 끝 유지
            len[currentLine] = MAX_COLS;                   // 길이는 30 유지

            carry = temp;                                  // 밀려난 맨 끝 문자를 다음 줄로 넘길 준비
            currentLine = currentLine + 1;                 // 다음 줄로 이동
            if (currentLine >= MAX_LINES) {                // 마지막 줄을 넘으면
                currentLine = 0;                           // 첫 줄로 순환
            }
            currentCol = 0;                                // 다음 줄 맨 앞에 삽입

            if (*usedLines < MAX_LINES) {                  // 아직 최대 줄 미만이면
                if (currentLine + 1 > *usedLines) {        // 새 줄을 처음 사용하면
                    *usedLines = currentLine + 1;          // 사용 줄 수 갱신
                }
            }
        }
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) // 메시지 처리 함수
{
    PAINTSTRUCT ps;                                        // 페인트 구조체
    HDC hDC;                                               // 디바이스 컨텍스트

    static TCHAR lines[MAX_LINES][MAX_COLS + 1];           // 각 줄 문자열 저장
    static int len[MAX_LINES];                             // 각 줄 길이 저장
    static int curLine = 0;                                // 현재 줄
    static int curCol = 0;                                 // 현재 열
    static int usedLines = 1;                              // 사용 중인 줄 수
    static int lineHeight = 20;                            // 줄 높이
    static int insertMode = 0;                             // 0: 덮어쓰기, 1: 삽입
    static int desiredCol = 0;                             // 상하 이동 시 유지할 열

    SIZE size;                                             // 픽셀 길이 계산용
    TEXTMETRIC tm;                                         // 폰트 정보
    int i;                                                 // 반복문 변수
    int targetLine;                                        // 이동 대상 줄
    int spacesToInsert;                                    // 탭 공백 수
    int j;                                                 // 반복문 변수

    switch (uMsg) {                                        // 메시지 분기

    case WM_CREATE:                                        // 창 생성 시
        CreateCaret(hWnd, NULL, 5, 15);                    // 캐럿 생성
        ShowCaret(hWnd);                                   // 캐럿 보이기

        for (i = 0; i < MAX_LINES; i++) {                  // 모든 줄 초기화
            lines[i][0] = '\0';                            // 빈 문자열
            len[i] = 0;                                    // 길이 0
        }

        curLine = 0;                                       // 시작 줄
        curCol = 0;                                        // 시작 열
        desiredCol = 0;                                    // 원하는 열 0
        usedLines = 1;                                     // 처음엔 1줄 사용
        insertMode = 0;                                    // 기본 덮어쓰기
        break;                                             // 종료

    case WM_KEYDOWN:                                       // 특수키 처리
        if (wParam == VK_LEFT) {                           // 왼쪽 화살표
            if (curCol > 0) {                              // 왼쪽 이동 가능하면
                curCol--;                                  // 한 칸 왼쪽
            }
            desiredCol = curCol;                           // 원하는 열 갱신
            InvalidateRect(hWnd, NULL, TRUE);              // 다시 그리기
            break;                                         // 종료
        }

        if (wParam == VK_RIGHT) {                          // 오른쪽 화살표
            if (curCol < len[curLine]) {                   // 줄 끝까지만 이동
                curCol++;                                  // 한 칸 오른쪽
            }
            desiredCol = curCol;                           // 원하는 열 갱신
            InvalidateRect(hWnd, NULL, TRUE);              // 다시 그리기
            break;                                         // 종료
        }

        if (wParam == VK_UP) {                             // 위 화살표
            targetLine = FindPrevValidLine(curLine, len);  // 이전 유효 줄 찾기

            if (targetLine != curLine) {                   // 이동할 줄이 있으면
                curLine = targetLine;                      // 그 줄로 이동
                if (desiredCol > len[curLine]) {           // 원하는 열이 길이보다 크면
                    curCol = len[curLine];                 // 줄 끝으로 조정
                }
                else {                                     // 아니면
                    curCol = desiredCol;                   // 열 유지
                }
            }

            InvalidateRect(hWnd, NULL, TRUE);              // 다시 그리기
            break;                                         // 종료
        }

        if (wParam == VK_DOWN) {                           // 아래 화살표
            targetLine = (curLine + 1) % MAX_LINES;        // 바로 다음 줄

            if (len[targetLine] > 0) {                     // 다음 줄에 문자가 있으면
                curLine = targetLine;                      // 이동
                if (desiredCol > len[curLine]) {           // 열이 길이보다 크면
                    curCol = len[curLine];                 // 줄 끝으로
                }
                else {                                     // 아니면
                    curCol = desiredCol;                   // 열 유지
                }
            }
            else {                                         // 다음 줄이 비어 있으면
                curLine = targetLine;                      // 다음 줄로 이동
                curCol = 0;                                // 맨 앞으로 이동

                if (usedLines < MAX_LINES) {               // 사용 줄 수 갱신
                    if (curLine + 1 > usedLines) {
                        usedLines = curLine + 1;
                    }
                }
            }

            desiredCol = curCol;                           // 원하는 열 갱신
            InvalidateRect(hWnd, NULL, TRUE);              // 다시 그리기
            break;                                         // 종료
        }

        if (wParam == VK_HOME) {                           // Home 키
            curCol = 0;                                    // 줄 맨 앞
            desiredCol = curCol;                           // 원하는 열 갱신
            InvalidateRect(hWnd, NULL, TRUE);              // 다시 그리기
            break;                                         // 종료
        }

        if (wParam == VK_END) {                            // End 키
            if (len[curLine] >= MAX_COLS) {                // 줄이 가득 차 있으면
                MoveToNextLineStart(&curLine, &curCol, len, &usedLines); // 다음 줄 맨 앞으로
            }
            else {                                         // 아니면
                curCol = len[curLine];                     // 현재 줄 끝
            }
            desiredCol = curCol;                           // 원하는 열 갱신
            InvalidateRect(hWnd, NULL, TRUE);              // 다시 그리기
            break;                                         // 종료
        }

        if (wParam == VK_INSERT) {                         // Insert 키
            if (insertMode == 0) {                         // 현재 덮어쓰기면
                insertMode = 1;                            // 삽입 모드로 변경
            }
            else {                                         // 현재 삽입이면
                insertMode = 0;                            // 덮어쓰기로 변경
            }
            InvalidateRect(hWnd, NULL, TRUE);              // 다시 그리기
            break;                                         // 종료
        }

        if (wParam == VK_DELETE) {                         // Delete 키
            DeleteWordAtCaret(lines, len, curLine, &curCol); // 현재 단어 삭제
            if (curCol > len[curLine]) {                   // 열 보정
                curCol = len[curLine];
            }
            desiredCol = curCol;                           // 원하는 열 갱신
            InvalidateRect(hWnd, NULL, TRUE);              // 다시 그리기
            break;                                         // 종료
        }

        if (wParam == VK_PRIOR) {                          // PgUp 키
            curLine = curLine - 3;                         // 3줄 위로
            while (curLine < 0) {                          // 음수면
                curLine += MAX_LINES;                      // 순환 보정
            }

            if (desiredCol > len[curLine]) {               // 열이 길이보다 크면
                curCol = len[curLine];                     // 줄 끝으로
            }
            else {                                         // 아니면
                curCol = desiredCol;                       // 열 유지
            }

            InvalidateRect(hWnd, NULL, TRUE);              // 다시 그리기
            break;                                         // 종료
        }

        if (wParam == VK_NEXT) {                           // PgDn 키
            curLine = (curLine + 3) % MAX_LINES;           // 3줄 아래로 순환

            if (desiredCol > len[curLine]) {               // 열이 길이보다 크면
                curCol = len[curLine];                     // 줄 끝으로
            }
            else {                                         // 아니면
                curCol = desiredCol;                       // 열 유지
            }

            InvalidateRect(hWnd, NULL, TRUE);              // 다시 그리기
            break;                                         // 종료
        }

        break;                                             // WM_KEYDOWN 종료

    case WM_CHAR:                                          // 문자 입력 처리
        if (wParam == VK_ESCAPE) {                         // ESC 키
            DestroyWindow(hWnd);                           // 창 닫기
            break;                                         // 종료
        }

        if (wParam == VK_BACK) {                           // 백스페이스
            if (curCol > 0) {                              // 현재 줄 안에서 삭제 가능
                for (j = curCol - 1; j < len[curLine]; j++) { // 뒤 문자 앞으로 당김
                    lines[curLine][j] = lines[curLine][j + 1]; // 복사
                }
                curCol--;                                  // 왼쪽으로 이동
                len[curLine]--;                            // 길이 감소
                if (len[curLine] < 0) {                    // 음수 방지
                    len[curLine] = 0;
                }
                lines[curLine][len[curLine]] = '\0';       // 문자열 끝 표시
            }
            else {                                         // 줄 맨 앞이면
                targetLine = FindPrevValidLine(curLine, len); // 이전 유효 줄 찾기
                if (targetLine != curLine) {               // 있으면
                    curLine = targetLine;                  // 그 줄로 이동
                    curCol = len[curLine];                 // 줄 끝으로 이동
                    if (curCol > 0) {                      // 삭제 가능하면
                        curCol--;                          // 마지막 문자 위치
                        lines[curLine][curCol] = '\0';     // 마지막 문자 삭제
                        len[curLine] = curCol;             // 길이 갱신
                    }
                }
            }

            desiredCol = curCol;                           // 원하는 열 갱신
            InvalidateRect(hWnd, NULL, TRUE);              // 다시 그리기
            break;                                         // 종료
        }

        if (wParam == VK_RETURN) {                         // 엔터
            MoveToNextLineStart(&curLine, &curCol, len, &usedLines); // 다음 줄 맨 앞
            desiredCol = curCol;                           // 원하는 열 갱신
            InvalidateRect(hWnd, NULL, TRUE);              // 다시 그리기
            break;                                         // 종료
        }

        if (wParam == VK_TAB) {                            // Tab 키
            spacesToInsert = TAB_SIZE;                     // 공백 4칸

            for (i = 0; i < spacesToInsert; i++) {         // 4번 반복
                if (insertMode == 1) {                     // 삽입 모드면
                    InsertWithWrap(lines, len, curLine, curCol, ' ', &usedLines); // 공백 삽입+줄넘김
                }
                else {                                     // 덮어쓰기 모드면
                    if (curCol >= MAX_COLS) {              // 현재 줄 끝이면
                        MoveToNextLineStart(&curLine, &curCol, len, &usedLines); // 다음 줄로 이동
                    }
                    OverwriteCharAt(lines, len, curLine, curCol, ' '); // 공백 덮어쓰기
                }

                curCol++;                                  // 캐럿 오른쪽 이동
                if (curCol > MAX_COLS) {                   // 30자 넘으면
                    MoveToNextLineStart(&curLine, &curCol, len, &usedLines); // 다음 줄 시작
                }
            }

            desiredCol = curCol;                           // 원하는 열 갱신
            InvalidateRect(hWnd, NULL, TRUE);              // 다시 그리기
            break;                                         // 종료
        }

        if (wParam >= 32 && wParam <= 126) {               // 출력 가능한 일반 문자
            if (insertMode == 1) {                         // 삽입 모드면
                InsertWithWrap(lines, len, curLine, curCol, (TCHAR)wParam, &usedLines); // 삽입+줄넘김
            }
            else {                                         // 덮어쓰기 모드면
                if (curCol >= MAX_COLS) {                  // 현재 줄 끝이면
                    MoveToNextLineStart(&curLine, &curCol, len, &usedLines); // 다음 줄 시작
                }
                OverwriteCharAt(lines, len, curLine, curCol, (TCHAR)wParam); // 덮어쓰기
            }

            curCol++;                                      // 캐럿 오른쪽 이동
            if (curCol > MAX_COLS) {                       // 줄 끝을 넘으면
                MoveToNextLineStart(&curLine, &curCol, len, &usedLines); // 다음 줄 맨 앞으로 이동
            }

            if (usedLines < MAX_LINES) {                   // 사용 줄 수 갱신
                if (curLine + 1 > usedLines) {
                    usedLines = curLine + 1;
                }
            }

            desiredCol = curCol;                           // 원하는 열 갱신
            InvalidateRect(hWnd, NULL, TRUE);              // 다시 그리기
        }
        break;                                             // WM_CHAR 종료

    case WM_PAINT:                                         // 다시 그리기
        hDC = BeginPaint(hWnd, &ps);                       // 그리기 시작

        GetTextMetrics(hDC, &tm);                          // 폰트 정보 얻기
        lineHeight = tm.tmHeight + 4;                      // 줄 높이 계산

        for (i = 0; i < usedLines; i++) {                  // 사용된 줄 출력
            TextOut(hDC, 0, i * lineHeight, lines[i], len[i]); // 각 줄 출력
        }

        GetTextExtentPoint32(hDC, lines[curLine], curCol, &size); // 현재 줄에서 curCol까지 픽셀 길이 구함
        SetCaretPos(size.cx, curLine * lineHeight);        // 캐럿 위치 설정

        EndPaint(hWnd, &ps);                               // 그리기 종료
        break;                                             // 종료

    case WM_DESTROY:                                       // 창 종료
        HideCaret(hWnd);                                   // 캐럿 숨기기
        DestroyCaret();                                    // 캐럿 제거
        PostQuitMessage(0);                                // 프로그램 종료 메시지
        return 0;                                          // 종료 반환
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);      // 기본 처리
}