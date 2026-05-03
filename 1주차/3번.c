#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// 영화 제목 최대 10자 + '\0'
#define MAX_TITLE 11

// 영화관 좌석 10x10
#define MAX_SEATS 10

// 한 영화관의 상영시간 개수
#define MAX_SHOW 3

// 전체 영화관 수
#define MAX_THEATER 3

// ---------------------------- 구조체 ------------------------------

// 좌석 구조체
typedef struct {
    int reservedNum;     // 0이면 빈 좌석, 1~99면 예약번호
} Seat;

// 상영 시간 구조체
typedef struct {
    int hour;                        // 상영 시간
    Seat seats[MAX_SEATS][MAX_SEATS];
} ShowTime;

// 영화관 구조체
typedef struct {
    int theaterNum;               // 영화관 번호
    char title[MAX_TITLE];        // 영화 제목
    ShowTime shows[MAX_SHOW];     // 상영 시간 3개
} Theater;

// ---------------------------- 전역 변수 ----------------------------

// 3개 영화관 초기화
Theater theaters[MAX_THEATER] = {
    {1, "Matrix", {{1100}, {1400}, {1700}}},
    {2, "Avatar", {{1000}, {1300}, {1600}}},
    {3, "Inception", {{1200}, {1500}, {1800}}}
};

// 현재 예약번호
int currentReservation = 1;

// ---------------------------- 함수 원형 ----------------------------

void initSeats();
void printTheaterInfo();
void printSeatsByTheater(int tIndex);
int findTheaterByTitle(char* title);
void reserveSeat();
void cancelReservation();
void printReservationRate();

// ---------------------------- 함수 정의 ----------------------------

// 모든 좌석 초기화
void initSeats() {
    int t, s, i, j;

    for (t = 0; t < MAX_THEATER; t++) {
        for (s = 0; s < MAX_SHOW; s++) {
            for (i = 0; i < MAX_SEATS; i++) {
                for (j = 0; j < MAX_SEATS; j++) {
                    theaters[t].shows[s].seats[i][j].reservedNum = 0;
                }
            }
        }
    }
}

// 영화관 정보 출력
void printTheaterInfo() {
    int t, s;

    for (t = 0; t < MAX_THEATER; t++) {
        printf("영화관 %d: %s\n", theaters[t].theaterNum, theaters[t].title);
        printf("상영 시간: ");
        for (s = 0; s < MAX_SHOW; s++) {
            printf("%d ", theaters[t].shows[s].hour);
        }
        printf("\n");
    }
}

// 특정 영화관의 좌석 상태 출력
void printSeatsByTheater(int tIndex) {
    int s, i, j;

    printf("영화관 %d: %s\n", theaters[tIndex].theaterNum, theaters[tIndex].title);

    for (s = 0; s < MAX_SHOW; s++) {
        printf("상영시간 %d 좌석 상태:\n", theaters[tIndex].shows[s].hour);

        for (i = 0; i < MAX_SEATS; i++) {
            for (j = 0; j < MAX_SEATS; j++) {
                int num = theaters[tIndex].shows[s].seats[i][j].reservedNum;

                if (num == 0)
                    printf(" - ");
                else
                    printf("%02d ", num);
            }
            printf("\n");
        }
        printf("\n");
    }
}

// 영화 제목으로 영화관 찾기
int findTheaterByTitle(char* title) {
    int t;

    for (t = 0; t < MAX_THEATER; t++) {
        if (strcmp(theaters[t].title, title) == 0) {
            return t;
        }
    }
    return -1;
}

