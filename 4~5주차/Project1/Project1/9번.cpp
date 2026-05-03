#include <windows.h>                         // 윈도우 API 사용
#include <tchar.h>                           // TCHAR, LPCTSTR 사용

#define MAX_LINES 10                         // 최대 줄 수
#define MAX_COLS 30                          // 한 줄 최대 글자 수
#define TAB_SIZE 4                           // Tab 입력 시 공백 4칸
#define DISP_BUF 1024                        // 화면 출력용 임시 버퍼 크기

HINSTANCE g_hInst;                           // 프로그램 인스턴스 핸들
LPCTSTR lpszClass = L"My Window Class";      // 윈도우 클래스 이름
LPCTSTR lpszWindowName = L"Window Programming Lab"; // 창 제목

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM); // 메시지 처리 함수 선언

/* ---------------- 전역 데이터 ---------------- */

TCHAR g_lines[MAX_LINES][MAX_COLS + 1];      // 실제 입력된 문자열 저장
int g_len[MAX_LINES];                        // 각 줄의 현재 길이 저장

int g_curLine = 0;                           // 현재 캐럿이 있는 줄 번호
int g_curCol = 0;                            // 현재 캐럿이 있는 열 번호
int g_usedLines = 1;                         // 현재 사용 중인 줄 수
int g_lineHeight = 20;                       // 줄 높이
int g_insertMode = 0;                        // 0: 덮어쓰기 모드, 1: 삽입 모드
int g_desiredCol = 0;                        // 위/아래 이동할 때 유지하고 싶은 열 위치

int g_upperInputMode = 0;                    // F1 상태: 0이면 소문자, 1이면 대문자 입력
int g_f2Mode = 0;                            // F2 상태
int g_f3Mode = 0;                            // F3 상태
int g_f4Mode = 0;                            // F4 상태
int g_f5Mode = 0;                            // F5 상태

/* ---------------- 문자열 관련 함수 ---------------- */

int StringLen(const TCHAR str[])             // 문자열 길이 구하기
{
    int i = 0;                               // 인덱스 변수

    while (str[i] != '\0') {                 // 문자열 끝까지 반복
        i++;                                 // 한 글자씩 이동
    }

    return i;                                // 길이 반환
}

void CopyString(TCHAR dest[], const TCHAR src[]) // 문자열 복사
{
    int i = 0;                               // 인덱스 변수

    while (src[i] != '\0') {                 // 원본 문자열 끝까지 반복
        dest[i] = src[i];                    // 한 글자씩 복사
        i++;                                 // 다음 위치로 이동
    }

    dest[i] = '\0';                          // 마지막에 문자열 끝 표시
}

int IsDigitChar(TCHAR ch)                    // 숫자인지 검사
{
    return (ch >= '0' && ch <= '9');         // 0~9면 참
}

int IsAlphaLower(TCHAR ch)                   // 소문자인지 검사
{
    return (ch >= 'a' && ch <= 'z');         // a~z면 참
}

int IsAlphaUpper(TCHAR ch)                   // 대문자인지 검사
{
    return (ch >= 'A' && ch <= 'Z');         // A~Z면 참
}

int IsAlphaChar(TCHAR ch)                    // 영문자인지 검사
{
    return IsAlphaLower(ch) || IsAlphaUpper(ch); // 소문자 또는 대문자면 참
}

int IsAlphaNumChar(TCHAR ch)                 // 영문자 또는 숫자인지 검사
{
    return IsAlphaChar(ch) || IsDigitChar(ch);   // 영문자이거나 숫자면 참
}

TCHAR ToUpperChar(TCHAR ch)                  // 대문자로 변환
{
    if (IsAlphaLower(ch)) {                  // 소문자면
        return ch - ('a' - 'A');             // 대문자로 바꿔 반환
    }

    return ch;                               // 아니면 그대로 반환
}

TCHAR ToLowerChar(TCHAR ch)                  // 소문자로 변환
{
    if (IsAlphaUpper(ch)) {                  // 대문자면
        return ch + ('a' - 'A');             // 소문자로 바꿔 반환
    }

    return ch;                               // 아니면 그대로 반환
}

/* ---------------- 편집 관련 함수 ---------------- */

void ResetAllText()                          // 전체 초기화 함수
{
    int i;                                   // 반복 변수

    for (i = 0; i < MAX_LINES; i++) {        // 모든 줄에 대해 반복
        g_lines[i][0] = '\0';                // 문자열 비우기
        g_len[i] = 0;                        // 길이 0으로 초기화
    }

    g_curLine = 0;                           // 현재 줄을 0으로 초기화
    g_curCol = 0;                            // 현재 열을 0으로 초기화
    g_usedLines = 1;                         // 사용 줄 수를 1로 초기화
    g_desiredCol = 0;                        // 원하는 열도 0으로 초기화
}

