/*
Family Name: Merante
Given Name: Samuel
Section: Z
Student Number: 212795670
CS Login: smerante
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#define TEN_MILLIS_IN_NANOS 10000000
#define ENCRPYTION 0
#define DECRYPTION 1
#define UNPROCESSED 2
#define CHANGING 3
#define PROCESSED 4
#define DONE 4

typedef struct BufferItem BufferItem;
typedef struct Buffer_Node Buffer_Node;
typedef struct Buffer_Queue Buffer_Queue;

volatile Buffer_Queue common_buffer;
int KEY,nIN,nWORK,nOUT,bufSize, runningThreads,mode,doneReadingFile,doneProcessingData;
FILE *file_in,*file_out;
pthread_mutex_t File_Reader_Mutex,Buffer_Writing_Mutex,File_Writing_Mutex; 
struct BufferItem {
	int byte;
	int offset;
	int state;
};

struct Buffer_Node {
   BufferItem *data;
   Buffer_Node *next;
   Buffer_Node *prev;
};
struct Buffer_Queue {
    int size;
    Buffer_Node *front;
    Buffer_Node *back;
};

/* performs basic initialization on the queue `q' */
void initializeBuffer(volatile Buffer_Queue *q) {
    q->front = q->back = NULL;
    q->size = 0;
}

Buffer_Node *createBufferNode(BufferItem *p) {
    Buffer_Node *node = (Buffer_Node*) malloc(sizeof(Buffer_Node));
    if (node == NULL) printf("out of memory");
    node->data = p;
    node->next = NULL;
    node->prev = NULL;
    return node;
}

/*enqueues a bufferedItem to the end of the Queue*/
void enqueueBufferItem(volatile Buffer_Queue *q, BufferItem *p) {
    Buffer_Node *node = createBufferNode(p);

    if (q->front == NULL) {
        assert(q->back == NULL);
        q->front = q->back = node;
    } else {
        assert(q->back != NULL);
        q->back->next = node;
        q->back = node;
    }
    q->size++;
}

/*removes a node from a Buffer_q*/
void removeBufferNode(volatile Buffer_Queue *q, Buffer_Node *p)
{
	if(p == q->front)
		q->front = p->next;
	if(p->next)
		p->next->prev = p->prev;
	if(p->prev)
		p->prev->next = p->next;
	q->size--;
	if(q->front == NULL)
		q->back = NULL;
}

void *InThreads(void *param) 
{
	struct timespec t;
	unsigned int seed=0;
	t.tv_sec = 0;
	t.tv_nsec = rand_r(&seed)%(TEN_MILLIS_IN_NANOS+1);
	nanosleep(&t, NULL);
	
	while(!feof(file_in)){ //havent reached end of file
		if(common_buffer.size <= bufSize){// if buffer isnt full
			pthread_mutex_lock(&File_Reader_Mutex); /*reading from file CRITICAL SECTION*/ 
			int offset = ftell(file_in); 
			int byte;
			if((byte = fgetc(file_in)) == EOF){//if byte is at EOF
				runningThreads--;	//decrease amount of running threads
				doneReadingFile++;	//increase the amount of threads done reading
				pthread_mutex_unlock(&File_Reader_Mutex); //unlock and leave thread
				pthread_exit(0); 	//exit pthread
			} 
			
			pthread_mutex_unlock(&File_Reader_Mutex); /*done reading from file CRITICAL SECTION*/
			
			t.tv_nsec = rand_r(&seed)%(TEN_MILLIS_IN_NANOS+1);
			nanosleep(&t, NULL); //sleep after reading a byte

			BufferItem *p;
			p = (BufferItem *)malloc(sizeof(BufferItem));
			p->byte = byte;
			p->offset = offset;
			p->state = UNPROCESSED;
		
			pthread_mutex_lock(&Buffer_Writing_Mutex);
			/*write to buffer CRITICAL SECTION*/
			enqueueBufferItem(&common_buffer,p);
			/*write to buffer CRITICAL SECTION*/
			pthread_mutex_unlock(&Buffer_Writing_Mutex);
			
		}
		
	}
	runningThreads--;
	doneReadingFile++;
	pthread_exit(0); 
}

