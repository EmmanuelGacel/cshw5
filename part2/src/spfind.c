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
#define BUFSIZE 1024
int main(int argc, char** argv){

	int pfind_to_sort[2]; // initializes both elements to 0
	int sort_to_parent[2]; // initializes both elements to 0
	pid_t child_1pid, child_2pid;
    pipe(pfind_to_sort); 
    pipe(sort_to_parent);

    if((child_1pid = fork()) <= 0){//Inside child 1
        if(child_1pid < 0){
            fprintf(stderr, "Fork error");
             return EXIT_FAILURE;
        }

        close(pfind_to_sort[0]);
        dup2(pfind_to_sort[1],STDOUT_FILENO);
        close(pfind_to_sort[1]);
        close(sort_to_parent[0]);
        close(sort_to_parent[1]);

        if (execv("pfind", argv) == -1){//Run pfind with argv as the input
            //printf("Error: execlp() failed. %s.\n", argv[2]);
            fprintf(stderr, "Error: pfind failed. %s.\n", strerror(errno));
            return EXIT_FAILURE;
        }
	}

    if((child_2pid =fork()) <= 0){//Inside child 2
        if(child_2pid < 0){
            fprintf(stderr, "Fork error");
            return EXIT_FAILURE;
        }
        //char buf[BUFSIZE];
        close(pfind_to_sort[1]);
        dup2(pfind_to_sort[0],STDIN_FILENO);
        close(pfind_to_sort[0]);
        dup2(sort_to_parent[1],STDOUT_FILENO);
        close(sort_to_parent[0]);
        close(sort_to_parent[1]);
       
        if (execlp("sort", "sort", NULL) == -1){
            printf("Error: sort failed. %s.\n", argv[2]);
            //fprintf(stderr, "Error: execlp() failed. %s.\n", strerror(errno));
            return EXIT_FAILURE;
        }
    }//Inside of parent
        close(pfind_to_sort[0]);
        close(pfind_to_sort[1]);
        dup2(sort_to_parent[0],STDIN_FILENO);
        close(sort_to_parent[0]);
        close(sort_to_parent[1]);
        
        char buf[BUFSIZE];
        int file_count = 0;
        ssize_t count = read(STDIN_FILENO, buf, sizeof(buf) - 1);
        if (count == -1){
            perror("read()");
            exit (EXIT_FAILURE);
        }
        buf[count] = '\0';
        for (int i = 0; buf[i] != '\0'; i++ ){//Counts the number of files found
            if (buf[i] == '\n') file_count ++;
        }
        printf("%s", buf);
        printf("Total matches: %d\n", file_count);

        int status;
        int status1;
        do{//Wait for child 1 to finish
            pid_t w = waitpid(child_1pid, &status, 0);//WUNTRACED  | WCONTINUED);
            if (w == -1){//waitpid() failed
                perror("waitpid()");
                exit(EXIT_FAILURE);
            }
            if (WIFEXITED(status)) {
                printf("Child process %ld exited, status=%d.\n", (long)child_1pid,
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
            pid_t w = waitpid(child_2pid, &status1, 0);//WUNTRACED| WCONTINUED);
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

	return EXIT_SUCCESS;
}