int FindPrevValidLine(int curLine)           // 이전에 글자가 있는 줄 찾기
{
    int i;                                   // 반복 변수
    int line;                                // 검사할 줄 번호

    for (i = 1; i <= MAX_LINES; i++) {       // 최대 줄 수만큼 검사
        line = curLine - i;                  // 현재 줄보다 이전 줄 계산

        if (line < 0) {                      // 음수가 되면
            line += MAX_LINES;               // 마지막 줄로 순환
        }

        if (g_len[line] > 0) {               // 해당 줄에 글자가 있으면
            return line;                     // 그 줄 번호 반환
        }
    }

    return curLine;                          // 없으면 현재 줄 반환
}

void MoveToNextLineStart()                   // 다음 줄 맨 앞으로 이동
{
    g_curLine++;                             // 줄 번호 1 증가

    if (g_curLine >= MAX_LINES) {            // 마지막 줄을 넘으면
        g_curLine = 0;                       // 첫 줄로 순환
    }

    g_curCol = 0;                            // 열은 맨 앞으로 이동

    if (g_usedLines < MAX_LINES) {           // 아직 최대 줄 수보다 작으면
        if (g_curLine + 1 > g_usedLines) {   // 새 줄을 처음 사용하는 경우
            g_usedLines = g_curLine + 1;     // 사용 줄 수 갱신
        }
    }
}

void OverwriteCharAtCurrent(TCHAR ch)        // 현재 위치에 문자 덮어쓰기
{
    if (g_curCol >= MAX_COLS) {              // 열이 최대 범위를 넘으면
        return;                              // 종료
    }

    g_lines[g_curLine][g_curCol] = ch;       // 현재 줄, 현재 열에 문자 저장

    if (g_curCol >= g_len[g_curLine]) {      // 기존 문자열 끝 뒤에 쓴 경우
        g_len[g_curLine] = g_curCol + 1;     // 길이 갱신
        g_lines[g_curLine][g_len[g_curLine]] = '\0'; // 문자열 끝 표시
    }
}

void DeleteWordAtCaret()                     // Delete 키: 현재 단어 삭제
{
    int start;                               // 단어 시작 위치
    int end;                                 // 단어 끝 위치
    int i;                                   // 반복 변수

    if (g_len[g_curLine] == 0) {             // 현재 줄이 비어 있으면
        return;                              // 삭제할 것이 없으므로 종료
    }

    if (g_curCol >= g_len[g_curLine]) {      // 캐럿이 문자열 끝 뒤에 있으면
        if (g_len[g_curLine] > 0) {          // 문자가 하나라도 있으면
            g_curCol = g_len[g_curLine] - 1; // 마지막 문자 위치로 이동
        }
        else {                               // 완전히 빈 줄이면
            g_curCol = 0;                    // 열 0으로 맞추고
            return;                          // 종료
        }
    }

    if (g_lines[g_curLine][g_curCol] == ' ') { // 현재 위치가 공백이면
        while (g_curCol < g_len[g_curLine] && g_lines[g_curLine][g_curCol] == ' ') {
            g_curCol++;                      // 공백을 건너뛰어 다음 단어로 이동
        }

        if (g_curCol >= g_len[g_curLine]) {  // 끝까지 갔으면
            g_curCol = g_len[g_curLine];     // 줄 끝에 두고
            return;                          // 종료
        }
    }

    start = g_curCol;                        // 시작 위치를 현재 캐럿으로 설정
    while (start > 0 && g_lines[g_curLine][start - 1] != ' ') {
        start--;                             // 왼쪽으로 이동해 단어 시작 찾기
    }

    end = g_curCol;                          // 끝 위치도 현재 캐럿으로 시작
    while (end < g_len[g_curLine] && g_lines[g_curLine][end] != ' ') {
        end++;                               // 오른쪽으로 이동해 단어 끝 찾기
    }

    while (end < g_len[g_curLine] && g_lines[g_curLine][end] == ' ') {
        end++;                               // 뒤쪽 공백까지 함께 삭제
    }

    for (i = start; i + (end - start) <= g_len[g_curLine]; i++) {
        g_lines[g_curLine][i] = g_lines[g_curLine][i + (end - start)]; // 뒤 문자열을 앞으로 당김
    }

    g_len[g_curLine] -= (end - start);       // 줄 길이 감소

    if (g_len[g_curLine] < 0) {              // 혹시 음수가 되면
        g_len[g_curLine] = 0;                // 0으로 보정
    }

    g_lines[g_curLine][g_len[g_curLine]] = '\0'; // 문자열 끝 표시

    if (start > g_len[g_curLine]) {          // start가 줄 길이를 넘으면
        g_curCol = g_len[g_curLine];         // 캐럿을 줄 끝으로 이동
    }
    else {
        g_curCol = start;                    // 아니면 삭제 시작 위치로 이동
    }
}

