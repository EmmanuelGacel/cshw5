/*
 * Prints the files that match a given file
 * permission and prints them 
 *
 * Author: Katherine Wimmer krw2146
 * */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h> 
#include <dirent.h>
#include <errno.h>
#include <limits.h>

void display_usage(int val){
	if(val == 1){
		fprintf(stdout, "Usage: ./pfind -d <directory> -p <permissions string> [-h]\n");
	}
	else{
		fprintf(stderr, "Usage: ./pfind -d <directory> -p <permissions string> [-h]\n");
	}
}

void matches_perm(char *filename, char * perm_string){

	int perms[] = {S_IRUSR, S_IWUSR, S_IXUSR,
               S_IRGRP, S_IWGRP, S_IXGRP,
               S_IROTH, S_IWOTH, S_IXOTH};


}

int recurse_dir(char* directory, char * perm_string){

	DIR *direct;
        if ((direct = opendir(directory)) == NULL) { //directory string should hold full path name
                fprintf(stderr, "Error: Cannot open or find directory '%s'. %s.\n",
                directory, strerror(errno)); //checks if the directory exists
                return EXIT_FAILURE;
        }

	char file_path[PATH_MAX + 1]; //extra space for the ending /	

	if (strcmp(directory, "/")) { //when the path is not the root (/)
        	size_t copy_len = strnlen(directory, PATH_MAX); //finds which is shorter
        	memcpy(file_path, directory, copy_len); //copies the full length of directory name to file_path
        	file_path[copy_len] = '\0'; //adds null terminator to file_path
    	}

	size_t length = strlen(file_path) + 1;
    	file_path[length - 1] = '/';
    	file_path[length] = '\0';

    	struct dirent *file;
    	struct stat sb;
    	while ((file = readdir(direct)) != NULL) {
        // Skip . and ..
        if (strcmp(file->d_name, ".") == 0 ||
            strcmp(file->d_name, "..") == 0) {
            continue;
        }
        // Add the current entry's name to the end of full_filename, following
        // the trailing '/' and overwriting the '\0'.
        strncpy(file_path + length, file->d_name, PATH_MAX - length);
        if (lstat(file_path, &sb) < 0) {
            fprintf(stderr, "Error: Cannot stat file '%s'. %s.\n",
                    file_path, strerror(errno));
            continue;
        }
        // Differentiate directories from other file types.
	if (S_ISDIR(sb.st_mode)) {
	    	recurse_dir(file_path, perm_string); //RECURSIVE CALL
        } else {
		//matches_perm(file_path, perm_string);
		printf("%s [FILE]\n", file_path);
        }
    }

	return EXIT_SUCCESS;
}


int main(int argc, char **argv){

	int dflag = 0, pflag = 0, hflag = 0;
	

	int option;
	char *directory, *perm;

	opterr = 0; //suppress error messages
	while((option = getopt(argc, argv, "d:p:h")) != -1){
		switch(option){
			case 'd':
				dflag = 1;
				directory = optarg;
				break;
			case 'p':
				pflag = 1;
				perm = optarg;
				break;		
			case 'h':
				hflag = 1;
				break;
			case '?':
				fprintf(stderr,"Error: Unknown option '%c' received.\n", optopt);
				return EXIT_FAILURE;
			default:
				return EXIT_FAILURE;
		}			
	}

	printf("=======\n dlfag: %d, pflag: %d, hflag: %d \n", dflag, pflag, hflag);
	printf("directory: %s, permission: %s\n=======\n", directory, perm);
	
	if(hflag == 1){
		display_usage(1);
		return EXIT_SUCCESS;
	}
	if(dflag == 0 && pflag == 0){
		display_usage(0);
		return EXIT_FAILURE;
	} else if(dflag == 0){
		fprintf(stderr, "Error: Required argument -d <directory> not found.\n");
		return EXIT_FAILURE;
	} else if(pflag == 0){
		fprintf(stderr, "Error: Required argument -p <permissions string> not found.\n");
	}
	
	
	//check permission string
	if(strlen(perm) != 9){
		fprintf(stderr, "Error: Invaid permission string '%s' entered.\n", perm);
                return EXIT_FAILURE;
	}
	for(int i = 0; i < 9; i = i + 3){
		if(perm[i] != 'r' && perm[i] != '-'){
			fprintf(stderr, "Error: Invaid permission string '%s' entered.\n", perm);
			return EXIT_FAILURE;
		}else if(perm[i + 1] != 'w' && perm[i + 1] != '-'){
			fprintf(stderr, "Error: Invaid permission string '%s' entered.\n", perm);
                        return EXIT_FAILURE;
		}else if(perm[i + 2] != 'x' && perm[i + 2] != '-'){
			fprintf(stderr, "Error: Invaid permission string '%s' entered.\n", perm);
                        return EXIT_FAILURE;
		}
	}

	//recurse directory
	
	recurse_dir(directory, perm); //goes to methods that perform the recursion
	
	return EXIT_SUCCESS;	
}
