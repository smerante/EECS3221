//Family Name: Merante
//Given Name: Sam
//Student Number: 212795670
//CSE login: smerante

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "sch-helpers.h"  

process processes[MAX_PROCESSES+1]; //all processes read from file
process_queue incomingQ,readyQ;
process_node *headOfWaitingList;
int numberOfProcesses = 0;
float clockTime = 0;
CPU cpu0,cpu1,cpu2,cpu3;
struct CPU {
	float processingTime;
	process *p;
};
int compareByPID(const void *aa, const void *bb) {
    process *a = (process*) aa;
    process *b = (process*) bb;
    if (a->pid < b->pid) return -1;
    if (a->pid > b->pid) return 1;
    return 0;
}       
void makeQueue(process_queue *Q)
{
	//Q =(process_queue*)malloc(sizeof(process_queue)); //Queue of processes executed to run
	initializeProcessQueue(Q);
}
int notDoneProcessing()
{	
	return (incomingQ.size !=0 || readyQ.size !=0 || headOfWaitingList!=NULL
		   ||cpu0.p != NULL||cpu1.p != NULL||cpu2.p != NULL||cpu3.p != NULL);
}
//puts incoming processes into ready Q
void checkIncomingProcesses()
{
	while(incomingQ.size>0 && incomingQ.front->data->arrivalTime<=clockTime)
	{
		enqueueProcess(&readyQ,incomingQ.front->data);
		dequeueProcess(&incomingQ);
	}
		
	
}    

//checks to put waiting processes into readyQ
void checkWaitingProcesses()
{	
	process *moveToReadyProcesses[MAX_PROCESSES+1]; 
	process sortedReadyProcesses[MAX_PROCESSES+1];
	int processesToMove=0;
	process_node *p= headOfWaitingList;
	while(p){ //while the process in waitingList != NULL
		process* data = p->data;
		if(data->bursts[data->currentBurst].step >= data->bursts[data->currentBurst].length){
			//if the process is done its I/O BURST
			data->currentBurst++;
			moveToReadyProcesses[processesToMove]= p->data;
			sortedReadyProcesses[processesToMove] = *data;
			processesToMove++;
			//enqueueProcess(&readyQ,data); //put process back into readyQ
			//adjust the waitingList accordingly
			if(p == headOfWaitingList)
				headOfWaitingList = p->next;
			if(p->next)
				p->next->prev = p->prev;
			if(p->prev)
				p->prev->next = p->next;
		}
		
		p=p->next;
	}
	// if multiple processes are ready at the same time enter into readyQ by earliest PID
 	qsort(sortedReadyProcesses,processesToMove,sizeof(process),compareByPID); //sort all processes in terms of PID
   
	int i=0;
	for(i=0;i<processesToMove;i++)
		{
			int j = 0;
			for(j=0;j<processesToMove;j++){
				if(moveToReadyProcesses[j]->pid == sortedReadyProcesses[i].pid)
					enqueueProcess(&readyQ,moveToReadyProcesses[j]);
			}
		}
}

