#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <linux/limits.h>
#include <errno.h>
#include <fcntl.h>

#define READ 0
#define WRITE 1

void slice_str(const char * str, char * buffer, size_t start, size_t end){
    size_t j = 0;
    for ( size_t i = start; i <= end; ++i ) {
        buffer[j++] = str[i];
    }
    buffer[j] = 0;
}

void int_to_char(int no, char* ch){
    sprintf(ch, "%d", no);
}

void cmd_builder(int all, int b, int B, int Bsize, int path, int L, int S, int mDepth, int maxDepth, char* pathAd, char** cmd){
    cmd[0] = "./simpledu";
    cmd[1] = "-l";
    
    int i = 2;

    if (all) {
        cmd[i] = "-a";
        i++;
    }
    if (b) {
        cmd[i] = "-b";
        i++;
    }
    if (B) {
        char str[100] = "";
        cmd[i] = "-B";
        i++;
        int_to_char(Bsize, str);
        cmd[i] = str;
        i++;
    }
    if (path) {
        cmd[i] = pathAd;
        i++;
    }
    if (L) {
        cmd[i] = "-L";
        i++;
    }
    if (S) {
        cmd[i] = "-S";
        i++;
    }
    if (mDepth){
        printf("Does it get here??\n");
        char max[100];
        char no[100];
        int_to_char(maxDepth, no);
        sprintf(max, "%s%s", "--max-depth=",no);
        cmd[i] = max;
        i++;
    }
}

void round_up_4096(long * num){
    if(*num % 4096 != 0){
        *num = (*num / 4096) * 4096 + 4096;
    }
}

_Bool is_number(char*a){
    if(strlen(a) == 0){
        return 0;
    }
    for(int i = 0; i < strlen(a); i++){
        if(!(a[i] > 57 && a[i] < 48)){
            return 0;
        }
    }
    return 1;
}

int check_point_folders(char* directoryname){
    if(directoryname[0] != '.' || directoryname[1] != '\0'){
        if(directoryname[1] != '.' || directoryname[2] != '\0')
            return 1;
    }
    return 0;
}

void line_divider(char content[], char** lines){
    char* line;
    line = strtok(content, "\n");
    int i = 0;
    while (line != NULL){
        lines[i] = malloc(strlen(line));
        strcpy(lines[i], line);
        i++;
        line = strtok(NULL, "\n");
    }
}

void add_initial_numbers(char** lines, int* dirSize, char* to, char* from){
    int i = 0;
    char* num;

    while (lines[i] != NULL){
        num = strtok(lines[i], " ");
        i++;
    }
    //printf("Adding: %s to %s from %s \n", num, to, from);
    *dirSize += atoi(num);
}
