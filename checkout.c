#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#define SERVER_FILE "/home/juan/Escritorio/.Server/.cvs"
#define SERVER_PATH "/home/juan/Escritorio/.Server/TP"
#define BLOCK 10
void copyFile(char * clientpath, char * serverpath);

int
main(void) {
	FILE * client_file = fopen(".cvs","r");
	FILE * server_file = fopen(SERVER_FILE,"r");
	FILE * client_map;
	char * buffer = NULL;
	char * folders;
	char * aux;
	char * filename;
	char path_client[500];
	char path_server[500];
	int c, version, inode, dim = 0;
	time_t current_time;
	struct stat buf;
	time(&current_time);
	
	if(client_file != NULL) {
		printf("There is an existing project in this folder\n");
		fclose(client_file);
		exit(1);
	}
	
	if(server_file == NULL) {
		printf("Couldn't connect to the server\n");
		exit(1);
	}
	
	
	if((mkdir("TP", S_IRWXU | S_IRWXG | S_IRWXO)) == -1) {
		printf("Couldn't create the main folder\n");
		exit(1);
	}
	
	client_file = fopen("TP/.cvs","w+");
	client_map = fopen("TP/.map","w+");
	
	fprintf(client_file,"%s",ctime(&current_time));
	fprintf(client_map,"%s",ctime(&current_time));
	sprintf(path_client,"%s%s", getcwd(NULL,0), "/TP");
	fseek(server_file,25L,SEEK_SET);
	while((c = fgetc(server_file)) != EOF) {
		if(c == '\n') {
			buffer[dim] = 0;
			dim = 0;
			inode = atoi(strtok(buffer,"-"));
			version = atoi(strtok(NULL,"-"));
			while((folders = strtok(NULL,"/")) != NULL) {
				sprintf(path_client,"%s%s%s",path_client,"/",folders);
				mkdir(path_client,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
				filename = folders;
			}
			rmdir(path_client);
			sprintf(path_server,"%s%s%d%s%s%s%d%s",SERVER_PATH,"/",inode,"/",filename,"(",version,")");
			copyFile(path_client,path_server);
			stat(path_client, &buf);
			fprintf(client_file,"%s\n",path_client);
			fprintf(client_map,"%d-%d-%d\n",version,inode,(int)(buf.st_ino));
			sprintf(path_client,"%s%s", getcwd(NULL,0), "/TP");
		} else {
			if(dim % BLOCK == 0) {
				aux = realloc(buffer,(dim + BLOCK + 1) * sizeof(char));
				if(aux == NULL) {
					free(buffer);
					printf("Application out of memory\n");
					exit(1);
				} else {
					buffer = aux;
				}
			}
			buffer[dim++] = c;
		}
	}
	fclose(client_file);
	fclose(client_map);
	
	return 0;
}

void
copyFile(char * clientpath, char * serverpath) {
	FILE * fileserver = fopen(serverpath,"r");
	FILE * fileclient = fopen(clientpath,"w+");
	int c;
	
	while((c = fgetc(fileserver)) != EOF) {
		fputc(c,fileclient);
	}
	
	fclose(fileserver);
	fclose(fileclient);  
}
