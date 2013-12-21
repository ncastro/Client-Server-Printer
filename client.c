#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mysocks.h"

#define LINESIZE 1024
#define ARGMAX 50

void myFree(char *buff){
	int i;
	for(i=0;i<LINESIZE;i++)
		buff[i] =0;
}

void print(char *filename, char *adress, char *port) {
	char *buff = malloc(LINESIZE * sizeof(char));
	sprintf(buff, "P %s\n", filename);
	int socket = myConnectSocket( adress, atoi(port));
	if(socket != -1){
		myWriteSocket( socket, buff, strlen(buff) );
		myFree(buff);
		myReadSocket(socket, buff, LINESIZE);
		if(strcmp(buff,"f") == 0)
			printf("File not found\n");
		else if(strcmp(buff,"t") == 0)
			printf("Print request has been queued\n");
		else
			printf("Printer is full\n");
	}
	else
		printf("Conection error\n");
}

void status(char *adress, char *port) {
	char *buff = malloc(LINESIZE * sizeof(char));
	sprintf(buff, "S");
	int socket = myConnectSocket( adress, atoi(port));
	if(socket != -1){
		myWriteSocket( socket, buff, strlen(buff) );
		myFree(buff);
		myReadSocket(socket, buff, LINESIZE);
		if(strcmp(buff,"f") == 0)
			printf("Printer is full\n");
		else {
			buff = strtok(buff, " \t\n");
			printf("There're %s in the queue to be printed\n", buff);
		}
	}
}

int makeargv(char *s, char *argv[]){
	int ntokens;

	if(s==NULL || argv==NULL || ARGMAX==0)
		return -1;


	ntokens = 0;
	argv[ntokens]=strtok(s, " \t\n");
	while((argv[ntokens] != NULL) && (ntokens<ARGMAX)) {
		ntokens++;
		argv[ntokens]=strtok(NULL, " \t\n");
	}
	argv[ntokens]=NULL;
	return ntokens;
}

int runcommand(char *argv[], char *server[]){
	if(strcmp(argv[0], "quit") ==0)
		exit(0);
	else if(strcmp(argv[0],"print")==0){
		print(argv[1],server[1],server[2]);
	}
	else if(strcmp(argv[0],"status")==0){
		status(server[1],server[2]);
	}
	else
		printf("Unknown request\n");
}




int main(int argc, char *argv[]) {

	char line[LINESIZE];
	char *av[ARGMAX];

	printf("> "); fflush(stdout);
	while (fgets( line, LINESIZE, stdin ) != NULL ) {
		if(makeargv( line, av)>0 ) runcommand( av,argv );
		printf("> "); fflush(stdout);
	}
}


