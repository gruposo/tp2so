#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

int
main(int argc, char ** argv) {
	FILE * client_file = fopen(".cvs","r");
	FILE * changes = fopen(".changes","a");
	FILE * file_add;
	FILE * client_map = fopen(".map","r");
	int i,available = 0,version, server_inode;
	struct stat buf;
	int newinode, oldinode;
	
	if(client_file == NULL) {
		printf("There is no existing project in this folder\n");
		fclose(changes);
		remove(".changes");
		exit(1);
	}
	
	for(i = 1; i < argc; i++) {
		available = 0;
		oldinode = 0;
		if((file_add = fopen(argv[i],"r")) == NULL) {
			printf("The file %s does not exist\n", argv[i]);
		} else {
			stat(argv[i], &buf);
			newinode = (int)buf.st_ino;
			fseek(client_map,25L,SEEK_SET);
			while(!feof(client_map)) {
				fscanf(client_map,"%d-%d-%d",&version,&server_inode,&oldinode);
				if(oldinode == newinode) {
					printf("The file %s is already added to the cvs\n",argv[i]);
					available = 1;
					break;
				}
			}
			if(!available || oldinode == 0) {
				fprintf(changes,"%s%s%s%s\n","A-",getcwd(NULL,0),"/",argv[i]);
			}
			fclose(file_add);
		}
	}
	
	fclose(client_file);
	fclose(client_map);
	fclose(changes);
	
	return 0;
}
