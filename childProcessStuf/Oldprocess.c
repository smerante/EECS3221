// Family Name: Merante
// Given Name: Samuel
// Section: Z
// Student Number: 212795670
// CSE Login: smerante
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <wait.h>
struct VectorPair
{
	char word1[100],word2[100];
	float similarity;
};

//global variables too big to fit on cache stack
struct VectorPair vectorPairs[1000000]; //object for each pair of words
char string[100000][100];//lots of possible words
float vectors[100000][100]; //lots of possible vectors
int numVectors = 0;
	
int cmpSimfunc (const void * a, const void * b){
	const struct VectorPair *va = (struct VectorPair *)a;
	const struct VectorPair *vb = (struct VectorPair *)b;
	//sort it in decreasing order
	if(va->similarity < vb->similarity)
	{
		return 1;
	}
	else if(va->similarity>vb->similarity)
		return -1;
	else
		return 0;
}

float dotProductFunc(float vector1[],float vector2[]){
	int i = 0;
	float product = 0;
	for(i=0;i<numVectors;i++){
		product +=  vector1[i]*vector2[i];
	}
	return product;
}

float powerOf2(float vector1[]){
	return dotProductFunc(vector1,vector1);
}

float cosineSimilarity(float vector1[],float vector2[]){
	float magnitiude = 0,dotProduct = 0;
	dotProduct = dotProductFunc(vector1,vector2);
	magnitiude  = sqrt(powerOf2(vector1))*sqrt(powerOf2(vector2));
	return dotProduct/magnitiude;
}


int main(int argc,char *argv[])
{
	int args; //for command line args
	int i=0,counter = 0; // i -> words, counter -> vectors
	
	int pid = 0,fd[2]; //for multiple process and pipe
	char printStatement[10000]; //for final println
	if(pipe(fd) < 0) printf("Pipe Failed\n");
	for(args=1;args<argc;args++)
	{
		pid = fork();
		if(pid < 0){
			printf("Fork Failed\n");
		}
		else if(pid == 0) /*child process*/
		{
			i=0,counter = 0;
			FILE *file;
			file = fopen(argv[args],"r");
			
			char tempString[10000],word[100];
			int tempBuffer = 0,charsRead = 0,scanning = 1;
			float f = 0; //scanning possible vectors
			while(!feof(file)){
				if(fgets(tempString,10000,file) != NULL){
					scanning = 1;
					tempBuffer=0;
					while(scanning){
						if(sscanf(tempString+tempBuffer,"%s%n",word,&charsRead)==1){
							tempBuffer+=charsRead;//scan through the line read
							if(sscanf(word,"%f",&f)){ //try to scan vector
									vectors[i-1][counter] = f;
									counter++;
								}else{ //scan the string
								strncpy(string[i],word,sizeof(word));
								i++;
								numVectors = counter;
								counter = 0;
							}
						}
						else
							scanning = 0;
					}
				}
			}
	
	
	
			fclose(file);
			
			int j,k; //for computing all possible pairs of cosine sims
			int numPairs = 0;
			for(j=0;j<=i-2;j++){
				for(k=j+1;k<=i-1;k++){
				strcpy(vectorPairs[numPairs].word1,string[j]);
				strcpy(vectorPairs[numPairs].word2,string[k]);
				vectorPairs[numPairs].similarity = cosineSimilarity(vectors[j],vectors[k]);
				numPairs++;
				}
			}
		
			qsort(vectorPairs, numPairs, sizeof(struct VectorPair), cmpSimfunc);
			close(fd[0]); // close intput
			write(fd[1],argv[args],strlen(argv[args]));//fileName
			write(fd[1]," ",strlen(" ")); //fileName
			int printPairs;
			for(printPairs=0;printPairs<3;printPairs++){
				write(fd[1],vectorPairs[printPairs].word1,strlen(vectorPairs[printPairs].word1));//wordpair
				write(fd[1]," ",strlen(" ")); //wordpair
				write(fd[1],vectorPairs[printPairs].word2,strlen(vectorPairs[printPairs].word2));//wordpair
				write(fd[1]," ",strlen(" ")); //wordpair
				char floatToString[20];
				sprintf(floatToString,"%.6f",vectorPairs[printPairs].similarity);
				write(fd[1],floatToString,strlen(floatToString));//sim
				write(fd[1]," ",strlen(" ")); //sim
			}
			write(fd[1],"\n",strlen("\n")); //fileName
		}
		else
		{	/*parent process*/
			/* parent will wait for the child to complete*/
			wait(NULL);
			//printf("Child  %d Complete, processed %s\n",i,argv[i]);
			
			if(args==1){
				close(fd[1]); //close output to read input
				read(fd[0],printStatement,10000);
				printf("%s",printStatement);
			}	  
			return 0;                   
		}
	}
		
		
		
	return 0;		
}