void InsertWithWrapAtCurrent(TCHAR ch)       // 삽입 모드에서 문자 입력
{
    int currentLine;                         // 현재 작업 중인 줄
    int currentCol;                          // 현재 작업 중인 열
    int i;                                   // 반복 변수
    TCHAR carry;                             // 다음 줄로 넘길 문자
    TCHAR temp;                              // 임시 저장 문자

    currentLine = g_curLine;                 // 시작 줄 저장
    currentCol = g_curCol;                   // 시작 열 저장
    carry = ch;                              // 처음 삽입할 문자 저장

    if (currentCol > g_len[currentLine]) {   // 캐럿이 문자열 끝 뒤에 있으면
        currentCol = g_len[currentLine];     // 줄 끝으로 보정
    }

    while (1) {                              // 삽입이 끝날 때까지 반복
        if (g_len[currentLine] < MAX_COLS) { // 현재 줄에 공간이 있으면
            for (i = g_len[currentLine]; i >= currentCol; i--) {
                g_lines[currentLine][i + 1] = g_lines[currentLine][i]; // 뒤 문자를 한 칸씩 오른쪽으로 밀기
            }

            g_lines[currentLine][currentCol] = carry; // 현재 위치에 문자 삽입
            g_len[currentLine]++;             // 길이 1 증가
            g_lines[currentLine][g_len[currentLine]] = '\0'; // 문자열 끝 표시
            break;                            // 작업 완료
        }
        else {                                // 현재 줄이 꽉 찬 경우
            temp = g_lines[currentLine][MAX_COLS - 1]; // 마지막 문자 저장

            for (i = MAX_COLS - 1; i > currentCol; i--) {
                g_lines[currentLine][i] = g_lines[currentLine][i - 1]; // 삽입 위치 뒤를 오른쪽으로 밀기
            }

            g_lines[currentLine][currentCol] = carry; // 현재 줄에 문자 삽입
            g_lines[currentLine][MAX_COLS] = '\0';    // 문자열 끝 유지
            g_len[currentLine] = MAX_COLS;            // 길이는 최대 길이 유지

            carry = temp;                     // 밀려난 문자를 다음 줄로 넘김
            currentLine++;                    // 다음 줄로 이동

            if (currentLine >= MAX_LINES) {  // 마지막 줄을 넘으면
                currentLine = 0;             // 첫 줄로 순환
            }

            currentCol = 0;                  // 다음 줄 맨 앞에서 삽입 시작

            if (g_usedLines < MAX_LINES) {   // 아직 최대 줄 수 전이라면
                if (currentLine + 1 > g_usedLines) {
                    g_usedLines = currentLine + 1; // 사용 줄 수 갱신
                }
            }
        }
    }
}

void BackspaceAtCurrent()                    // Backspace 처리
{
    int j;                                   // 반복 변수
    int targetLine;                          // 이전 줄 번호

    if (g_curCol > 0) {                      // 현재 줄 맨 앞이 아니라면
        for (j = g_curCol - 1; j < g_len[g_curLine]; j++) {
            g_lines[g_curLine][j] = g_lines[g_curLine][j + 1]; // 삭제한 뒤 문자열을 앞으로 당김
        }

        g_curCol--;                          // 캐럿 왼쪽으로 이동
        g_len[g_curLine]--;                  // 길이 1 감소

        if (g_len[g_curLine] < 0) {          // 음수 방지
            g_len[g_curLine] = 0;
        }

        g_lines[g_curLine][g_len[g_curLine]] = '\0'; // 문자열 끝 표시
    }
    else {                                   // 줄 맨 앞에서 Backspace를 누른 경우
        targetLine = FindPrevValidLine(g_curLine); // 이전에 글자가 있는 줄 찾기

        if (targetLine != g_curLine) {       // 이전 줄이 존재하면
            g_curLine = targetLine;          // 그 줄로 이동
            g_curCol = g_len[g_curLine];     // 그 줄 끝으로 이동

            if (g_curCol > 0) {              // 삭제할 문자가 있으면
                g_curCol--;                  // 마지막 문자 위치로 이동
                g_lines[g_curLine][g_curCol] = '\0'; // 마지막 문자 삭제
                g_len[g_curLine] = g_curCol; // 길이 갱신
            }
        }
    }
}

