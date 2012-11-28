#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#define SERVER_FILE "/home/juan/Escritorio/.Server/.cvs"
#define SERVER_PATH "/home/juan/Escritorio/.Server/TP"
#define DATE_SIZE 25

int
main(void) {
	FILE * client_file = fopen(".cvs","r");
	FILE * server_file = fopen(SERVER_FILE,"r");
	FILE * client_map;
	char * current_directory;
	int len,i,server_inode,client_inode;
	char path[500];
	int version;
	
	current_directory = getcwd(NULL,0);
	len = strlen(current_directory) + 1;
	
	if(client_file == NULL) {
		printf("There is no existing project in this folder\n");
		exit(1);
	}
	
	if(server_file == NULL) {
		printf("Couldn't connect to the server\n");
		exit(1);
	}
	
	client_map = fopen(".map","r");
	
	printf("*******************************************************\n");
	printf("			CLIENTE\n");
	printf("*******************************************************\n");
	printf("Fecha del ultimo update hecho: ");
	for(i = 0; i < DATE_SIZE; i++) {
		printf("%c",fgetc(client_file));
	}
	
	fseek(client_map,DATE_SIZE,SEEK_SET);
	fgetc(client_file);
	
	if(!feof(client_file)) {
		fseek(client_file,-1L,SEEK_CUR);
		while(!feof(client_file)) {
			fscanf(client_file,"%s\n",path);
			fscanf(client_map,"%d-%d-%d\n",&version,&server_inode,&client_inode);
			printf("File: %s, version: %d\n",&path[len],version);
		}
	}
	
	printf("*******************************************************\n");
	printf("			SERVIDOR\n");
	printf("*******************************************************\n");
	printf("Fecha del ultimo commit hecho: ");
	for(i = 0; i < DATE_SIZE; i++) {
		printf("%c",fgetc(server_file));
	}
	
	fgetc(server_file);
	
	if(!feof(server_file)) {
		fseek(server_file,-1L,SEEK_CUR);
		while(!feof(server_file)) {
			fscanf(server_file,"%d-%d-%s\n",&server_inode,&version,path);
			printf("File: %s, version: %d\n",path, version);
		}
	}
	
	fclose(client_map);
	fclose(client_file);
	fclose(server_file);
	
	return 0;
}