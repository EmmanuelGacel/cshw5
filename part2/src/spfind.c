/*******************************************************************************
 * Name          :
 * Author        :
 * Version       :
 * Date          :
 * Last Modified :
 * Description   : 
*******************************************************************************/
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#define BUFSIZE 128
int main(int argc, char** argv){
    int pipefd[2];
    if((pipe(pipefd)) == -1){
        fprintf(stderr,"Error: can't create pipe");
        return EXIT_FAILURE;
    }
    pid_t child_1pid, child_2pid;
    (child_1pid = fork()) && (child_2pid = fork()); //Creates two child processes
    //printf("Child 1 process %ld .\n",(long)child_1pid);
    //printf("Child 2 process %ld .\n",(long)child_2pid);
    if ((child_1pid  || child_2pid ) < 0){
        fprintf(stderr, "Before fork error");
    }else if(child_1pid == 0){//Inside child 1
        close (pipefd[0]);
        dup2(pipefd[1], 1);
        dup2(pipefd[1], 2);
        close (pipefd[1]);
        if (execv("pfind", argv) == -1){//Run pfind with argv as the input
            printf("Error: execlp() failed. %s.\n", argv[2]);
            fprintf(stderr, "Error: execlp() failed. %s.\n", strerror(errno));
            return EXIT_FAILURE;
        }
        printf("Child 1 process %ld continued.\n",(long)child_1pid);
    } else if(child_2pid == 0){//Inside child 2
        char buf[BUFSIZE];
        dup2(pipefd[1], 1);
        dup2(pipefd[1], 2);
        close (pipefd[1]);
        read(pipefd[0], buf, sizeof(BUFSIZE));
        int fd = open("file", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);//
        if (write (fd, buf, sizeof(buf) - 1) != (sizeof(buf) -1)){
            perror("write error");
            return EXIT_FAILURE;
        }
        if (execlp("sort", "sort", fd, (char *) 0) == -1){
            printf("Error: execlp() failed. %s.\n", argv[2]);
            fprintf(stderr, "Error: execlp() failed. %s.\n", strerror(errno));
            return EXIT_FAILURE;
        } 
        close (fd);
        close (pipefd[0]);
        printf("Child 2 process %ld continued.\n", (long)child_2pid);
    }else{//Inside parent
         close (pipefd[0]);
        dup2(pipefd[1], 1);
        dup2(pipefd[1], 2);
        close (pipefd[1]);
        printf("Parent process continued.\n");
        /*
        
        if (execvp("sort", argv) == -1){
            printf("Error: execlp() failed. %s.\n", argv[2]);
            fprintf(stderr, "Error: execlp() failed. %s.\n", strerror(errno));
            return EXIT_FAILURE;
        }
        */

        int status;
        int status1;
        do{//Wait for child 1 to finish
            pid_t w = waitpid(child_1pid, &status, WUNTRACED  | WCONTINUED);
            if (w == -1){//waitpid() failed
                perror("waitpid()");
                exit(EXIT_FAILURE);
            }
            if (WIFEXITED(status)) {
                printf("Child process %ld exited, status=%d.1\n", (long)child_1pid,
                       WEXITSTATUS(status)); 
            } else if (WIFSIGNALED(status)) {
                printf("Child process %ld killed by signal %d.\n", (long)child_1pid,
                       WTERMSIG(status));
            } else if (WIFSTOPPED(status)) {
                printf("Child process %ld stopped by signal %d.\n", (long)child_1pid,
                       WSTOPSIG(status));
            } else if (WIFCONTINUED(status)) {
                printf("Child process %ld continued.\n", (long)child_1pid);
            }
        }while (!WIFEXITED(status) && !WIFSIGNALED(status));
        do{//Wait fo child 2 to finish
            pid_t w = waitpid(child_2pid, &status1, WUNTRACED| WCONTINUED);
            if (w == -1){//waitpid() failed
                perror("waitpid()");
                exit(EXIT_FAILURE);
            }
            if (WIFEXITED(status1)) {
                printf("Child process %ld exited, status=%d.\n", (long)child_2pid,
                       WEXITSTATUS(status1));
            } else if (WIFSIGNALED(status1)) {
                printf("Child process %ld killed by signal %d.\n", (long)child_2pid,
                       WTERMSIG(status1));
            } else if (WIFSTOPPED(status1)) {
                printf("Child process %ld stopped by signal %d.\n", (long)child_2pid,
                       WSTOPSIG(status1));
            } else if (WIFCONTINUED(status1)) {
                printf("Child process %ld continued.\n", (long)child_2pid);
            }
        }while (!WIFEXITED(status1) && !WIFSIGNALED(status1));
    }
    return EXIT_SUCCESS;
}