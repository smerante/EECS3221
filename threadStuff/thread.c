#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
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
int fileNum = 1;	//global variable for the file the current thread is on

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

//func for pthreads
void *findSim(void *param) 
{
	int i=0,counter = 0; // i -> words, counter -> vectors
	int currentFile = fileNum; // set current threads counter to the fileNum
	fileNum+=1;		// increase the fileNum
	
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
		strcpy(vectorPairs[currentFile][numPairs].word1,string[j]); //set vectorPairs at currentFile Num
		strcpy(vectorPairs[currentFile][numPairs].word2,string[k]);
		vectorPairs[currentFile][numPairs].similarity = cosineSimilarity(vectors[j],vectors[k],numVectors);
		numPairs++;
		}
	}
	
	qsort(vectorPairs[currentFile], numPairs, sizeof(struct VectorPair), cmpSimfunc);
	pthread_exit(0);
}
int main(int argc, char *argv[])
{
	pthread_t tid; //thread id
	pthread_attr_t attr; //attributes for thread
	pthread_attr_init(&attr); // attributes for this current thread
	int args; //for command line args
	
	for(args=1;args<argc;args++)
	{// file for loop
		pthread_create(&tid,&attr,findSim,argv[args]);
	}//end of file for loop   
	pthread_join(tid,NULL); // wait for all children to finish 
	
	for(args=1;args<argc;args++)
	{	
	  printf("%s %s %s %.6f " ,argv[args],vectorPairs[args][0].word1 ,vectorPairs[args][0].word2,vectorPairs[args][0].similarity); 
	  printf("%s %s %.6f " ,vectorPairs[args][1].word1 ,vectorPairs[args][1].word2,vectorPairs[args][1].similarity); 
	  printf("%s %s %.6f\n" ,vectorPairs[args][2].word1 ,vectorPairs[args][2].word2,vectorPairs[args][2].similarity); 
	}
	
	return 0;
}
