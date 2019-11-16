#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

char* getCondition(FILE* fp){
    char *tempCondition = (char*) malloc(100);
    char firstCharacter;
    fgets(tempCondition, 100, fp);
    printf("\nOriginal Stuff:\n%s\n\n", tempCondition);
    firstCharacter = tempCondition[0];

    int i = 0; // for the tempCondition
    int j = 0; // for the finalCondition
    char *finalCondition = (char*) malloc(100);
    while(tempCondition[i] != '\0') {
        finalCondition[j] = 'p';
        finalCondition[j+1] = 'o';
        finalCondition[j+2] = 'r';
        finalCondition[j+3] = 't';
        finalCondition[j+4] = 's';
        finalCondition[j+5] = '.';
        j+= 6;
        while (tempCondition[i] != ']') {
            finalCondition[j] = tempCondition[i];
            i++;
            j++;
        }
        finalCondition[j] = tempCondition[i];
        i++;
        j++;
        if (tempCondition[i] == '=') {
            finalCondition[j] = '=';
            j++;
            i += 2;
        }
        if (tempCondition[i] == '<') {
            finalCondition[j] = '<';
            i++;
            j++;
            if (tempCondition[i] == '=') {
                finalCondition[j] = '=';
                i++;
                j++;
            }
        }
        if (tempCondition[i] == '>') {
            finalCondition[i] = '>';
            i++;
            j++;
            if (tempCondition[i] == '=') {
                finalCondition[i] = '=';
                i++;
                j++;
            }
        }
        if (tempCondition[i] == '!') {
            finalCondition[i] = '!';
            i++;
            j++;
            finalCondition[i] = '=';
            i++;
            j++;
        }
        while ( //Keep writing until you either hit the end or a logical connector.
                tempCondition[i] != 'O' &&
                tempCondition[i] != 'A' &&
                tempCondition[i] != '-' &&
                tempCondition[i] != '<' &&
                tempCondition[i] != '\0'&&
                tempCondition[i] != firstCharacter) {
            finalCondition[j] = tempCondition[i];
            j++;
            i++;
        }
        if (tempCondition[i] == 'A') { //An AND begins
            finalCondition[j] = '&';
            j++;
            i+=3;
            continue;
        }
        if (tempCondition[i] == 'O') { //An OR begins
            finalCondition[j] = '|';
            j++;
            i+=2;
            continue;
        }
        if (tempCondition[i] == '-') {//An -> begins
            finalCondition[j] = '-';
            i++;
            j++;
            finalCondition[j] = '>';
            i++;
            j++;
            continue;
        }
        if (finalCondition[i] == '<') {//An <-> begins. Not sure if that really does happen, though...
            finalCondition[j] = '<';
            i++;
            j++;
            finalCondition[j] = '-';
            i++;
            j++;
            finalCondition[j] = '>';
            i++;
            j++;
            continue;
        }
        if(tempCondition[i] == '\0' || tempCondition[i] != firstCharacter) continue; //Either end or begin anew
    }
    return finalCondition;
}

int main() {
    FILE* fp = fopen("Teste.txt", "r");
    if(!fp)printf("Deu ruim em");
    char* test = getCondition(fp);
    printf("Final Stuff:\n%s\n", test);
    fclose(fp);
    return 0;
}