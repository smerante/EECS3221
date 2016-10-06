#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <wait.h>

struct VectorPair{
	char word1[100],word2[100];
	float similarity;
};

// Family Name: Merante
// Given Name: Samuel
// Section: Z
// Student Number: 212795670
// CSE Login: smerante


//global variables for data sharing
struct VectorPair vectorPairs[100][1000000]; //object for each pair of words

int cmpSimfunc (const void * a, const void * b)
{
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

float dotProductFunc(float vector1[],float vector2[],int numVectors){
	int i = 0;
	float product = 0;
	for(i=0;i<numVectors;i++){
		product +=  vector1[i]*vector2[i];
	}
	return product;
}
float powerOf2(float vector1[],int numVectors){
	return dotProductFunc(vector1,vector1,numVectors);
}

float cosineSimilarity(float vector1[],float vector2[],int numVectors){
	float magnitiude = 0,dotProduct = 0;
	dotProduct = dotProductFunc(vector1,vector2,numVectors);
	magnitiude  = sqrt(powerOf2(vector1,numVectors))*sqrt(powerOf2(vector2,numVectors));
	return dotProduct/magnitiude;
}
int args; //for command line args
int pid, fd[100][2]; // for up to 100 childprocceses
char *global_argv[100];

//func finding similarities given a file
void *findSim(void *param) 
{
	int i=0,counter = 0; // i -> words, counter -> vectors
	
	float **vectors = (float**)malloc(sizeof(long int)*100000);; //lots of possible vectors
	char **string = (char**)malloc(sizeof(char)*100000);	// lots of possible words
	int makeArray;
	for(makeArray = 0; makeArray< 100000;makeArray++){
		string[makeArray] = (char*)malloc(sizeof(char)*200); //each word can be upto 200 chars long
		vectors[makeArray] = (float*)malloc(sizeof(long int)*200); //each vector can be upto 200 floats long
	}
	
	FILE *file;
	file = fopen(param,"r");
	char tempString[10000],word[100]; // line & word for a file
	int tempBuffer = 0,charsRead = 0,scanning = 1;//buffer for liine, charsRead to increase buffer, scanning until line is done
	float f = 0; // temp float for reading vectors from file
	int numVectors = 0; //number of vectors from each line
	while(!feof(file)){
		if(fgets(tempString,10000,file) != NULL){
			scanning = 1;
			tempBuffer=0;
			while(scanning){
				if(sscanf(tempString+tempBuffer,"%s%n",word,&charsRead)==1){
					 tempBuffer+=charsRead;	
					if(sscanf(word,"%f",&f)){
						vectors[i-1][counter] = f;
						counter++;
					}else{
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
		
	int numPairs = 0;	//for qSort to work properly 
	int j,k; //for computing all possible pairs of cosine sims                  
	for(j=0;j<=i-2;j++)
	{
		for(k=j+1;k<=i-1;k++){
		strcpy(vectorPairs[args][numPairs].word1,string[j]); //set vectorPairs at currentFile Num
		strcpy(vectorPairs[args][numPairs].word2,string[k]);
		vectorPairs[args][numPairs].similarity = cosineSimilarity(vectors[j],vectors[k],numVectors);
		numPairs++;
		}
	}
	
	qsort(vectorPairs[args], numPairs, sizeof(struct VectorPair), cmpSimfunc);
	//printf("done child process for args %d\n",args);
	close(fd[args][0]); // close input to write to the filed
	write(fd[args][1],global_argv[args],strlen(global_argv[args]));//fileName
	write(fd[args][1]," ",strlen(" ")); //fileName
	int printPairs;
	for(printPairs=0;printPairs<3;printPairs++){
		write(fd[args][1],vectorPairs[args][printPairs].word1,strlen(vectorPairs[args][printPairs].word1));//wordpair
		write(fd[args][1]," ",strlen(" ")); //wordpair
		write(fd[args][1],vectorPairs[args][printPairs].word2,strlen(vectorPairs[args][printPairs].word2));//wordpair
		write(fd[args][1]," ",strlen(" ")); //wordpair
		char floatToString[20];
		sprintf(floatToString,"%.6f",vectorPairs[args][printPairs].similarity);
		write(fd[args][1],floatToString,strlen(floatToString));//sim
		write(fd[args][1]," ",strlen(" ")); //sim
	}
	write(fd[args][1],"\n",strlen("\n")); //nextline
	
	exit(0);
}
int main(int argc, char *argv[])
{
	
	for(args=1;args<(argc);args++)
	{// file for loop
		if(pipe(fd[args]) < 0 ){printf("Pipe Failed \n");}
		global_argv[args] = argv[args];
	}
	for(args=1;args<argc;args++)
	{// file for loop
		pid = fork();
		if(pid < 0){printf("Fork Failed \n");}
		else if(pid == 0){ /*child process*/
			findSim(argv[args]);
		}
		
	}//end of file for loop   
	
	for(args=1;args<argc;args++){
		wait(NULL); /*parent will wait for child to complete*/
	}
	for(args=1;args<argc;args++){
		char printStatement[10000];
		close(fd[args][1]); //close output to read input
		read(fd[args][0],printStatement,10000);
		printf("%s",printStatement);
	}
	return 0;
	
}
