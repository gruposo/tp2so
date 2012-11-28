#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#define DATE_SIZE 25

int
main(int argc, char ** argv) {
	FILE * client_file = fopen(".cvs","r");
	FILE * changes = fopen(".changes","a");
	FILE * client_map = fopen(".map","r");
	int i, available = 0, version, server_inode, client_inode;
	struct stat buf;
	char * current_directory;
	
	current_directory = getcwd(NULL,0);
	
	if(client_file == NULL) {
		printf("There is no existing project in this folder\n");
		fclose(changes);
		remove(".changes");
		exit(1);
	}
	
	for(i = 1; i < argc; i++) {
		fseek(client_map,(long)DATE_SIZE,SEEK_SET);
		available = 0;
		while(!feof(client_map)) {
			fscanf(client_map,"%d-%d-%d\n",&version,&server_inode,&client_inode);
			stat(argv[i], &buf);
			if(buf.st_ino == client_inode) {
				available = 1;
				break;
			}
		}
		if(available) {
			fprintf(changes,"%s%s%s%s\n","D-",current_directory,"/",argv[i]);
		} else {
			printf("The file %s is not on this cvs project\n",argv[i]);
		}
	}
	
	fclose(client_file);
	fclose(client_map);
	fclose(changes);
	
	return 0;
}
