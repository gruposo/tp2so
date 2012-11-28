#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#define DATE_SIZE 25
#define SERVER_FILE "/home/juan/Escritorio/.Server/.cvs"
#define SERVER_DELETE "/home/juan/Escritorio/.Server/.deletes"
#define SERVER_DELETE_AUX "/home/juan/Escritorio/.Server/.deletesaux"
#define SERVER_PATH "/home/juan/Escritorio/.Server/TP"

void copyFile(char * clientpath, char * serverpath);

int
main(int argc, char ** argv) {
	FILE * client_file = fopen(".cvs","r+");
	FILE * server_file = fopen(SERVER_FILE,"r+");
	FILE * client_map = fopen(".map","r+");
	FILE * server_deletes;
	FILE * server_deletes_aux;
	char client_date[DATE_SIZE];
	char server_date[DATE_SIZE];
	char * current_directory;
	time_t current_time;
	time_t client_time;
	int version,server_inode,available = 0;
	char buffer[100];
	char path_server[500];
	char path_client[500];
	char file[50];
	char * folders;
	struct stat buf;
	
	current_directory = getcwd(NULL,0);
	time(&current_time);
	
	if(client_file == NULL) {
		printf("There is no existing project in this folder\n");
		exit(1);
	}
	
	if(server_file == NULL) {
		printf("Couldn't connect to the server\n");
		fclose(client_file);
		fclose(client_map);
		exit(1);
	}
	
	if(argc != 2) {
		printf("Incorrect number of arguments\n");
		fclose(client_file);
		fclose(server_file);
		fclose(client_map);
		exit(1);
	}
	
	if(controlDates(client_date,server_date,&client_time)) {
		printf("There is a newer version. Please update first\n");
		fclose(client_file);
		fclose(server_file);
		fclose(client_map);
		exit(1);
	}
	
	server_deletes = fopen(SERVER_DELETE,"r+");
	
	while(!feof(server_deletes)) {
		fscanf(server_deletes,"%d-%d-%s\n",&version,&server_inode,buffer);
		if(strcmp(buffer,argv[1]) == 0) {
			available = 1;
			break;
		}
	}
	
	if(available) {
		server_deletes_aux = fopen(SERVER_DELETE_AUX,"w+");
		fprintf(client_file,"%s",ctime(&current_time));
		fprintf(client_map,"%s",ctime(&current_time));
		fprintf(server_file,"%s",ctime(&current_time));
		strcpy(path_client,current_directory);
		fseek(client_file,0L,SEEK_END);
		fseek(client_map,0L,SEEK_END);
		fseek(server_file,0L,SEEK_END);
		folders = strtok(buffer,"/");
		sprintf(path_client,"%s%s%s",path_client,"/",folders);
		strcpy(file,folders);
		mkdir(path_client, S_IRWXU | S_IRWXG | S_IRWXO);
		while((folders = strtok(NULL,"/"))) {
			sprintf(path_client,"%s%s%s",path_client,"/",folders);
			mkdir(path_client,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
			strcpy(file,folders);
		}
		rmdir(path_client);
		sprintf(path_server,"%s/%d/%s(%d)",SERVER_PATH,server_inode,file,version);
		copyFile(argv[1],path_server);
		stat(argv[1],&buf);
		fprintf(client_file,"%s/%s\n",current_directory,argv[1]);
		fprintf(client_map,"%d-%d-%d\n",version,server_inode,(int)buf.st_ino);
		fprintf(server_file,"%d-%d-%s\n",server_inode,version,argv[1]);
		fseek(server_deletes,0L,SEEK_SET);
		while(!feof(server_deletes)) {
			fscanf(server_deletes,"%d-%d-%s\n",&version,&server_inode,buffer);
			if(strcmp(argv[1],buffer) != 0) {
				fprintf(server_deletes_aux,"%d-%d-%s\n",version,server_inode,buffer);
			}
		}
		fclose(server_deletes);
		fclose(server_deletes_aux);
		remove(SERVER_DELETE);
		rename(SERVER_DELETE_AUX,SERVER_DELETE);
	} else {
		printf("The file is not on the current cvs, does not exist or is not deleted\n");
		fclose(client_file);
		fclose(client_map);
		fclose(server_file);
		exit(1);
	}
	
	fclose(client_file);
	fclose(client_map);
	fclose(server_file);
	
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