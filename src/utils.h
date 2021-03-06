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

void slice_str(const char * str, char * buffer, size_t start, size_t end);

void cmd_builder(int all, int b, int B, int Bsize, int path, int L, int S, int mDepth, int maxDepth, char* pathAd, int file);

void round_up_4096(long * num);

void int_to_char(int no, char* ch);

_Bool is_number(char*a);

int check_point_folders(char* directoryname);

void sigint_handler(int signal);