void InsertTabSpaces()                       // Tab 키 처리
{
    int i;                                   // 반복 변수

    for (i = 0; i < TAB_SIZE; i++) {         // 공백 4칸 삽입
        if (g_insertMode == 1) {             // 삽입 모드이면
            InsertWithWrapAtCurrent(' ');    // 삽입 방식으로 공백 추가
        }
        else {                               // 덮어쓰기 모드이면
            if (g_curCol >= MAX_COLS) {      // 현재 줄 끝에 도달했으면
                MoveToNextLineStart();       // 다음 줄로 이동
            }
            OverwriteCharAtCurrent(' ');     // 현재 위치에 공백 덮어쓰기
        }

        g_curCol++;                          // 캐럿 오른쪽 이동

        if (g_curCol > MAX_COLS) {           // 범위를 넘으면
            MoveToNextLineStart();           // 다음 줄 시작으로 이동
        }
    }
}

void InputNormalChar(TCHAR ch)               // 일반 문자 입력 처리
{
    if (g_upperInputMode && IsAlphaChar(ch)) {   // F1이 켜져 있고 영문자면
        ch = ToUpperChar(ch);                    // 대문자로 변환
    }
    else if (!g_upperInputMode && IsAlphaChar(ch)) { // F1이 꺼져 있고 영문자면
        ch = ToLowerChar(ch);                    // 소문자로 변환
    }

    if (g_insertMode == 1) {                 // 삽입 모드이면
        InsertWithWrapAtCurrent(ch);         // 삽입 처리
    }
    else {                                   // 덮어쓰기 모드이면
        if (g_curCol >= MAX_COLS) {          // 줄 끝에 도달했으면
            MoveToNextLineStart();           // 다음 줄로 이동
        }
        OverwriteCharAtCurrent(ch);          // 현재 위치에 덮어쓰기
    }

    g_curCol++;                              // 입력 후 캐럿 오른쪽 이동

    if (g_curCol > MAX_COLS) {               // 범위를 넘으면
        MoveToNextLineStart();               // 다음 줄 시작으로 이동
    }

    if (g_usedLines < MAX_LINES) {           // 아직 최대 줄 수를 다 안 썼다면
        if (g_curLine + 1 > g_usedLines) {   // 새 줄을 처음 사용하면
            g_usedLines = g_curLine + 1;     // 사용 줄 수 갱신
        }
    }
}

/* ---------------- F키 화면 효과 함수 ---------------- */

void ApplyF2(const TCHAR src[], TCHAR dest[]) // 숫자 앞에 **** 추가
{
    int i, k = 0;                            // i는 원본 인덱스, k는 결과 인덱스

    for (i = 0; src[i] != '\0'; i++) {       // 문자열 끝까지 반복
        if (IsDigitChar(src[i])) {           // 현재 문자가 숫자면
            dest[k++] = '*';                 // 별 1개
            dest[k++] = '*';                 // 별 2개
            dest[k++] = '*';                 // 별 3개
            dest[k++] = '*';                 // 별 4개
        }
        dest[k++] = src[i];                  // 원래 문자도 복사
    }

    dest[k] = '\0';                          // 문자열 끝 표시
}

void ApplyF3(const TCHAR src[], TCHAR dest[]) // 단어를 괄호로 감싸고 대문자로 출력
{
    int i = 0;                               // 원본 문자열 인덱스
    int k = 0;                               // 결과 문자열 인덱스

    while (src[i] != '\0') {                 // 문자열 끝까지 반복
        if (src[i] == ' ') {                 // 공백이면
            i++;                             // 건너뜀
            continue;
        }

        {
            int start = i;                   // 단어 시작 위치
            int end = i;                     // 단어 끝 위치
            int coreEnd;                     // 실제 단어 본체 끝 위치

            while (src[end] != '\0' && src[end] != ' ') {
                end++;                       // 공백 전까지 이동
            }

            coreEnd = end - 1;              // 일단 마지막 위치를 끝으로 가정
            while (coreEnd >= start && !IsAlphaNumChar(src[coreEnd])) {
                coreEnd--;                  // 뒤쪽 문장부호 제외
            }

            if (coreEnd >= start) {         // 실제 단어가 존재하면
                dest[k++] = '(';            // 앞 괄호 추가

                while (start <= coreEnd) {  // 단어 본체를
                    dest[k++] = ToUpperChar(src[start]); // 대문자로 바꿔 저장
                    start++;
                }

                dest[k++] = ')';            // 뒤 괄호 추가

                while (coreEnd + 1 < end) { // 뒤 문장부호가 있으면
                    coreEnd++;
                    dest[k++] = src[coreEnd]; // 그대로 붙임
                }
            }
            else {                          // 문장부호만 있는 경우
                while (start < end) {
                    dest[k++] = src[start]; // 그대로 복사
                    start++;
                }
            }

            i = end;                        // 다음 단어 시작 위치로 이동
        }
    }

    dest[k] = '\0';                         // 문자열 끝 표시
}

