/*
 * Prints the files that match a given file
 * permission and prints them 
 *
 * Author: Katherine Wimmer and Emmanauel Gacel
 * Version: final
 * March 30, 2022
 *
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
	
	struct stat statbuf;
    	
	if (stat(filename, &statbuf) < 0) { //stats the filename
        	fprintf(stderr, "Error: Cannot stat '%s'. %s.\n", filename,
                strerror(errno));
        	exit(EXIT_FAILURE);
	}
	if (!S_ISREG(statbuf.st_mode) && !S_ISDIR(statbuf.st_mode)) { //checks to make sure its a file or dir
     	  	fprintf(stderr, "Error: '%s' is not a regular file.\n", filename);
        	exit(EXIT_FAILURE);
    	}
	
	//will hold the permissions for the file
	char *file_perm;
    	if ((file_perm = malloc(11 * sizeof(char))) == NULL) { //saves space on heap
        	fprintf(stderr, "Error: malloc failed. %s.\n",
                strerror(errno));
        	exit(EXIT_FAILURE);
    	}
    	for (int i = 0; i < 9; i += 3) { //creates the string
        	file_perm[i] = statbuf.st_mode & perms[i] ? 'r' : '-';
        	file_perm[i + 1] = statbuf.st_mode & perms[i + 1] ? 'w' : '-';
        	file_perm[i + 2] = statbuf.st_mode & perms[i + 2] ? 'x' : '-';
    	}
    	file_perm[9] = '\0'; //null terminates
    
	//printf("Entered: %s, File: %s \n", perm_string, file_perm);	
	if(strcmp(perm_string, file_perm) == 0){ //PERMISSIONS MATCH
		
		printf("%s\n", filename);
	}

	free(file_perm); //for malloc
}

int recurse_dir(char* directory, char * perm_string){

	DIR *direct;
        if ((direct = opendir(directory)) == NULL) { //attempts to open the directory
                fprintf(stderr, "Error: Cannot stat '%s'. %s.\n",
                directory, strerror(errno)); //if it doesnt exist, or something else, it wont stat
                exit(EXIT_FAILURE);
        }

	char file_path[PATH_MAX + 1]; //extra space for the ending /	

	if (strcmp(directory, "/")) { //when the path is not the root (/)
        	size_t copy_len = strnlen(directory, PATH_MAX); //finds which is shorter
        	memcpy(file_path, directory, copy_len); //copies the full length of directory name to file_path
        	file_path[copy_len] = '\0'; //adds null terminator to file_path
    	}

	size_t length = strlen(file_path) + 1;
    	file_path[length - 1] = '/'; //adds / to prepend next filename
    	file_path[length] = '\0'; //null terminates

    	struct dirent *file;
    	struct stat sb;
    	while ((file = readdir(direct)) != NULL) { //skips . and ..
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
	    	matches_perm(file_path, perm_string);
		recurse_dir(file_path, perm_string); //RECURSIVE CALL
        } else {
		matches_perm(file_path, perm_string);
        }
    }
	closedir(direct);

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
				fprintf(stderr,"Error: Unknown option '%c' received.\n", optopt); //Add '-'
				return EXIT_FAILURE;
			default:
				return EXIT_FAILURE;
		}			
	}

	//printf("=======\n dlfag: %d, pflag: %d, hflag: %d \n", dflag, pflag, hflag);
	//printf("directory: %s, permission: %s\n=======\n", directory, perm);
	
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
		return EXIT_FAILURE;
	}
	
	recurse_dir(directory, perm);	
	
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
	
	//recurse_dir(directory, perm); //goes to methods that perform the recursion
	
	return EXIT_SUCCESS;	
}