// 좌석 예약
void reserveSeat() {
    char input[20];
    int tIndex = -1;
    int showTime;
    int row, col;
    int sIndex = -1;
    int s;

    // 영화관 번호 또는 제목 입력
    printf("영화관 번호 또는 영화제목: ");
    scanf("%s", input);

    // 숫자면 영화관 번호, 아니면 제목
    if (input[0] >= '1' && input[0] <= '3')
        tIndex = atoi(input) - 1;
    else
        tIndex = findTheaterByTitle(input);

    if (tIndex == -1) {
        printf("영화관 없음\n");
        return;
    }

    // 상영시간 입력
    printf("상영시간 선택: ");
    scanf("%d", &showTime);

    for (s = 0; s < MAX_SHOW; s++) {
        if (theaters[tIndex].shows[s].hour == showTime) {
            sIndex = s;
            break;
        }
    }

    if (sIndex == -1) {
        printf("상영시간 없음\n");
        return;
    }

    // 좌석 입력
    printf("좌석 선택 (행 1~10 열 1~10): ");
    scanf("%d %d", &row, &col);

    row--;
    col--;

    // 좌석 범위 확인
    if (row < 0 || row >= MAX_SEATS || col < 0 || col >= MAX_SEATS) {
        printf("잘못된 좌석\n");
        return;
    }

    // 이미 예약된 좌석인지 확인
    if (theaters[tIndex].shows[sIndex].seats[row][col].reservedNum != 0) {
        printf("이미 예약됨\n");
        return;
    }

    // 예약 저장
    theaters[tIndex].shows[sIndex].seats[row][col].reservedNum = currentReservation;

    // 예약 완료 메시지 + 예약 정보 출력
    printf("\n===== 예약 완료 =====\n");
    printf("예약번호 : %02d\n", currentReservation);
    printf("영화제목 : %s\n", theaters[tIndex].title);
    printf("영화관   : %d관\n", theaters[tIndex].theaterNum);
    printf("상영시간 : %d\n", theaters[tIndex].shows[sIndex].hour);
    printf("좌석     : %d행 %d열\n", row + 1, col + 1);
    printf("====================\n");

    // 예약번호 증가
    currentReservation++;
    if (currentReservation > 99) {
        currentReservation = 1;
    }
}

// 예약 취소
void cancelReservation() {
    int resNum;
    int t, s, i, j;

    printf("취소할 예약번호: ");
    scanf("%d", &resNum);

    for (t = 0; t < MAX_THEATER; t++) {
        for (s = 0; s < MAX_SHOW; s++) {
            for (i = 0; i < MAX_SEATS; i++) {
                for (j = 0; j < MAX_SEATS; j++) {
                    if (theaters[t].shows[s].seats[i][j].reservedNum == resNum) {
                        theaters[t].shows[s].seats[i][j].reservedNum = 0;
                        printf("예약 취소 완료: 영화관 %s, 상영시간 %d, 좌석 (%d,%d)\n",
                            theaters[t].title,
                            theaters[t].shows[s].hour,
                            i + 1, j + 1);
                        return;
                    }
                }
            }
        }
    }

    printf("예약번호 없음\n");
}

// 예약률 출력
void printReservationRate() {
    int t, s, i, j;

    for (t = 0; t < MAX_THEATER; t++) {
        printf("영화관 %d: %s\n", theaters[t].theaterNum, theaters[t].title);

        for (s = 0; s < MAX_SHOW; s++) {
            int reserved = 0;
            double rate;

            for (i = 0; i < MAX_SEATS; i++) {
                for (j = 0; j < MAX_SEATS; j++) {
                    if (theaters[t].shows[s].seats[i][j].reservedNum != 0) {
                        reserved++;
                    }
                }
            }

            rate = reserved * 100.0 / (MAX_SEATS * MAX_SEATS);
            printf("  상영시간 %d 예약률: %.2f%%\n", theaters[t].shows[s].hour, rate);
        }
    }
}

// 메인 함수
int main() {
    char cmd;

    initSeats();

    while (1) {
        printf("\n명령어 (d:영화관정보, p:좌석상태, r:예약, c:취소, h:예약률, q:종료): ");
        scanf(" %c", &cmd);

        if (cmd == 'q') {
            break;
        }
        else if (cmd == 'd') {
            printTheaterInfo();
        }
        else if (cmd == 'p') {
            char arg[20];
            int tIndex = -1;

            printf("영화관 번호 또는 영화제목: ");
            scanf("%s", arg);

            if (arg[0] >= '1' && arg[0] <= '3')
                tIndex = atoi(arg) - 1;
            else
                tIndex = findTheaterByTitle(arg);

            if (tIndex != -1)
                printSeatsByTheater(tIndex);
            else
                printf("영화관 없음\n");
        }
        else if (cmd == 'r') {
            reserveSeat();
        }
        else if (cmd == 'c') {
            cancelReservation();
        }
        else if (cmd == 'h') {
            printReservationRate();
        }
        else {
            printf("잘못된 명령어\n");
        }
    }

    printf("프로그램 종료\n");
    return 0;
}