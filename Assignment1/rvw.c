#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
struct VectorPair{
	char word1[100],word2[100];
	float similarity;
};

// Family Name: Merante
// Given Name: Samuel
// Section: Z
// Student Number: 212795670
// CSE Login: smerante


//global variables too big to fit on cache stack
struct VectorPair vectorPairs[1000000]; //object for each pair of words
int numVectors = 0,numPairs = 0; 
int i=0,counter = 0; // i -> words, counter -> vectors
int j,k; //for computing all possible pairs of cosine sims
int args; //for command line args
char string[100000][100],tempString[100];	//lots of possible words
float vectors[100000][100],temp = 0; //lost of possible vectors
	
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

int main(int argc, char *argv[])
{
	for(args=1;args<argc;args++)
	{// file for loop
		i=0,counter = 0;
		FILE *file;
		file = fopen(argv[args],"r");
		
		
		char tempString[10000],word[100];
		int tempBuffer = 0,charsRead = 0,scanning = 1;
		float f = 0;
		
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
		
		numPairs = 0;
		for(j=0;j<=i-2;j++)
		{
			for(k=j+1;k<=i-1;k++){
			strcpy(vectorPairs[numPairs].word1,string[j]);
			strcpy(vectorPairs[numPairs].word2,string[k]);
			vectorPairs[numPairs].similarity = cosineSimilarity(vectors[j],vectors[k]);
			numPairs++;
			}
		}
	
	   qsort(vectorPairs, numPairs, sizeof(struct VectorPair), cmpSimfunc);
	   printf("%s %s %s %.6f " ,argv[args],vectorPairs[0].word1 ,vectorPairs[0].word2,vectorPairs[0].similarity); 
	   printf("%s %s %.6f " ,vectorPairs[1].word1 ,vectorPairs[1].word2,vectorPairs[1].similarity); 
	   printf("%s %s %.6f\n" ,vectorPairs[2].word1 ,vectorPairs[2].word2,vectorPairs[2].similarity); 

	}//end of file for loop         
	return 0;
}