void ApplyF4(const TCHAR src[], TCHAR dest[]) // 공백 제거 + 소문자 출력
{
    int i, k = 0;                            // i는 원본 인덱스, k는 결과 인덱스

    for (i = 0; src[i] != '\0'; i++) {       // 문자열 끝까지 반복
        if (src[i] != ' ') {                 // 공백이 아니면
            dest[k++] = ToLowerChar(src[i]); // 소문자로 바꿔 저장
        }
    }

    dest[k] = '\0';                          // 문자열 끝 표시
}

void ApplyF5(const TCHAR src[], TCHAR dest[]) // 가장 많이 나온 문자를 @로 변경
{
    int count[128] = { 0 };                  // ASCII 문자 빈도수 배열
    int i;                                   // 반복 변수
    int maxCount = 0;                        // 가장 많이 나온 횟수
    TCHAR target = 0;                        // 가장 많이 나온 문자

    CopyString(dest, src);                   // 우선 원본 문자열 복사

    for (i = 0; src[i] != '\0'; i++) {       // 원본 문자열 전체 검사
        if (src[i] != ' ' && src[i] >= 0 && src[i] < 128) { // 공백 제외, ASCII만 사용
            count[(int)src[i]]++;            // 해당 문자 개수 증가

            if (count[(int)src[i]] > maxCount) { // 지금까지 최대보다 많으면
                maxCount = count[(int)src[i]];   // 최대 개수 갱신
                target = src[i];                 // 대상 문자 갱신
            }
        }
    }

    if (maxCount == 0) {                     // 바꿀 문자가 없으면
        return;                              // 종료
    }

    for (i = 0; dest[i] != '\0'; i++) {      // 결과 문자열 전체 검사
        if (dest[i] == target) {             // 대상 문자면
            dest[i] = '@';                   // @로 바꾸기
        }
    }
}

void BuildDisplayLine(const TCHAR src[], TCHAR dest[]) // 화면 출력용 문자열 생성
{
    TCHAR temp1[DISP_BUF];                   // 중간 결과 버퍼 1
    TCHAR temp2[DISP_BUF];                   // 중간 결과 버퍼 2

    CopyString(temp1, src);                  // 원본을 temp1에 복사

    if (g_f2Mode) {                          // F2가 켜져 있으면
        ApplyF2(temp1, temp2);              // temp1 -> temp2 변환
        CopyString(temp1, temp2);           // 다시 temp1에 반영
    }

    if (g_f3Mode) {                          // F3가 켜져 있으면
        ApplyF3(temp1, temp2);              // temp1 -> temp2 변환
        CopyString(temp1, temp2);           // temp1에 반영
    }

    if (g_f4Mode) {                          // F4가 켜져 있으면
        ApplyF4(temp1, temp2);              // temp1 -> temp2 변환
        CopyString(temp1, temp2);           // temp1에 반영
    }

    if (g_f5Mode) {                          // F5가 켜져 있으면
        ApplyF5(temp1, temp2);              // temp1 -> temp2 변환
        CopyString(temp1, temp2);           // temp1에 반영
    }

    CopyString(dest, temp1);                // 최종 결과를 dest에 저장
}

void BuildDisplayPrefix(const TCHAR src[], int prefixLen, TCHAR dest[]) // 캐럿 앞부분만 화면용으로 생성
{
    TCHAR prefix[MAX_COLS + 1];             // 앞부분만 담을 버퍼
    int i;                                  // 반복 변수

    for (i = 0; i < prefixLen && src[i] != '\0'; i++) {
        prefix[i] = src[i];                 // prefixLen 길이만큼 복사
    }
    prefix[i] = '\0';                       // 문자열 끝 표시

    BuildDisplayLine(prefix, dest);         // 화면용 변환 적용
}

/* ---------------- F6, F7, F8 실제 데이터 변경 ---------------- */

void RotateLinesForward()                   // F6: 줄 순서를 한 칸 앞으로 이동
{
    int i;                                  // 반복 변수
    TCHAR tempLine[MAX_COLS + 1];           // 첫 줄 임시 저장
    int tempLen;                            // 첫 줄 길이 임시 저장

    if (g_usedLines <= 1) {                 // 한 줄 이하이면
        return;                             // 회전할 필요 없음
    }

    CopyString(tempLine, g_lines[0]);       // 첫 줄 저장
    tempLen = g_len[0];                     // 첫 줄 길이 저장

    for (i = 0; i < g_usedLines - 1; i++) { // 앞에서부터 한 줄씩 당기기
        CopyString(g_lines[i], g_lines[i + 1]); // 다음 줄을 현재 줄로 복사
        g_len[i] = g_len[i + 1];                // 길이도 복사
    }

    CopyString(g_lines[g_usedLines - 1], tempLine); // 마지막 줄에 원래 첫 줄 저장
    g_len[g_usedLines - 1] = tempLen;              // 길이도 복원

    g_curLine--;                             // 현재 줄 번호도 같이 조정
    if (g_curLine < 0) {                     // 음수가 되면
        g_curLine = g_usedLines - 1;         // 마지막 사용 줄로 이동
    }
}