// put running processes from CPU to waitingQ or to done
void checkRunningProcesses()
{
	if(cpu0.p !=NULL)	//if CPU0 is currently Processing a task
	{	
		if(cpu0.p->bursts[cpu0.p->currentBurst].step >= cpu0.p->bursts[cpu0.p->currentBurst].length)
		{	//if the current CPU0 process is done processing 
			if(cpu0.p->currentBurst < cpu0.p->numberOfBursts - 2) 
			{//process has an I/O burst next
				cpu0.p->currentBurst++; //increase the currentBurst
				if(headOfWaitingList == NULL)
				{ 
					process_node *temp = createProcessNode(cpu0.p);
					headOfWaitingList = temp;
				}
				else
				{
					process_node *temp = createProcessNode(cpu0.p);
					headOfWaitingList->prev = temp;
					temp->next = headOfWaitingList;
					headOfWaitingList = temp;
				} //update waitingList
			}
			else cpu0.p-> endTime= clockTime;
			cpu0.p = NULL;	//CPU becomes free
		}
	}
	if(cpu1.p !=NULL)	//if cpu1 is currently Processing a task
	{	
		//printf("cpu1 Pros\n");
		if(cpu1.p->bursts[cpu1.p->currentBurst].step >= cpu1.p->bursts[cpu1.p->currentBurst].length)
		{	//if the current CPU0 process is done processing 
			if(cpu1.p->currentBurst < cpu1.p->numberOfBursts - 2) 
			{//process has an I/O burst next
				cpu1.p->currentBurst++; //increase the currentBurst
				if(headOfWaitingList == NULL)
				{
					process_node *temp = createProcessNode(cpu1.p);
					headOfWaitingList = temp;
				}
				else
				{
					process_node *temp = createProcessNode(cpu1.p);
					headOfWaitingList->prev = temp;
					temp->next = headOfWaitingList;
					headOfWaitingList = temp;
				}//update waitingList
			}
			else cpu1.p-> endTime = clockTime;
			cpu1.p = NULL;	//cpu1 becomes free
		}
	}
	if(cpu2.p !=NULL)	//if cpu2 is currently Processing a task
	{	
		//printf("cpu2 Pros\n");
		if(cpu2.p->bursts[cpu2.p->currentBurst].step >= cpu2.p->bursts[cpu2.p->currentBurst].length)
		{	//if the current CPU0 process is done processing 
			if(cpu2.p->currentBurst < cpu2.p->numberOfBursts - 2) 
			{//process has an I/O burst next
				cpu2.p->currentBurst++; //increase the currentBurst
				if(headOfWaitingList == NULL)
				{
					process_node *temp = createProcessNode(cpu2.p);
					headOfWaitingList = temp;
				}
				else
				{
					process_node *temp = createProcessNode(cpu2.p);
					headOfWaitingList->prev = temp;
					temp->next = headOfWaitingList;
					headOfWaitingList = temp;
				}//update waitingList
			}
			else cpu2.p->endTime = clockTime;
			cpu2.p = NULL;	//cpu2 becomes free
		}
	}
	if(cpu3.p !=NULL)	//if cpu3 is currently Processing a task
	{	
		//printf("cpu3 Pros\n");
		if(cpu3.p->bursts[cpu3.p->currentBurst].step >= cpu3.p->bursts[cpu3.p->currentBurst].length)
		{	//if the current CPU0 process is done processing 
			if(cpu3.p->currentBurst < cpu3.p->numberOfBursts - 2) 
			{//process has an I/O burst next
				cpu3.p->currentBurst++; //increase the currentBurst
				if(headOfWaitingList == NULL)
				{
					process_node *temp = createProcessNode(cpu3.p);
					headOfWaitingList = temp;
				}
				else
				{
					process_node *temp = createProcessNode(cpu3.p);
					headOfWaitingList->prev = temp;
					temp->next = headOfWaitingList;
					headOfWaitingList = temp;
				}//update waitingList
			}
			else cpu3.p->endTime = clockTime;
			cpu3.p = NULL;	//cpu3 becomes free
		}
	}
}

//sets readyQ processes to CPUs
void checkReadyProcesses()
{
	while((cpu0.p == NULL || cpu1.p == NULL || cpu2.p == NULL || cpu3.p == NULL)
		 && readyQ.size > 0) //loop while process is ready and theres free cpu
	{
		//printf("ready Pros\n");
		if(cpu0.p == NULL) //first CPU is free
		{
			cpu0.p = readyQ.front->data;
			if(cpu0.p->startTime == -1) cpu0.p->startTime = clockTime;
			dequeueProcess(&readyQ);
		}
		else if(cpu1.p == NULL) //Second CPU is free
		{
			cpu1.p = readyQ.front->data;
			if(cpu1.p->startTime == -1) cpu1.p->startTime = clockTime;
			dequeueProcess(&readyQ);
		}
		else if(cpu2.p == NULL) //third CPU is free
		{
			cpu2.p = readyQ.front->data;
			if(cpu2.p->startTime == -1) cpu2.p->startTime = clockTime;
			dequeueProcess(&readyQ);
		}
		else if(cpu3.p == NULL) //fourth CPU is free
		{
			cpu3.p = readyQ.front->data;
			if(cpu3.p->startTime == -1) cpu3.p->startTime = clockTime;
			dequeueProcess(&readyQ);
		}

	}
	
	
}

