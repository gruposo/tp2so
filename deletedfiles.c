#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#define SERVER_PATH "/home/juan/Escritorio/.Server/TP"
#define SERVER_DELETE "/home/juan/Escritorio/.Server/.deletes"
#define DATE_SIZE 25

int
main(void) {
	FILE * client_map = fopen(".map","r");
	FILE * server_delete = fopen(SERVER_DELETE,"r+");
	DIR * dir, * dir2;
	struct dirent * ent, * ent2;
	int version, server_inode, client_inode, available = 0,server_inode2;
	char server_path[500], file[50];
	
	if(client_map == NULL) {
		printf("There is no existing project in this folder\n");
		exit(1);
	}
	
	
	
	printf("*******************************************************\n");
	printf("			DELETED FILES\n");
	printf("*******************************************************\n");
	
	fseek(client_map,DATE_SIZE,SEEK_SET);
	//fgetc(client_map);
	
	
	if(!feof(client_map)) {
	//	fseek(client_map,-1L,SEEK_CUR);
		while(!feof(server_delete)) {
			fseek(client_map,DATE_SIZE,SEEK_SET);
			fscanf(server_delete,"%d-%d-%s\n",&version,&server_inode2,server_path);
			while(!feof(client_map)) {
				fscanf(client_map,"%d-%d-%d\n",&version,&server_inode,&client_inode);
				if(server_inode == server_inode2) {
					available = 1;
					break;
				}
			}
			if(!available) {
				printf("File: %s\n",server_path);
			}
			available = 0;
		}
	}
	
	/*if(!feof(client_map)) {
	//	fseek(client_map,-1L,SEEK_CUR);
		dir = opendir(SERVER_PATH);
		while((ent = readdir(dir)) != NULL) {
			fseek(client_map,DATE_SIZE,SEEK_SET);
			while(!feof(client_map)) {
				fscanf(client_map,"%d-%d-%d\n",&version,&server_inode,&client_inode);
				if(((strcmp(ent->d_name, ".") == 0) || (strcmp(ent->d_name, "..") == 0)) || (atoi(ent->d_name) == server_inode)) {
					available = 1;
					break;
				}
			}
			if(!available) {
				sprintf(server_path,"%s%s%d",SERVER_PATH,"/",atoi(ent->d_name));
				dir2 = opendir(server_path);
				while((ent2 = readdir(dir2)) != NULL) {
					if((strcmp(ent2->d_name, ".") != 0) && (strcmp(ent2->d_name, "..") != 0)) {
						strcpy(file,ent2->d_name);
					}
				}
				strcpy(file,strtok(file,"("));
				printf("File: %s\n",file);
			}
			available = 0;
		}
	}*/
	
	fclose(client_map);
	fclose(server_delete);
	
	return 0;
}