void IncreaseAllDigits()                     // F7: 모든 숫자 +1
{
    int i, j;                                // 반복 변수

    for (i = 0; i < g_usedLines; i++) {      // 모든 사용 줄 반복
        for (j = 0; j < g_len[i]; j++) {     // 줄의 모든 문자 반복
            if (IsDigitChar(g_lines[i][j])) { // 숫자면
                if (g_lines[i][j] == '9') {   // 9면
                    g_lines[i][j] = '0';      // 0으로 순환
                }
                else {
                    g_lines[i][j] = g_lines[i][j] + 1; // 다음 숫자로 증가
                }
            }
        }
    }
}

void DecreaseAllDigits()                     // F8: 모든 숫자 -1
{
    int i, j;                                // 반복 변수

    for (i = 0; i < g_usedLines; i++) {      // 모든 사용 줄 반복
        for (j = 0; j < g_len[i]; j++) {     // 줄의 모든 문자 반복
            if (IsDigitChar(g_lines[i][j])) { // 숫자면
                if (g_lines[i][j] == '0') {   // 0이면
                    g_lines[i][j] = '9';      // 9로 순환
                }
                else {
                    g_lines[i][j] = g_lines[i][j] - 1; // 이전 숫자로 감소
                }
            }
        }
    }
}

/* ---------------- WinMain ---------------- */

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
    HWND hWnd;                                // 창 핸들
    MSG Message;                              // 메시지 구조체
    WNDCLASSEX WndClass;                      // 윈도우 클래스 구조체

    g_hInst = hInstance;                      // 인스턴스 저장

    WndClass.cbSize = sizeof(WndClass);       // 구조체 크기 설정
    WndClass.style = CS_HREDRAW | CS_VREDRAW; // 창 크기 변경 시 다시 그림
    WndClass.lpfnWndProc = WndProc;           // 메시지 처리 함수 등록
    WndClass.cbClsExtra = 0;                  // 추가 클래스 메모리 없음
    WndClass.cbWndExtra = 0;                  // 추가 윈도우 메모리 없음
    WndClass.hInstance = hInstance;           // 인스턴스 핸들 저장
    WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION); // 기본 아이콘
    WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);   // 기본 커서
    WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); // 흰 배경
    WndClass.lpszMenuName = NULL;             // 메뉴 없음
    WndClass.lpszClassName = lpszClass;       // 클래스 이름
    WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION); // 작은 아이콘

    RegisterClassEx(&WndClass);               // 윈도우 클래스 등록

    hWnd = CreateWindow(
        lpszClass,                            // 클래스 이름
        lpszWindowName,                       // 창 제목
        WS_OVERLAPPEDWINDOW,                  // 기본 창 스타일
        0, 0, 800, 600,                       // 위치와 크기
        NULL, NULL, hInstance, NULL          // 부모, 메뉴, 인스턴스, 추가 정보
    );

    ShowWindow(hWnd, nCmdShow);               // 창 표시
    UpdateWindow(hWnd);                       // 즉시 한 번 그리기

    while (GetMessage(&Message, NULL, 0, 0)) { // 메시지 루프
        TranslateMessage(&Message);           // 키보드 메시지 변환
        DispatchMessage(&Message);            // WndProc으로 전달
    }

    return (int)Message.wParam;               // 종료 코드 반환
}

