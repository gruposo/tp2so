#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#define SERVER_FILE_AUX "/home/juan/Escritorio/.Server/.cvsaux"
#define SERVER_FILE "/home/juan/Escritorio/.Server/.cvs"
#define SERVER_PATH "/home/juan/Escritorio/.Server/TP"
#define SERVER_DELETE "/home/juan/Escritorio/.Server/.deletes"
#define DATE_SIZE 25
#define BLOCK 10

int controlDates(char * client_date, char * server_date,time_t * time);
void copyFile(char * clientpath, char * serverpath);
int getMonth(char * month);
int getDay(char * day);


int
main(void) {
	FILE * client_file = fopen(".cvs","r+");
	FILE * client_map = fopen(".map","r+");
	FILE * server_file = fopen(SERVER_FILE,"r+");
	FILE * changes = fopen(".changes","r");
	FILE * server_file_aux;
	FILE * client_map_aux;
	FILE * client_file_aux;
	FILE * server_deletes;
	char client_date[DATE_SIZE];
	char server_date[DATE_SIZE];
	char ** deletes = NULL;
	int c,dim = 0,current_directory, delete_qty = 0, available = 0;
	char * buffer = NULL;
	char * bufferaux = NULL;
	char * aux;
	char change[2];
	char * folders;
	char file[30];
	char path_server[500];
	char path_server2[500];
	double diff;
	int server_inode;
	int client_inode;
	int version;
	struct stat buf;
	struct stat buf2;
	time_t current_time;
	time_t client_time;
	int i;
	
	current_directory = strlen(getcwd(NULL,0));
	time(&current_time);
	
	if(client_file == NULL) {
		printf("There is no existing project in this folder\n");
		exit(1);
	}
	
	if(server_file == NULL) {
		printf("Couldn't connect to the server\n");
		exit(1);
	}
	
	strcpy(client_date,fgets(client_date,DATE_SIZE,client_file));
	strcpy(server_date,fgets(server_date,DATE_SIZE,server_file));
	fseek(client_file,(long)DATE_SIZE,SEEK_SET);
	fseek(server_file,(long)DATE_SIZE,SEEK_SET);
	
	if(controlDates(client_date,server_date,&client_time)) {
		printf("There is a newer version. Please update first\n");
		fclose(client_file);
		fclose(client_map);
		fclose(server_file);
		exit(1);
	}
	
	if (chmod(SERVER_FILE, 0x0000) != 0){
		perror("Unable to change file mode");
		exit(1);
	}
	
	if(changes != NULL) {
		while((c = fgetc(changes)) != EOF) {
			if(c == '\n') {
				buffer[dim] = 0;
				bufferaux = calloc((dim + 1), sizeof(char));
				strcpy(bufferaux,buffer);
				strcpy(change,strtok(buffer,"-"));
				if(strcmp(change,"A") == 0) {
					while((folders = strtok(NULL,"/")) != NULL) {
						strcpy(file,folders);
					}
					sprintf(path_server,"%s%s%s%s",SERVER_PATH,"/",file,"(1)");
					copyFile(&bufferaux[2],path_server);
					stat(&bufferaux[2],&buf2);
					stat(path_server, &buf);
					sprintf(path_server2,"%s%s%d",SERVER_PATH,"/",(int)(buf.st_ino));
					mkdir(path_server2, S_IRWXU | S_IRWXG | S_IRWXO);
					sprintf(path_server2,"%s%s%d%s%s%s",SERVER_PATH,"/",(int)(buf.st_ino),"/",file,"(1)");
					rename(path_server,path_server2);
					fseek(client_file,0L,SEEK_END);
					fseek(client_map,0L,SEEK_END);
					fprintf(client_file,"%s\n",&bufferaux[2]);
					fprintf(client_map,"%d-%d-%d\n",0,(int)(buf.st_ino),(int)(buf2.st_ino));
				} else {
					delete_qty += 1;
					deletes = realloc(deletes,delete_qty * sizeof(char *));
					deletes[delete_qty - 1] = NULL;
					deletes[delete_qty - 1] = realloc(deletes[delete_qty - 1],strlen(&bufferaux[2]) + 1);
					strcpy(deletes[delete_qty - 1],&bufferaux[2]);
				}
				dim = 0;
				free(bufferaux);
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
		fclose(changes);
		remove(".changes");
	}
	
	fseek(client_file,DATE_SIZE,SEEK_SET);
	fseek(client_map,DATE_SIZE,SEEK_SET);
	
	server_file_aux = fopen(SERVER_FILE_AUX,"w+");
	client_map_aux = fopen(".mapaux","w+");
	client_file_aux = fopen(".cvsaux","w+");
	
	fprintf(client_file_aux,"%s",ctime(&current_time));
	fprintf(client_map_aux,"%s",ctime(&current_time));
	fprintf(server_file_aux,"%s",ctime(&current_time));
	
	server_deletes = fopen(SERVER_DELETE,"a+");
	
	while((c = fgetc(client_file)) != EOF) {
		available = 0;
		if(c == '\n') {
			buffer[dim] = 0;
			dim = 0;
			for(i = 0; i < delete_qty; i++) {
				if(strcmp(deletes[i],buffer) == 0) {
					available = 1;
					break;
				}
			}
			fscanf(client_map,"%d-%d-%d\n",&version,&server_inode,&client_inode);
			if(!available) {
				stat(buffer, &buf);
				diff = difftime(buf.st_mtime, client_time);
				if(diff > 0 && version != 0) {
					version += 1;
					aux = malloc((strlen(buffer) + 1) * sizeof(char));
					strcpy(aux,buffer);
					strtok(aux,"/");
					while((folders = strtok(NULL,"/")) != NULL) {
						strcpy(file,folders);
					}
					sprintf(path_server,"%s%s%d%s%s%s%d%s",SERVER_PATH,"/",server_inode,"/",file,"(",version,")");
					copyFile(buffer, path_server);
					free(aux);
				}
				if(version == 0) {
					version += 1;
				}
				stat(buffer, &buf);
				client_inode = (int)buf.st_ino;
				fprintf(client_map_aux,"%d-%d-%d\n",version,server_inode,client_inode);
				fprintf(server_file_aux,"%d-%d-%s\n",server_inode,version,&buffer[current_directory + 1]);
				fprintf(client_file_aux,"%s\n",buffer);
			} else {
				fprintf(server_deletes,"%d-%d-%s\n",version,server_inode,&(deletes[i][current_directory + 1]));
			}
		} else {
			if(dim % BLOCK == 0) {
				aux = realloc(buffer,(dim + BLOCK + 1) * sizeof(char));
				if(aux == NULL) {
					printf("Application out of memory\n");
					free(buffer);
					exit(1);
				} else {
					buffer = aux;
				}
			}
			buffer[dim++] = c;
		}
	}
	
	if (chmod(SERVER_FILE, S_IREAD|S_IWRITE) != 0){
		perror("Unable to change file mode");
		exit(1);
	}
	
	fclose(client_file);
	fclose(client_file_aux);
	fclose(client_map);
	fclose(server_file);
	fclose(server_file_aux);
	fclose(client_map_aux);
	fclose(server_deletes);
	remove(".map");
	rename(".mapaux",".map");
	remove(SERVER_FILE);
	rename(SERVER_FILE_AUX,SERVER_FILE);
	remove(".cvs");
	rename(".cvsaux",".cvs");
	
	return 0;
}

int
controlDates(char * client_date, char * server_date,time_t * time) {
	struct tm newServer_date;
	struct tm newClient_date;
	time_t server_time;
	time_t client_time;
	char server_day[4] = "\0";
	char client_day[4] = "\0";
	char server_mon[4] = "\0";
	char client_mon[4] = "\0";
	double diff;
	
	strncpy(server_day,server_date,3);
	strncpy(client_day,client_date,3);
	strncpy(server_mon,server_date + 4,3);
	strncpy(client_mon,client_date + 4,3);

	newServer_date.tm_wday = getDay(server_day);
	newClient_date.tm_wday = getDay(client_day);
	newServer_date.tm_mon = getMonth(server_mon);
	newClient_date.tm_mon = getMonth(client_mon);
	
	newServer_date.tm_mday = atoi((&server_date[7]) + 1);
	newClient_date.tm_mday = atoi((&client_date[7]) + 1);
	
	newServer_date.tm_hour = atoi((&server_date[10]) + 1);
	newClient_date.tm_hour = atoi((&client_date[10]) + 1);
	
	newServer_date.tm_min = atoi((&server_date[13]) + 1);
	newClient_date.tm_min = atoi((&client_date[13]) + 1);
	
	newServer_date.tm_sec = atoi((&server_date[16]) + 1);
	newClient_date.tm_sec = atoi((&client_date[16]) + 1);
	
	newServer_date.tm_year = atoi((&server_date[17]) + 3) - 1900;
	newClient_date.tm_year = atoi((&client_date[17]) + 3) - 1900;
	
	newServer_date.tm_isdst = 0;
	newClient_date.tm_isdst = 0;
	
	server_time = mktime(&newServer_date);
	client_time = mktime(&newClient_date);
	*time = mktime(&newClient_date);
	
	diff = difftime(server_time, client_time);
	
	return diff > 0;
}

int
getDay(char * day) {
	if(strcmp(day, "Sun") == 0) {
		return 0;
	} else if(strcmp(day, "Mon") == 0) {
		return 1;
	} else if(strcmp(day, "Tue") == 0) {
		return 2;
	} else if(strcmp(day, "Wed") == 0) {
		return 3;
	} else if(strcmp(day, "Thu") == 0) {
		return 4;
	} else if(strcmp(day, "Fri") == 0) {
		return 5;
	} else {
		return 6;
	}
}

int
getMonth(char * month) {
	if(strcmp(month, "Jan") == 0) {
		return 0;
	} else if(strcmp(month, "Feb") == 0) {
		return 1;
	} else if(strcmp(month, "Mar") == 0) {
		return 2;
	} else if(strcmp(month, "Apr") == 0) {
		return 3;
	} else if(strcmp(month, "May") == 0) {
		return 4;
	} else if(strcmp(month, "Jun") == 0) {
		return 5;
	} else if(strcmp(month, "Jul") == 0){
		return 6;
	} else if(strcmp(month, "Aug") == 0) {
		return 7;
	} else if(strcmp(month, "Sep") == 0) {
		return 8;
	} else if(strcmp(month, "Oct") == 0) {
		return 9;
	} else if(strcmp(month, "Nov") == 0) {
		return 10;
	} else {
		return 11;
	}
}

void
copyFile(char * clientpath, char * serverpath) {
	FILE * fileserver = fopen(serverpath,"w+");
	FILE * fileclient = fopen(clientpath,"r");
	int c;
	
	while((c = fgetc(fileclient)) != EOF) {
		fputc(c,fileserver);
	}
	
	fclose(fileserver);
	fclose(fileclient);  
}
