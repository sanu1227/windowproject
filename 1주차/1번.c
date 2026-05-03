#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <conio.h>
#include <string.h>

int main() {
    char sentence[41];
    char overflowSentence[200];
    char target;
    int i, j;
    int end = 0;
    int overflowEnd = 0;
    char ch;

    char words[20][41];
    int spaces[20] = { 0 };
    int wordCount = 0;

    // 1. 문장 입력
    printf("문장을 입력하세요 (최대 40자, Enter로 종료): ");
    while (end < 40) {
        ch = _getch();
        if (ch == 13) break;
        sentence[end++] = ch;
        printf("%c", ch);
    }

    if (end == 0) sentence[0] = '.', end = 1;
    else if (sentence[end - 1] != '.') {
        if (end < 40) sentence[end++] = '.';
        else sentence[39] = '.';
    }
    sentence[end] = '\0';
    overflowEnd = 0;

    // 단어 + 공백 초기화
    i = 0; wordCount = 0;
    while (i < end) {
        while (sentence[i] == ' ') i++;
        if (i >= end - 1) break;

        j = 0;
        while (sentence[i] != ' ' && sentence[i] != '.' && i < end)
            words[wordCount][j++] = sentence[i++];
        words[wordCount][j] = '\0';
        wordCount++;

        int spaceCount = 0;
        while (sentence[i] == ' ' && i < end) { spaceCount++; i++; }
        if (wordCount < 20) spaces[wordCount - 1] = spaceCount;
    }

    static int showingSorted = 0;
    static int showingWordSorted = 0;

    while (1) {
        printf("\nCommand: ");
        target = _getch();
        printf("%c\n", target);
        if (target == '0') break;

        // 대소문자 변환
        for (i = 0; i < wordCount; i++)
            for (j = 0; words[i][j] != '\0'; j++)
                if (words[i][j] == target ||
                    words[i][j] == target - ('a' - 'A') ||
                    words[i][j] == target + ('a' - 'A')) {

                    if (words[i][j] >= 'a' && words[i][j] <= 'z') words[i][j] -= ('a' - 'A');
                    else if (words[i][j] >= 'A' && words[i][j] <= 'Z') words[i][j] += ('a' - 'A');
                }

        // 공백 줄이기
        if (target == '1') {
            for (i = 0; i < wordCount - 1; i++)
                if (spaces[i] > 0) spaces[i]--;
        }

        // 공백 늘리기
        if (target == '2') {
            for (i = 0; i < wordCount - 1; i++) {
                if (spaces[i] < 5) spaces[i]++;
            }
        }

        // 알파벳 오름차순 출력
        if (target == '3') {
            if (!showingSorted) {
                int count[128] = { 0 };
                char sorted[41];
                int s = 0;
                for (i = 0; i < wordCount; i++)
                    for (j = 0; words[i][j] != '\0'; j++)
                        if ((words[i][j] >= 'A' && words[i][j] <= 'Z') || (words[i][j] >= 'a' && words[i][j] <= 'z'))
                            count[(int)words[i][j]]++;

                for (i = 'A'; i <= 'Z'; i++) if (count[i] > 0) sorted[s++] = (char)i;
                for (i = 'a'; i <= 'z'; i++) if (count[i] > 0) sorted[s++] = (char)i;
                sorted[s] = '\0';

                printf("알파벳 오름차순 및 개수:\n");
                for (i = 0; sorted[i] != '\0'; i++)
                    printf("%c : %d\n", sorted[i], count[(int)sorted[i]]);
                showingSorted = 1;
            }
            else {
                // 기존 문장 재구성 + overflow 포함
                end = 0; overflowEnd = 0;
                for (i = 0; i < wordCount; i++) {
                    for (j = 0; words[i][j] != '\0'; j++) {
                        if (end < 40) sentence[end++] = words[i][j];
                        else overflowSentence[overflowEnd++] = words[i][j];
                    }
                    if (i < wordCount - 1)
                        for (j = 0; j < spaces[i]; j++) {
                            if (end < 40) sentence[end++] = ' ';
                            else overflowSentence[overflowEnd++] = ' ';
                        }
                }
                if (end < 40) sentence[end++] = '.';
                else overflowSentence[overflowEnd++] = '.';
                sentence[end] = '\0';
                overflowSentence[overflowEnd] = '\0';
                printf("원래 문장: %s", sentence);
                if (overflowEnd > 0) printf(" ...[%d글자 overflow]", overflowEnd);
                printf("\n");
                showingSorted = 0;
            }
            continue;
        }

        // 단어 알파벳 수 오름차순 출력
        if (target == '4') {
            if (!showingWordSorted) {
                char tempWords[20][41];
                for (i = 0; i < wordCount; i++) strcpy(tempWords[i], words[i]);
                for (i = 0; i < wordCount - 1; i++)
                    for (j = i + 1; j < wordCount; j++)
                        if (strlen(tempWords[i]) > strlen(tempWords[j])) {
                            char t[41]; strcpy(t, tempWords[i]);
                            strcpy(tempWords[i], tempWords[j]);
                            strcpy(tempWords[j], t);
                        }
                printf("단어 알파벳 수 오름차순:\n");
                for (i = 0; i < wordCount; i++) {
                    printf("%s", tempWords[i]);
                    if (i < wordCount - 1) printf(" ");
                }
                printf("\n");
                showingWordSorted = 1;
            }
            else {
                // 기존 문장 재구성 + overflow 포함
                end = 0; overflowEnd = 0;
                for (i = 0; i < wordCount; i++) {
                    for (j = 0; words[i][j] != '\0'; j++) {
                        if (end < 40) sentence[end++] = words[i][j];
                        else overflowSentence[overflowEnd++] = words[i][j];
                    }
                    if (i < wordCount - 1)
                        for (j = 0; j < spaces[i]; j++) {
                            if (end < 40) sentence[end++] = ' ';
                            else overflowSentence[overflowEnd++] = ' ';
                        }
                }
                if (end < 40) sentence[end++] = '.';
                else overflowSentence[overflowEnd++] = '.';
                sentence[end] = '\0';
                overflowSentence[overflowEnd] = '\0';
                printf("원래 문장: %s", sentence);
                if (overflowEnd > 0) printf(" ...[%d글자 overflow]", overflowEnd);
                printf("\n");
                showingWordSorted = 0;
            }
            continue;
        }

        // 문장 재구성
        end = 0; overflowEnd = 0;
        for (i = 0; i < wordCount; i++) {
            for (j = 0; words[i][j] != '\0'; j++) {
                if (end < 40) sentence[end++] = words[i][j];
                else overflowSentence[overflowEnd++] = words[i][j];
            }
            if (i < wordCount - 1)
                for (j = 0; j < spaces[i]; j++) {
                    if (end < 40) sentence[end++] = ' ';
                    else overflowSentence[overflowEnd++] = ' ';
                }
        }
        if (end < 40) sentence[end++] = '.';
        else overflowSentence[overflowEnd++] = '.';
        sentence[end] = '\0';
        overflowSentence[overflowEnd] = '\0';
        printf("결과 문장: %s", sentence);
        if (overflowEnd > 0) printf(" ...[%d글자 overflow]", overflowEnd);
        printf("\n");
    }

    printf("프로그램 종료\n");
    return 0;
}