/* ---------------- WndProc ---------------- */

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;                           // WM_PAINT용 구조체
    HDC hDC;                                  // 디바이스 컨텍스트
    SIZE size;                                // 문자열 픽셀 길이 저장
    TEXTMETRIC tm;                            // 글꼴 정보 저장

    int i;                                    // 반복 변수
    int targetLine;                           // 이동할 대상 줄

    TCHAR drawLine[DISP_BUF];                 // 실제 출력할 문자열
    TCHAR drawPrefix[DISP_BUF];               // 캐럿 앞부분 문자열

    switch (uMsg) {

    case WM_CREATE:                           // 창이 처음 생성될 때
        CreateCaret(hWnd, NULL, 5, 15);       // 캐럿 생성
        ShowCaret(hWnd);                      // 캐럿 표시
        ResetAllText();                       // 전체 초기화
        break;

    case WM_KEYDOWN:                          // 기능키, 방향키 처리
        if (wParam == VK_F1) {                // F1 키
            g_upperInputMode = !g_upperInputMode; // 대문자/소문자 입력 모드 토글
            InvalidateRect(hWnd, NULL, TRUE); // 다시 그리기
            break;
        }

        if (wParam == VK_F2) {                // F2 키
            g_f2Mode = !g_f2Mode;             // F2 화면 효과 토글
            InvalidateRect(hWnd, NULL, TRUE);
            break;
        }

        if (wParam == VK_F3) {                // F3 키
            g_f3Mode = !g_f3Mode;             // F3 화면 효과 토글
            InvalidateRect(hWnd, NULL, TRUE);
            break;
        }

        if (wParam == VK_F4) {                // F4 키
            g_f4Mode = !g_f4Mode;             // F4 화면 효과 토글
            InvalidateRect(hWnd, NULL, TRUE);
            break;
        }

        if (wParam == VK_F5) {                // F5 키
            g_f5Mode = !g_f5Mode;             // F5 화면 효과 토글
            InvalidateRect(hWnd, NULL, TRUE);
            break;
        }

        if (wParam == VK_F6) {                // F6 키
            RotateLinesForward();             // 줄 순서 회전
            g_desiredCol = g_curCol;          // 원하는 열 갱신
            InvalidateRect(hWnd, NULL, TRUE);
            break;
        }

        if (wParam == VK_F7) {                // F7 키
            IncreaseAllDigits();              // 모든 숫자 +1
            InvalidateRect(hWnd, NULL, TRUE);
            break;
        }

        if (wParam == VK_F8) {                // F8 키
            DecreaseAllDigits();              // 모든 숫자 -1
            InvalidateRect(hWnd, NULL, TRUE);
            break;
        }

        if (wParam == VK_LEFT) {              // 왼쪽 방향키
            if (g_curCol > 0) {               // 왼쪽으로 이동 가능하면
                g_curCol--;                   // 한 칸 왼쪽 이동
            }
            g_desiredCol = g_curCol;          // 원하는 열 갱신
            InvalidateRect(hWnd, NULL, TRUE);
            break;
        }

        if (wParam == VK_RIGHT) {             // 오른쪽 방향키
            if (g_curCol < g_len[g_curLine]) { // 줄 끝 전까지만 이동
                g_curCol++;                   // 한 칸 오른쪽 이동
            }
            g_desiredCol = g_curCol;          // 원하는 열 갱신
            InvalidateRect(hWnd, NULL, TRUE);
            break;
        }

        if (wParam == VK_UP) {                // 위 방향키
            targetLine = FindPrevValidLine(g_curLine); // 이전에 글자가 있는 줄 찾기

            if (targetLine != g_curLine) {    // 이동할 줄이 있으면
                g_curLine = targetLine;       // 해당 줄로 이동

                if (g_desiredCol > g_len[g_curLine]) { // 원하는 열이 줄 길이보다 크면
                    g_curCol = g_len[g_curLine];       // 줄 끝으로 보정
                }
                else {
                    g_curCol = g_desiredCol;  // 아니면 원하는 열 유지
                }
            }

            InvalidateRect(hWnd, NULL, TRUE);
            break;
        }

        if (wParam == VK_DOWN) {              // 아래 방향키
            targetLine = (g_curLine + 1) % MAX_LINES; // 다음 줄 계산

            if (g_len[targetLine] > 0) {      // 다음 줄에 글자가 있으면
                g_curLine = targetLine;       // 그 줄로 이동

                if (g_desiredCol > g_len[g_curLine]) { // 원하는 열이 더 크면
                    g_curCol = g_len[g_curLine];       // 줄 끝으로 보정
                }
                else {
                    g_curCol = g_desiredCol;  // 아니면 원하는 열 유지
                }
            }
            else {                             // 다음 줄이 비어 있으면
                g_curLine = targetLine;        // 그 줄로 이동
                g_curCol = 0;                  // 맨 앞으로 이동

                if (g_usedLines < MAX_LINES) { // 새 줄 사용 처리
                    if (g_curLine + 1 > g_usedLines) {
                        g_usedLines = g_curLine + 1;
                    }
                }
            }

            g_desiredCol = g_curCol;           // 원하는 열 갱신
            InvalidateRect(hWnd, NULL, TRUE);
            break;
        }

        if (wParam == VK_HOME) {               // Home 키
            g_curCol = 0;                      // 줄 맨 앞으로 이동
            g_desiredCol = g_curCol;           // 원하는 열 갱신
            InvalidateRect(hWnd, NULL, TRUE);
            break;
        }

        if (wParam == VK_END) {                // End 키
            if (g_len[g_curLine] >= MAX_COLS) { // 줄이 꽉 차 있으면
                MoveToNextLineStart();         // 다음 줄 맨 앞으로 이동
            }
            else {
                g_curCol = g_len[g_curLine];   // 현재 줄 끝으로 이동
            }

            g_desiredCol = g_curCol;           // 원하는 열 갱신
            InvalidateRect(hWnd, NULL, TRUE);
            break;
        }

        if (wParam == VK_INSERT) {             // Insert 키
            if (g_insertMode == 0) {           // 현재 덮어쓰기면
                g_insertMode = 1;              // 삽입 모드로 변경
            }
            else {
                g_insertMode = 0;              // 덮어쓰기 모드로 변경
            }

            InvalidateRect(hWnd, NULL, TRUE);
            break;
        }

        if (wParam == VK_DELETE) {             // Delete 키
            DeleteWordAtCaret();               // 현재 단어 삭제

            if (g_curCol > g_len[g_curLine]) { // 삭제 후 캐럿 보정
                g_curCol = g_len[g_curLine];
            }

            g_desiredCol = g_curCol;           // 원하는 열 갱신
            InvalidateRect(hWnd, NULL, TRUE);
            break;
        }

        if (wParam == VK_PRIOR) {              // Page Up 키
            g_curLine = g_curLine - 3;         // 3줄 위로 이동

            while (g_curLine < 0) {            // 음수면
                g_curLine += MAX_LINES;        // 순환 보정
            }

            if (g_desiredCol > g_len[g_curLine]) {
                g_curCol = g_len[g_curLine];   // 줄 끝으로 보정
            }
            else {
                g_curCol = g_desiredCol;       // 아니면 원하는 열 유지
            }

            InvalidateRect(hWnd, NULL, TRUE);
            break;
        }

        if (wParam == VK_NEXT) {               // Page Down 키
            g_curLine = (g_curLine + 3) % MAX_LINES; // 3줄 아래로 이동

            if (g_desiredCol > g_len[g_curLine]) {
                g_curCol = g_len[g_curLine];   // 줄 끝으로 보정
            }
            else {
                g_curCol = g_desiredCol;       // 아니면 원하는 열 유지
            }

            InvalidateRect(hWnd, NULL, TRUE);
            break;
        }

        break;

    case WM_CHAR:                              // 일반 문자 입력 처리
        if (wParam == VK_ESCAPE) {             // ESC 키
            ResetAllText();                    // 전체 초기화
            InvalidateRect(hWnd, NULL, TRUE);  // 다시 그리기
            break;
        }

        if (wParam == VK_BACK) {               // Backspace 키
            BackspaceAtCurrent();              // Backspace 처리
            g_desiredCol = g_curCol;           // 원하는 열 갱신
            InvalidateRect(hWnd, NULL, TRUE);
            break;
        }

        if (wParam == VK_RETURN) {             // Enter 키
            MoveToNextLineStart();             // 다음 줄 맨 앞으로 이동
            g_desiredCol = g_curCol;           // 원하는 열 갱신
            InvalidateRect(hWnd, NULL, TRUE);
            break;
        }

        if (wParam == VK_TAB) {                // Tab 키
            InsertTabSpaces();                 // 공백 4칸 입력
            g_desiredCol = g_curCol;           // 원하는 열 갱신
            InvalidateRect(hWnd, NULL, TRUE);
            break;
        }

        if (wParam >= 32 && wParam <= 126) {   // 출력 가능한 일반 문자이면
            InputNormalChar((TCHAR)wParam);    // 문자 입력 처리
            g_desiredCol = g_curCol;           // 원하는 열 갱신
            InvalidateRect(hWnd, NULL, TRUE);
        }
        break;

    case WM_PAINT:                             // 화면 다시 그리기
        hDC = BeginPaint(hWnd, &ps);           // 그리기 시작

        GetTextMetrics(hDC, &tm);              // 글꼴 정보 얻기
        g_lineHeight = tm.tmHeight + 4;        // 줄 높이 계산

        for (i = 0; i < g_usedLines; i++) {    // 사용 중인 줄만 출력
            BuildDisplayLine(g_lines[i], drawLine); // 화면용 문자열 생성
            TextOut(hDC, 0, i * g_lineHeight, drawLine, StringLen(drawLine)); // 출력
        }

        BuildDisplayPrefix(g_lines[g_curLine], g_curCol, drawPrefix); // 캐럿 앞부분 문자열 생성
        GetTextExtentPoint32(hDC, drawPrefix, StringLen(drawPrefix), &size); // 픽셀 길이 계산
        SetCaretPos(size.cx, g_curLine * g_lineHeight); // 캐럿 위치 설정

        EndPaint(hWnd, &ps);                   // 그리기 종료
        break;

    case WM_DESTROY:                           // 창이 닫힐 때
        HideCaret(hWnd);                       // 캐럿 숨기기
        DestroyCaret();                        // 캐럿 제거
        PostQuitMessage(0);                    // 종료 메시지 전달
        return 0;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam); // 나머지 메시지는 기본 처리
}