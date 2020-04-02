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

//---Files
#include "utils.h"

// ./simpledu -l [path] [-a] [-b] [-B size] [-L] [-S] [--max-depth=N]

// • -a, --all – a informação exibida diz respeito também a ficheiros;
// • -b, --bytes – apresenta o número real de bytes de dados (ficheiros) ou alocados (diretórios);
// • -B, --block-size=SIZE – define o tamanho (bytes) do bloco para efeitos de representação;
// • -l, --count-links – contabiliza múltiplas vezes o mesmo ficheiro;
// • -L, --dereference – segue links simbólicos;
// • -S, --separate-dirs – a informação exibida não inclui o tamanho dos subdiretórios;
// • --max-depth=N – limita a informação exibida a N (0,1, …) níveis de profundidade de diretórios

int file;

int init(int all, int b, int B, int Bsize, int path, 
            int L, int S, int mDepth, int maxDepth, char* pathAd){
    /**USED FOR TESTING REASONS ONLY**/
    //if (all) printf("Got all\n");
    //if (b) printf("Got b\n");
    //if (L) printf("Got -L\n");
    //if (S) printf("Got -S\n");
    //if (mDepth) printf("Got %d depth\n", maxDepth);
    //if (B) printf("Got -B %d\n", Bsize);

    DIR *dirp;
    struct dirent *direntp;
    struct stat stat_buf;
    char directoryname[150] = { '\0' };
    
    int status;
    pid_t pid = 0;
    char fp[PATH_MAX];

    int dirSize = 0;

    if ((dirp = opendir(pathAd)) == NULL)
    {
        perror(pathAd);
        exit(2);
    }

    while ((direntp = readdir(dirp)) != NULL){
        int pp[2];
        if (pipe(pp) == -1) printf("Pipe error %s\n", strerror(errno));

        sprintf(fp, "%s/%s", pathAd, direntp->d_name);

        if (lstat(fp, &stat_buf)==-1){
            perror("lstat ERROR");
            exit(3);
        }

        if (S_ISREG(stat_buf.st_mode)) {
            long num = stat_buf.st_size;
            char sendFile[50];

            if (all && (!mDepth || maxDepth > 0)) {
                if(!(b && !B)){
                    round_up_4096(&num);
                    num = num / Bsize;
                }
                sprintf(sendFile, "%-8ld %s \n", num, fp);
                write(STDOUT_FILENO, sendFile, strlen(sendFile));
            }
            dirSize += num;
        }
        
        else if (S_ISDIR(stat_buf.st_mode) && (!mDepth || (maxDepth > 0))) {

            strcpy(directoryname, direntp->d_name);
            if(check_point_folders(directoryname)){
                pid = fork();

                if (pid == 0 ){
                    char* cmd[10];
                    if (mDepth) maxDepth--;
                    cmd_builder(all, b, B, Bsize, path, L, S, mDepth, maxDepth, fp, cmd);

                    close(pp[READ]);
                    if (dup2(pp[WRITE], STDOUT_FILENO) == -1) printf("Dup error %s\n", strerror(errno));
                    execvp("./simpledu", cmd);
                }

                else if (pid > 0){
                    wait(&status);

                    close(pp[WRITE]);
                    char content[PATH_MAX];
                    while (read(pp[READ], content, PATH_MAX)){
                        write(STDOUT_FILENO, content, strlen(content));
                        write(file, content, strlen(content));
                        if (!S){
                            char* lines[MAX_INPUT]; line_divider(content, lines);
                            add_initial_numbers(lines, &dirSize, pathAd, fp, file);
                        }
                        memset(content, 0, sizeof(content));
                    }
                    write(file, "\n", 1);
                }
            }
            memset(directoryname, 0, sizeof(directoryname));
        }
    }

    if (b && !B){
        //dirSize += stat_buf.st_size;
        dirSize += 4096;
    }
    else{
        round_up_4096(&stat_buf.st_size);
        stat_buf.st_size = stat_buf.st_size / Bsize;
        dirSize += stat_buf.st_size;
    }
    char sendDir[50];
    sprintf(sendDir,"%-8d %s \n", dirSize, pathAd);
    write(STDOUT_FILENO, sendDir, strlen(sendDir));

    closedir(dirp);

    return 0;
}

int main(int argc, char *argv[]){

    //TO KNOW WHAT IS RECEIVED THROUGHT EXEC
    // for (int i = 0; i < argc; i++){
    //     printf("%s ", argv[i]);
    // }
    // printf("\n");

    //'boolean' variable initialization
    int all = 0;
    int b = 0;
    int B = 0; int Bsize = 1024; // Bsize corresponds to block size indicated
    int path = 0; char pathAd[PATH_MAX];
    int L = 0;
    int S = 0;
    int mDepth = 0; int maxDepth = 0; //maxDepth corresponds to max depth value

    //worng usage
    if (argc > 10 || argc < 2 || strcmp(argv[1], "-l") != 0){ 
        write(STDOUT_FILENO, "USAGE: ./simpledu -l [path] [-a] [-b] [-B size] [-L] [-S] [--max-depth=N]\n",
        strlen("USAGE: ./simpledu -l [path] [-a] [-b] [-B size] [-L] [-S] [--max-depth=N]\n"));
        exit(1);
    }

    //interpret argv content
    for (int i = 2; i < argc; i++){

        //set fot [-a] or [-all]
        if (strcmp(argv[i],"-a") == 0 || strcmp(argv[i],"-all") == 0) all = 1;

        //set for [-b]
        else if (strcmp(argv[i],"-b") == 0) b = 1;

        //set for [-B size]
        else if (strcmp(argv[i], "-B") == 0){
            if(is_number(argv[i+1]) == 1){
                B = 1;
                Bsize = atoi(argv[i+1]); i++;
            }
        }

        //set for [-L]
        else if (strcmp(argv[i],"-L") == 0) L = 1;

        //set for [-S]
        else if (strcmp(argv[i],"-S") == 0) S = 1;

        //set for [--max-depth=N]
        else if (argv[i][0] == '-' && argv[i][1] == '-'){
            char maxD[50];
            slice_str(argv[i], maxD, 0, 11);
            if (strcmp(maxD, "--max-depth=") == 0){
                slice_str(argv[i], maxD, 12, 13);
                maxDepth = atoi(maxD);
                mDepth = 1;
            }
        }

        else if (atoi(argv[i])) break;

        //set for path
        else{
            path = 1;
            strcpy(pathAd, argv[i]);
        }
    }

    //Set default path
    if (!path) strcpy(pathAd,".");

    file = open("reg.txt", O_WRONLY | O_APPEND);
    init(all, b, B, Bsize, path, L, S, mDepth, maxDepth, pathAd);
    close(file);

    return 0;
}