void *WorkThreads(void *param) 
{
	struct timespec t;
	unsigned int seed=0;
	t.tv_sec = 0;
	t.tv_nsec = rand_r(&seed)%(TEN_MILLIS_IN_NANOS+1);
	nanosleep(&t, NULL);
	while(doneReadingFile < nIN){ //while threads are still reading files
		while(common_buffer.size == 0) { //sleep if buffer is empty
			t.tv_nsec = rand_r(&seed)%(TEN_MILLIS_IN_NANOS+1);
			nanosleep(&t, NULL);
		}
		Buffer_Node *bn = common_buffer.front;
		while(bn != NULL){ // read and set buffer to changing for work threads
			if(bn->data->state == UNPROCESSED)
				bn->data->state = CHANGING;
			if(bn->next)bn=bn->next;
			else bn = NULL;
		}
			
		pthread_mutex_lock(&Buffer_Writing_Mutex);
				/*write to buffer CRITICAL SECTION*/	
		bn = common_buffer.front;
		while(bn != NULL){ // check all of the buffer to encrypt/decrypt data
			if(bn->data->state == CHANGING){
				if(mode == ENCRPYTION && bn->data->byte>31 && bn->data->byte<127) 
					bn->data->byte = (((int)bn->data->byte-32)+2*95+KEY)%95+32 ;
				else if(mode == DECRYPTION && bn->data->byte>31 && bn->data->byte<127)
					 bn->data->byte = (((int)bn->data->byte-32)+2*95-KEY)%95+32 ;
				bn->data->state = PROCESSED;
			}
			if(bn->next)bn=bn->next; //go to next spot in buffer
			else bn = NULL;
		}
				/*write to buffer CRITICAL SECTION*/
		pthread_mutex_unlock(&Buffer_Writing_Mutex);	
	} //while threads still readingFile

	runningThreads--;
	doneProcessingData++;
	pthread_exit(0);
}

void *OutThreads(void *param) 
{
	struct timespec t;
	unsigned int seed=0;
	t.tv_sec = 0;
	t.tv_nsec = rand_r(&seed)%(TEN_MILLIS_IN_NANOS+1);
	nanosleep(&t, NULL);
	while(/*size < file_in_size*/doneProcessingData < nWORK || common_buffer.size > 0)
	{ //while theres still threads running or the buffer is not empty
		Buffer_Node *bn = common_buffer.front;
		while(bn != NULL){
			if(bn->data->state == PROCESSED){//if bufferedItem has been changed
				pthread_mutex_lock(&File_Writing_Mutex);
				/*write to FILE CRITICAL SECTION*/
				if(fseek(file_out,bn->data->offset,SEEK_SET) == EOF)
					printf("err seeking writing dest to file\n");
				if(fputc(bn->data->byte,file_out) == EOF)
					printf("err writing byte to file\n");
				/*write to FILE CRITICAL SECTION*/
				
				pthread_mutex_unlock(&File_Writing_Mutex);

				pthread_mutex_lock(&Buffer_Writing_Mutex);
					/*write to buffer CRITICAL SECTION*/
						removeBufferNode(&common_buffer, bn);

					bn->data->state = DONE;
					/*write to buffer CRITICAL SECTION*/
				pthread_mutex_unlock(&Buffer_Writing_Mutex);

			}
			else{//sleep after checking for a bufferedItem 
				t.tv_nsec = rand_r(&seed)%(TEN_MILLIS_IN_NANOS+1);
				//nanosleep(&t, NULL);
			}
			if(bn->next)bn=bn->next;
			else bn = NULL;
			
		}
	}
	
	runningThreads--;
	pthread_exit(0);
}

int main(int argc, char *argv[])
{

	initializeBuffer(&common_buffer);
	
	KEY = atoi(argv[1]);
	if(KEY < 0) mode = DECRYPTION;
	else mode = ENCRPYTION;
	KEY = abs(KEY);
	nIN = atoi(argv[2]);
	nWORK = atoi(argv[3]);
	nOUT = atoi(argv[4]);
	file_in = fopen(argv[5],"r");
	file_out = fopen(argv[6],"w");
	bufSize = atoi(argv[7]);
	doneReadingFile = 0;	//keeps track of done IN threads
	doneProcessingData = 0;	//keys track of done WORK threads
	runningThreads = nIN+nWORK+nOUT;	//keeps track of remaining total threads

	pthread_t tid; //thread id
	pthread_attr_t attr; //attributes for thread
	pthread_attr_init(&attr); // attributes for this current thread

	pthread_mutex_init(&File_Reader_Mutex,NULL);	//lock for reading from file_in
	pthread_mutex_init(&Buffer_Writing_Mutex,NULL); //lock for writing to the buffer
	pthread_mutex_init(&File_Writing_Mutex,NULL); //lock for writing to file_out

	int args = 1; /*create all threads*/
	for(args=0;args<nIN;args++){
		pthread_create(&tid,&attr,InThreads,NULL);
	} 
	for(args=0;args<nWORK;args++){
		pthread_create(&tid,&attr,WorkThreads,NULL);
	} 
	for(args=0;args<nOUT;args++){
		pthread_create(&tid,&attr,OutThreads,NULL);
	} 

	
	
	while(runningThreads > 0) sleep(1);// wait for all threads to finish
	fclose(file_in); //close input file
	fclose(file_out); //close output file
	return(0);
}















































