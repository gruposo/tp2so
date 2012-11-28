#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#define SERVER_FILE "/home/juan/Escritorio/.Server/.cvs"
#define SERVER_PATH "/home/juan/Escritorio/.Server/TP"
#define DATE_SIZE 25

int
main(int argc, char ** argv) {
	FILE * client_file = fopen(".cvs","r");
	FILE * client_map;
	int version, server_inode, client_inode,c, dim = 0;
	char client_path[500];
	char aux_client_path[500];
	char server_path[500];
	char * current_directory;
	int exists = 0;
	DIR *dir;
	struct dirent * ent;
	
	current_directory = getcwd(NULL,0);
	
	if(client_file == NULL) {
		printf("There is no existing project in this folder\n");
		exit(1);
	}
	
	if(argc != 2) {
		printf("Incorrect number of arguments. Please enter only one file\n");
		fclose(client_file);
		exit(1);
	}
	 
	client_map = fopen(".map","r");
	fseek(client_file,DATE_SIZE,SEEK_SET);
	fseek(client_map,DATE_SIZE,SEEK_SET);
	
	while((c = fgetc(client_file)) != EOF) {
		if(c == '\n') {
			client_path[dim] = 0;
			dim = 0;
			fscanf(client_map,"%d-%d-%d\n",&version,&server_inode,&client_inode);
			sprintf(aux_client_path,"%s%s%s",current_directory,"/",argv[1]);
			if(strcmp(client_path,aux_client_path) == 0) {
				exists = 1;
				sprintf(server_path,"%s%s%d",SERVER_PATH,"/",server_inode);
				break;
			}
		} else {
			client_path[dim++] = c;
		}
	}
	
	if(exists) {
		printf("*******************************************************\n");
		printf("		FILE: %s, VERSION: %d\n",argv[1],version);
		printf("*******************************************************\n");
		dir = opendir(server_path);
		while ((ent = readdir (dir)) != NULL) {
      		if ((strcmp(ent->d_name, ".") != 0) && (strcmp(ent->d_name, "..")!=0) ) {
      			printf("File: %s, ",strtok(ent->d_name,"("));
				printf("Version: %s\n",strtok(NULL,")"));
			}
		}
		closedir (dir);
	} else {
		printf("The file %s is not on the cvs\n",argv[1]);
	}
	
	fclose(client_file);
	fclose(client_map);
	
	return 0;
}