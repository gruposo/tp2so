#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#define SERVER_FILE "/home/juan/Escritorio/.Server/.cvs"
#define SERVER_PATH "/home/juan/Escritorio/.Server/TP"
#define SERVER_FILE_AUX "/home/juan/Escritorio/.Server/.cvsaux"
#define DATE_SIZE 25

void copyFile(char * clientpath, char * serverpath);

int
main(int argc, char ** argv) {
	FILE * client_file = fopen(".cvs","r+");
	FILE * server_file = fopen(SERVER_FILE,"r");
	FILE * client_map_aux;
	FILE * server_file_aux;
	FILE * client_map;
	char client_date[DATE_SIZE];
	char server_date[DATE_SIZE];
	time_t client_time;
	time_t current_time;
	char * current_directory;
	char complete_path[500];
	char client_path[500];
	char server_path[500];
	char server_path2[500];
	int dim = 0, c,available = 0;
	int version, server_inode, client_inode;
	char path[100];
	char * folders;
	char file[50];
	struct stat buf;
	
	time(&current_time);
	current_directory = getcwd(NULL,0);
	
	if(client_file == NULL) {
		printf("There is no existing project in this folder\n");
		exit(1);
	}
	
	if(server_file == NULL) {
		printf("Couldn't connect to the server\n");
		fclose(client_file);
		exit(1);
	}
	
	if(argc != 3 || atoi(argv[1]) != 0 || atoi(argv[2]) == 0) {
		printf("Incorrect number of arguments or order of arguments. Please enter only one file and one version\n");
		fclose(client_file);
		fclose(server_file);
		exit(1);
	}
	
	strcpy(client_date,fgets(client_date,DATE_SIZE,client_file));
	strcpy(server_date,fgets(server_date,DATE_SIZE,server_file));
	fseek(server_file,(long)DATE_SIZE,SEEK_SET);
	
	if(controlDates(client_date,server_date,&client_time)) {
		printf("There is a newer version. Please update first\n");
		fclose(client_file);
		fclose(server_file);
		exit(1);
	}
	sprintf(complete_path,"%s%s%s",current_directory,"/",argv[1]);
	
	client_map = fopen(".map","r");
	fseek(client_file,(long)DATE_SIZE,SEEK_SET);
	fseek(client_map,(long)DATE_SIZE,SEEK_SET);
	while((c = fgetc(client_file)) != EOF) {
		if(c == '\n') {
			client_path[dim] = 0;
			dim = 0;
			fscanf(client_map,"%d-%d-%d\n",&version,&server_inode,&client_inode);
			if(strcmp(client_path,complete_path) == 0) {
				available  = 1;
				break;
			}
		} else {
			client_path[dim++] = c;
		}
	}
	if(!available) {
		printf("The file %s is not on the current cvs project\n",argv[1]);
		fclose(client_file);
		fclose(server_file);
		fclose(client_map);
		exit(1);
	} else {
		if(version == atoi(argv[2])) {
			printf("You already have this version\n");
			fclose(client_file);
			fclose(server_file);
			fclose(client_map);
			exit(1);
		} else if(atoi(argv[2]) > version) {
			printf("The version does not exist\n");
			fclose(client_file);
			fclose(server_file);
			fclose(client_map);
			exit(1);
		}
	}
	
	client_map_aux = fopen(".mapaux","w+");
	server_file_aux = fopen(SERVER_FILE_AUX,"w+");
	fseek(client_map,(long)DATE_SIZE,SEEK_SET);
	fprintf(client_map_aux,"%s",ctime(&current_time));
	fprintf(server_file_aux,"%s",ctime(&current_time));
	fseek(client_file,0L,SEEK_SET);
	fprintf(client_file,"%s",ctime(&current_time));
	dim = 0;
	
	while((c = fgetc(client_file)) != EOF) {
		if(c == '\n') {
			client_path[dim] = 0;
			dim = 0;
			fscanf(client_map,"%d-%d-%d\n",&version,&server_inode,&client_inode);
			fscanf(server_file,"%d-%d-%s\n",&server_inode,&version,path);
			if(strcmp(client_path,complete_path) == 0) {
				strtok(complete_path,"/");
				while((folders = strtok(NULL,"/")) != NULL) {
					strcpy(file,folders);
				}
				sprintf(server_path,"%s%s%d%s%s%s%d%s",SERVER_PATH,"/",server_inode,"/",file,"(",atoi(argv[2]),")");
				copyFile(argv[1],server_path);
				version += 1;
				sprintf(server_path2,"%s%s%d%s%s%s%d%s",SERVER_PATH,"/",server_inode,"/",file,"(",version,")");
				copyFile(server_path2,server_path);
				stat(argv[1],&buf);
				client_inode = buf.st_ino;
			}
			fprintf(client_map_aux,"%d-%d-%d\n",version,server_inode,client_inode);
			fprintf(server_file_aux,"%d-%d-%s\n",server_inode,version,path);
		} else {
			client_path[dim++] = c;
		}
	}
		
	fclose(client_file);
	fclose(client_map);
	fclose(server_file);
	fclose(server_file_aux);
	fclose(client_map_aux);
	remove(".map");
	rename(".mapaux",".map");
	remove(SERVER_FILE);
	rename(SERVER_FILE_AUX,SERVER_FILE);
	
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
	FILE * fileserver = fopen(serverpath,"r");
	FILE * fileclient = fopen(clientpath,"w+");
	int c;
	
	while((c = fgetc(fileserver)) != EOF) {
		fputc(c,fileclient);
	}
	
	fclose(fileserver);
	fclose(fileclient);  
}