//update processes waiting time while in readyQ
void updateReadyQ()
{
	if(readyQ.front != NULL)
	{
		process_node *p = readyQ.front;
		while(p->data != NULL)
		{
			p->data->waitingTime++;	//increase waiting Time for processes in ready Queue
			if(p->next == NULL)break;
			p = p->next;
		}
	}
}
//update I/O Burst time when in waiting Q
void updateWaitingQ()
{
	if(headOfWaitingList){
	process_node *temp = headOfWaitingList;
	temp->data->bursts[temp->data->currentBurst].step++;
		while(temp->next){
			temp = temp->next;
			temp->data->bursts[temp->data->currentBurst].step++;
			}
	}
}
//update CPU Burst time while processes are on CPUs
void updateCPUs()
{
	if(cpu0.p != NULL) cpu0.p->bursts[cpu0.p->currentBurst].step++; //incease the current step of processes on the cpu0
	if(cpu1.p != NULL) cpu1.p->bursts[cpu1.p->currentBurst].step++; //incease the current step of processes on the cpu1
	if(cpu2.p != NULL) cpu2.p->bursts[cpu2.p->currentBurst].step++; //incease the current step of processes on the cpu2
	if(cpu3.p != NULL) cpu3.p->bursts[cpu3.p->currentBurst].step++; //incease the current step of processes on the cpu3

	if(cpu0.p != NULL) cpu0.processingTime++;
	if(cpu1.p != NULL) cpu1.processingTime++;
	if(cpu2.p != NULL) cpu2.processingTime++;
	if(cpu3.p != NULL) cpu3.processingTime++;
}
//updates all processes time in the system and the clock Time
void updatePC()
{
	updateReadyQ();
	updateWaitingQ();
	updateCPUs();
	clockTime++;
}
int main(int argc, char* argv[])
{

	int status =0;
	while ((status=readProcess(&processes[numberOfProcesses]))){  
         	if(status==1) {
			processes[numberOfProcesses].startTime = -1; // to indicate a process that hasnt started
			numberOfProcesses ++; //read in all processes trying to run
		}
	} 

     qsort(processes,numberOfProcesses,sizeof(process),compareByArrival); //sort all processes in terms of running Time
   
	makeQueue(&incomingQ);	//process first executed before goes to waiting or ready
	makeQueue(&readyQ); // queue for processes that have arrived
	
	int i = 0;
     for(i=0;i<numberOfProcesses;i++)
     	enqueueProcess(&incomingQ,&processes[i]); //put all processes into startingQ
			
	while(notDoneProcessing()) //While there are still processes in any Queue
	{
		checkIncomingProcesses(); //Incoming Procs -> ReadyQ
		checkRunningProcesses(); //Running Procs -> WaitingQ Or Done
		checkWaitingProcesses();	//Waiting Procs -> ReadyQ
		checkReadyProcesses();	//Ready Procs -> CPUs
		updatePC();
	}
	
	int avrgWaitingTime = 0,turnAround =0;
	float avgCpuUtil=0;
	for(i=0;i<numberOfProcesses;i++)
	{
		avrgWaitingTime+=processes[i].waitingTime;
		turnAround += (processes[i].endTime - processes[i].arrivalTime);
	}
		/*printf("cpu0 running time: %f\n",cpu0.processingTime);
		printf("cpu1 running time: %f\n",cpu1.processingTime);
		printf("cpu2 running time: %f\n",cpu2.processingTime);
		printf("cpu3 running time: %f\n",cpu3.processingTime);
		printf("Clock Time: %f\n",clockTime-1);
		printf("--------------------------------------\n");*/
		avrgWaitingTime = avrgWaitingTime/numberOfProcesses;
		turnAround = turnAround/numberOfProcesses;
		avgCpuUtil = cpu0.processingTime/(clockTime-1) + 
				   cpu1.processingTime/(clockTime-1) +
				   cpu2.processingTime/(clockTime-1) +
				   cpu3.processingTime/(clockTime-1);
		avgCpuUtil*=100;
		printf("Average waiting time                 : %d.00 units\n",avrgWaitingTime);
		printf("Average turnaround time              : %d.00 units\n",turnAround);
		printf("Time all processes finished          : %.0f\n",clockTime -1);
		printf("Average CPU utilization              : %.1f %%\n",avgCpuUtil);
		printf("Number of context switches           : 0\n");
		printf("PID(s) of last process(es) to finish :");
		for(i=0;i<numberOfProcesses;i++)
		{
			if(processes[i].endTime == (clockTime-1))
				printf(" %d",processes[i].pid);
		}
		printf("\n");
	return 0;
}	
