#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#define BUFFER_SIZE 16
#define ARGMAX 50
#define READ 1024

/* Circular buffer of integers. */

struct prodcons {
  int buffer[BUFFER_SIZE];      /* the actual data */
  pthread_mutex_t lock;         /* mutex ensuring exclusive access to buffer */
  int readpos, writepos;        /* positions for reading and writing */
  pthread_cond_t notempty;      /* signaled when buffer is not empty */
  pthread_cond_t notfull;       /* signaled when buffer is not full */
};

/* Initialize a buffer */

void init(struct prodcons * b)
{
  pthread_mutex_init(&b->lock, NULL);
  pthread_cond_init(&b->notempty, NULL);
  pthread_cond_init(&b->notfull, NULL);
  b->readpos = 0;
  b->writepos = 0;
}

/* Store an integer in the buffer */

void put(struct prodcons * b, int data)
{
  pthread_mutex_lock(&b->lock);
  /* Wait until buffer is not full */
  while ((b->writepos + 1) % BUFFER_SIZE == b->readpos) {
    pthread_cond_wait(&b->notfull, &b->lock);
    /* pthread_cond_wait reacquired b->lock before returning */
  }
  /* Write the data and advance write pointer */
  b->buffer[b->writepos] = data;
  b->writepos++;
  if (b->writepos >= BUFFER_SIZE) b->writepos = 0;
  /* Signal that the buffer is now not empty */
  pthread_cond_signal(&b->notempty);
  pthread_mutex_unlock(&b->lock);
}

/* Read and remove an integer from the buffer */

int get(struct prodcons * b)
{
  int data;
  pthread_mutex_lock(&b->lock);
  /* Wait until buffer is not empty */
  while (b->writepos == b->readpos) {
    pthread_cond_wait(&b->notempty, &b->lock);
  }
  /* Read the data and advance read pointer */
  data = b->buffer[b->readpos];
  b->readpos++;
  if (b->readpos >= BUFFER_SIZE) b->readpos = 0;
  /* Signal that the buffer is now not full */
  pthread_cond_signal(&b->notfull);
  pthread_mutex_unlock(&b->lock);
  return data;
}

struct prodcons buffer;
pthread_t printer;
int pt_ch;

int producer(char *filename)
{
  int pw = open(filename, O_RDONLY);
  if(pw==-1) {
    return 0;
  }
  else {
    put(&buffer, pw);
    return 1;
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

void realPrint(int ch, char *c) {
  write(ch, c, 1);
  usleep(50000);
}

void *print () {
  while(1){
    char buf[READ];
    int i, nr;

    int pw = get(&buffer);
    do{
      nr= read(pw, buf, READ);
      for(i=0; i<nr; i++)
        realPrint(pt_ch, buf+i);
    }while (nr ==READ);
    close(pw);}
  }

  void myFree(char *buff){
    int i;
    for(i=0;i<READ;i++){
      buff[i]=0;
    }
  }

  int main(int argc, char *argv[])
{ /*
  Open the pseudo terminal argv[1] or exit in case of error
  */
  pt_ch= open(argv[1],O_WRONLY);
  if(pt_ch<0){
    printf("Could not open pseudo terminal %s\n", argv[1]);
    exit(1);
  }

  char buff[READ]; //contains the message send from the client
  char *cmd[ARGMAX];//contains the comand to execute
  struct prodcons * b = &buffer;
  
  /*
  Server, buffer, thread initialization
  */
  int s = myServerSocket(atoi(argv[2]));
  init(&buffer); 
  pthread_create(&printer, NULL, print, 0);

  /*
  Infinite cicle to handle client requests
  */
  while(1) {
    int sc = myAcceptServerSocket(s);
    myReadSocket( sc, buff, READ );

    if(makeargv( buff, cmd)>1 ){ 
      if((b->writepos + 1) % BUFFER_SIZE == b->readpos) {
        myFree(buff);
        sprintf(buff, "b");

        myWriteSocket( sc, buff ,sizeof(char));
      }                            
      else if(producer(cmd[1])==1) { 
        myFree(buff);
        sprintf(buff, "t");

        myWriteSocket( sc, buff ,sizeof(char));
      } 
      else {
        myFree(buff);
        sprintf(buff, "f");

        myWriteSocket( sc, buff,sizeof(char));
      }                            
    }
    /*else if(makeargv( buff, cmd)==1 ){

      if((b->writepos + 1) % BUFFER_SIZE == b->readpos) {
       myFree(buff);
       sprintf(buff, "f");

       myWriteSocket( sc, buff,sizeof(char));
      }
      else{
        myFree(buff);
        sprintf(buff, "%d\n", b->writepos);

        myWriteSocket( sc, buff,strlen(buff));
      }*/

      else if(makeargv( buff, cmd)==1 ){
        int status = (((b->writepos - b->readpos) + BUFFER_SIZE) % BUFFER_SIZE);

        myFree(buff);
        sprintf(buff, "%d\n", status);

        myWriteSocket( sc, buff,strlen(buff));
      
   }
 }	


 void * retval;
  /* Wait until producer and consumer finish. */
 pthread_join(printer, &retval);
 return 0;
}



