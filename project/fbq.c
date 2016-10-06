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
process_queue incomingQ,Q0,Q1,FCFS;
process_node *headOfWaitingList;
int numberOfProcesses = 0; int	timeQuantum1,timeQuantum2; // for RR
float clockTime = 0;
CPU cpu0,cpu1,cpu2,cpu3;
struct CPU {
	float processingTime;
	int contextSwitches;
	process *p;
};
void enqueueWaitingQ(process *p)
{
	if(headOfWaitingList == NULL)
	{ 
		process_node *temp = createProcessNode(p);
		headOfWaitingList = temp;
	}
	else
	{
		process_node *temp = createProcessNode(p);
		headOfWaitingList->prev = temp;
		temp->next = headOfWaitingList;
		headOfWaitingList = temp;
	} //update waitingList

}
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
	return (incomingQ.size !=0 || FCFS.size !=0 || Q0.size!=0 || Q1.size!=0 || headOfWaitingList!=NULL
		   ||cpu0.p != NULL||cpu1.p != NULL||cpu2.p != NULL||cpu3.p != NULL);
}
//puts incoming processes into ready Q
void checkIncomingProcesses()
{
	while(incomingQ.size>0 && incomingQ.front->data->arrivalTime<=clockTime)
	{
		incomingQ.front->data->currentQueue = 0;
		enqueueProcess(&Q0,incomingQ.front->data);
		dequeueProcess(&incomingQ);
	}
		
	
}    

//checks to put waiting processes into there Q
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
			//put process back into its Q
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
	// if multiple processes are ready at the same time enter into FCFS by earliest PID
 	qsort(sortedReadyProcesses,processesToMove,sizeof(process),compareByPID); //sort all processes in terms of PID
   
	int i=0;
	for(i=0;i<processesToMove;i++)
		{
			int j = 0;
			for(j=0;j<processesToMove;j++){
				if(moveToReadyProcesses[j]->pid == sortedReadyProcesses[i].pid)
				{
					if(moveToReadyProcesses[j]->currentQueue ==0) //move into readyQ0
						enqueueProcess(&Q0,moveToReadyProcesses[j]);
					else if(moveToReadyProcesses[j]->currentQueue ==1)//move into readyQ1
						enqueueProcess(&Q1,moveToReadyProcesses[j]);
					else	//move into FCFS
						enqueueProcess(&FCFS,moveToReadyProcesses[j]);
				}
			}
		}
}

// put running processes from CPU to waitingQ or to done or to ready
void checkRunningProcesses()
{
	if(cpu0.p !=NULL)	//if CPU0 is currently Processing a task
	{	
		
		if(cpu0.p->bursts[cpu0.p->currentBurst].step >= cpu0.p->bursts[cpu0.p->currentBurst].length)
		{	//if the current CPU0 process is done processing 
			if(cpu0.p->currentBurst < cpu0.p->numberOfBursts - 2) 
			{//process has an I/O burst next
				cpu0.p->currentBurst++; //increase the currentBurst

				if(cpu0.p->currentQueue == 0)
					cpu0.p->quantumRemaining = timeQuantum1; 
				else if(cpu0.p->currentQueue == 1){//reset time slice for FBQ
					cpu0.p->quantumRemaining = timeQuantum2;//promote to higher Q
					//cpu0.p->currentQueue = 0;
				}
				else
				{
					//cpu0.p->quantumRemaining = timeQuantum2;//promote to higher Q
					//cpu0.p->currentQueue = 1;
				}
				enqueueWaitingQ(cpu0.p);
			}
			else cpu0.p-> endTime= clockTime;
			cpu0.p = NULL;	//CPU becomes free
		}
		else
		{ //process is not done its burst
			if(cpu0.p->currentQueue == 0 && cpu0.p->quantumRemaining <= 0)
			{// time slice expired for process in Q0
				cpu0.p->quantumRemaining = timeQuantum2; //reset timeSlice FBQ to Q1
				cpu0.p->currentQueue = 1;
				cpu0.contextSwitches++; // context switch happens
				enqueueProcess(&Q1,cpu0.p);//preempt process into next Queue
				cpu0.p = NULL;	//CPU becomes free
			}
			else if(cpu0.p->currentQueue == 1 && cpu0.p->quantumRemaining <= 0)
			{// time slice expired for process in Q1
				cpu0.p->currentQueue = 2;
				cpu0.contextSwitches++; // context switch happens
				enqueueProcess(&FCFS,cpu0.p);//preempt process into next Queue
				cpu0.p = NULL;	//CPU becomes free
			}
			else if(Q0.size > 0 && cpu0.p->currentQueue != 0)
			{//More important Process is waiting in higher Queue
				if(cpu0.p->currentQueue ==1)//move into readyQ1
					enqueueProcess(&Q1,cpu0.p);
				else
					enqueueProcess(&FCFS,cpu0.p);
				cpu0.contextSwitches++; // context switch happens
				cpu0.p = Q0.front->data;//switch cpu to more important process
				dequeueProcess(&Q0);
			}
		}
	}
	if(cpu1.p !=NULL)	
	{	//if cpu1 is currently Processing a task
		
		if(cpu1.p->bursts[cpu1.p->currentBurst].step >= cpu1.p->bursts[cpu1.p->currentBurst].length)
		{	//if the current CPU1 process is done processing 
			if(cpu1.p->currentBurst < cpu1.p->numberOfBursts - 2) 
			{//process has an I/O burst next
				cpu1.p->currentBurst++; //increase the currentBurst

				if(cpu1.p->currentQueue == 0)
					cpu1.p->quantumRemaining = timeQuantum1; 
				else if(cpu1.p->currentQueue == 1){//reset time slice for FBQ
					cpu1.p->quantumRemaining = timeQuantum2;//promote to higher Q
					//cpu1.p->currentQueue = 0;
				}
				else
				{
					//cpu1.p->quantumRemaining = timeQuantum2;//promote to higher Q
					//cpu1.p->currentQueue = 1;
				}
				enqueueWaitingQ(cpu1.p);
			}
			else cpu1.p-> endTime = clockTime;
			cpu1.p = NULL;	//cpu1 becomes free
		}
		else 
		{ 
			// time slice expired for process in Q0
			if(cpu1.p->currentQueue == 0 && cpu1.p->quantumRemaining <= 0)
			{
				cpu1.p->quantumRemaining = timeQuantum2; //reset timeSlice FBQ to Q1
				cpu1.p->currentQueue = 1;
				cpu1.contextSwitches++; // context switch happens
				enqueueProcess(&Q1,cpu1.p);//preempt process into next Queue
				cpu1.p = NULL;	//CPU becomes free
			}
			else if(cpu1.p->currentQueue == 1 && cpu1.p->quantumRemaining <= 0)
			{// time slice expired for process in Q1
				cpu1.p->currentQueue = 2;
				cpu1.contextSwitches++; // context switch happens
				enqueueProcess(&FCFS,cpu1.p);//preempt process into next Queue
				cpu1.p = NULL;	//CPU becomes free
			} 
			else if(Q0.size > 0 && cpu1.p->currentQueue != 0)
			{//More important Process is waiting in higher Queue
				if(cpu1.p->currentQueue ==1)//move into readyQ1
					enqueueProcess(&Q1,cpu1.p);
				else
					enqueueProcess(&FCFS,cpu1.p);
				cpu1.contextSwitches++; // context switch happens
				cpu1.p = Q0.front->data;//switch cpu to more important process
				dequeueProcess(&Q0);
			}
		}
	}
	if(cpu2.p !=NULL)	
	{	//if cpu2 is currently Processing a task
		
		if(cpu2.p->bursts[cpu2.p->currentBurst].step >= cpu2.p->bursts[cpu2.p->currentBurst].length)
		{	//if the current CPU0 process is done processing 
			if(cpu2.p->currentBurst < cpu2.p->numberOfBursts - 2) 
			{//process has an I/O burst next
				if(cpu2.p->currentQueue == 0)
					cpu2.p->quantumRemaining = timeQuantum1; 
				else if(cpu2.p->currentQueue == 1){//reset time slice for FBQ
					cpu2.p->quantumRemaining = timeQuantum2; //promote to higherQ
					//cpu2.p->currentQueue = 0;
				}
				else{
					//cpu2.p->quantumRemaining = timeQuantum2;//promote to higherQ
					//cpu2.p->currentQueue = 1;
				}

				cpu2.p->currentBurst++; //increase the currentBurst
				enqueueWaitingQ(cpu2.p);
			}
			else cpu2.p->endTime = clockTime;
			cpu2.p = NULL;	//cpu2 becomes free
		}
		else 
		{ 
			// time slice expired for process in Q0
			if(cpu2.p->currentQueue == 0 && cpu2.p->quantumRemaining <= 0)
			{
				cpu2.p->quantumRemaining = timeQuantum2; //reset timeSlice FBQ to Q1
				cpu2.p->currentQueue = 1;
				cpu2.contextSwitches++; // context switch happens
				enqueueProcess(&Q1,cpu2.p);//preempt process into next Queue
				cpu1.p = NULL;	//CPU becomes free
			}
			else if(cpu2.p->currentQueue == 1 && cpu2.p->quantumRemaining <= 0)
			{// time slice expired for process in Q1
				cpu2.p->currentQueue = 2;
				cpu2.contextSwitches++; // context switch happens
				enqueueProcess(&FCFS,cpu2.p);//preempt process into next Queue
				cpu2.p = NULL;	//CPU becomes free
			} 
			else if(Q0.size > 0 && cpu2.p->currentQueue != 0)
			{//More important Process is waiting in higher Queue
				if(cpu2.p->currentQueue ==1)//move into readyQ1
					enqueueProcess(&Q1,cpu2.p);
				else
					enqueueProcess(&FCFS,cpu2.p);
				cpu2.contextSwitches++; // context switch happens
				cpu2.p = Q0.front->data; //switch cpu to more important process
				dequeueProcess(&Q0);
			}
		}
	}
	if(cpu3.p !=NULL)	//if cpu3 is currently Processing a task
	{	
		
		if(cpu3.p->bursts[cpu3.p->currentBurst].step >= cpu3.p->bursts[cpu3.p->currentBurst].length)
		{	//if the current CPU0 process is done processing 
			if(cpu3.p->currentBurst < cpu3.p->numberOfBursts - 2) 
			{//process has an I/O burst next
				if(cpu3.p->currentQueue == 0)
					cpu3.p->quantumRemaining = timeQuantum1; 
				else if(cpu3.p->currentQueue == 1){//reset time slice for FBQ
					cpu3.p->quantumRemaining = timeQuantum2;
					//cpu3.p->currentQueue = 0;
				}
				else
				{
					//cpu3.p->quantumRemaining = timeQuantum2;
					//cpu3.p->currentQueue = 1;
				}

				cpu3.p->currentBurst++; //increase the currentBurst
				enqueueWaitingQ(cpu3.p);
			}
			else cpu3.p->endTime = clockTime;
			cpu3.p = NULL;	//cpu3 becomes free
		}
		else 
		{ 
			// time slice expired for process in Q0
			if(cpu3.p->currentQueue == 0 && cpu3.p->quantumRemaining <= 0)
			{
				cpu3.p->quantumRemaining = timeQuantum2; //reset timeSlice FBQ to Q1
				cpu3.p->currentQueue = 1;
				cpu3.contextSwitches++; // context switch happens
				enqueueProcess(&Q1,cpu3.p);//preempt process into next Queue
				cpu3.p = NULL;	//CPU becomes free
			}
			else if(cpu3.p->currentQueue == 1 && cpu3.p->quantumRemaining <= 0)
			{// time slice expired for process in Q1
				cpu3.p->currentQueue = 2;
				cpu3.contextSwitches++; // context switch happens
				enqueueProcess(&FCFS,cpu3.p);//preempt process into next Queue
				cpu3.p = NULL;	//CPU becomes free
			}
			else if(Q0.size > 0 && cpu3.p->currentQueue != 0)
			{//More important Process is waiting in higher Queue
				if(cpu3.p->currentQueue ==1)//move into readyQ1
					enqueueProcess(&Q1,cpu3.p);
				else
					enqueueProcess(&FCFS,cpu3.p);
				cpu3.contextSwitches++; // context switch happens
				cpu3.p = Q0.front->data;//switch cpu to more important process
				dequeueProcess(&Q0);
			}
		}
	}
}

//sets Ready processes to CPUs by order of Priority
void checkReadyProcesses()
{
	while((cpu0.p == NULL || cpu1.p == NULL || cpu2.p == NULL || cpu3.p == NULL)
		 && (Q0.size > 0 || Q1.size > 0 || FCFS.size > 0)) //while process is ready and theres free cpu
	{
		if(cpu0.p == NULL) //first CPU is free
		{
			if(Q0.size >0){ //process waiting in Q0
				cpu0.p = Q0.front->data;
				if(cpu0.p->startTime == -1) cpu0.p->startTime = clockTime;
				dequeueProcess(&Q0);
			}
			else if(Q1.size > 0)//process waiting in Q1
			{
				cpu0.p = Q1.front->data;
				if(cpu0.p->startTime == -1) cpu0.p->startTime = clockTime;
				dequeueProcess(&Q1);
			}
			else if(FCFS.size > 0)//process waiing in FCFS
			{
				cpu0.p = FCFS.front->data;
				if(cpu0.p->startTime == -1) cpu0.p->startTime = clockTime;
				dequeueProcess(&FCFS);
			}
		}
		else if(cpu1.p == NULL) //Second CPU is free
		{
			if(Q0.size >0){ //process waiting in Q0
				cpu1.p = Q0.front->data;
				if(cpu1.p->startTime == -1) cpu1.p->startTime = clockTime;
				dequeueProcess(&Q0);
			}
			else if(Q1.size > 0)//process waiting in Q1
			{
				cpu1.p = Q1.front->data;
				if(cpu1.p->startTime == -1) cpu1.p->startTime = clockTime;
				dequeueProcess(&Q1);
			}
			else if(FCFS.size > 0)//process waiing in FCFS
			{
				cpu1.p = FCFS.front->data;
				if(cpu1.p->startTime == -1) cpu1.p->startTime = clockTime;
				dequeueProcess(&FCFS);
			}
		}
		else if(cpu2.p == NULL) //third CPU is free
		{
			if(Q0.size >0){ //process waiting in Q0
				cpu2.p = Q0.front->data;
				if(cpu2.p->startTime == -1) cpu2.p->startTime = clockTime;
				dequeueProcess(&Q0);
			}
			else if(Q1.size > 0)//process waiting in Q1
			{
				cpu2.p = Q1.front->data;
				if(cpu2.p->startTime == -1) cpu2.p->startTime = clockTime;
				dequeueProcess(&Q1);
			}
			else if(FCFS.size > 0)//process waiing in FCFS
			{
				cpu2.p = FCFS.front->data;
				if(cpu2.p->startTime == -1) cpu2.p->startTime = clockTime;
				dequeueProcess(&FCFS);
			}
		}
		else if(cpu3.p == NULL) //fourth CPU is free
		{
			if(Q0.size >0){ //process waiting in Q0
				cpu3.p = Q0.front->data;
				if(cpu3.p->startTime == -1) cpu3.p->startTime = clockTime;
				dequeueProcess(&Q0);
			}
			else if(Q1.size > 0)//process waiting in Q1
			{
				cpu3.p = Q1.front->data;
				if(cpu3.p->startTime == -1) cpu3.p->startTime = clockTime;
				dequeueProcess(&Q1);
			}
			else if(FCFS.size > 0)//process waiing in FCFS
			{
				cpu3.p = FCFS.front->data;
				if(cpu3.p->startTime == -1) cpu3.p->startTime = clockTime;
				dequeueProcess(&FCFS);
			}
		}

	}
	
	
}

//update processes waiting time while in ReadyQ
void updateReadyQ()
{
	
	if(Q0.front != NULL)
	{
		process_node *p = Q0.front;
		while(p->data != NULL)
		{
			p->data->waitingTime++;	//increase waiting Time for processes in ready Queue
			if(p->next == NULL)break;
			p = p->next;
		}
	}
	if(Q1.front != NULL)
	{
		process_node *p = Q1.front;
		while(p->data != NULL)
		{
			p->data->waitingTime++;	//increase waiting Time for processes in ready Queue
			if(p->next == NULL)break;
			p = p->next;
		}
	}
	if(FCFS.front != NULL)
	{
		process_node *p = FCFS.front;
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

	 //For RR Time Quantam Expire
	if(cpu0.p != NULL) cpu0.p->quantumRemaining--;
	if(cpu1.p != NULL) cpu1.p->quantumRemaining--;
	if(cpu2.p != NULL) cpu2.p->quantumRemaining--;
	if(cpu3.p != NULL) cpu3.p->quantumRemaining--;
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
	timeQuantum1 = atoi(argv[1]);
	timeQuantum2 = atoi(argv[2]);
	int status =0;
	while ((status=readProcess(&processes[numberOfProcesses]))){  
         	if(status==1) {
			processes[numberOfProcesses].startTime = -1; // to indicate a process that hasnt started
			processes[numberOfProcesses].quantumRemaining = timeQuantum1; // for FBQ
			numberOfProcesses ++; //read in all processes trying to run
		}
	} 

     qsort(processes,numberOfProcesses,sizeof(process),compareByArrival); //sort all processes in terms of running Time
   
	makeQueue(&incomingQ);	//process first executed before goes to waiting or ready
	makeQueue(&Q0); // queue for processes that have arrived
	makeQueue(&Q1); // queue for processes that have preempted from Q0
	makeQueue(&FCFS); // lowest level queue FCFS
	
	int i = 0;
     for(i=0;i<numberOfProcesses;i++)
     	enqueueProcess(&incomingQ,&processes[i]); //put all processes into startingQ
			
	while(notDoneProcessing()) //While there are still processes in any Queue
	{
		checkIncomingProcesses(); //Incoming Procs -> ReadyQ
		checkRunningProcesses(); //Running Procs -> WaitingQ Or Done Or Ready
		checkWaitingProcesses(); //Waiting Procs -> ReadyQ
		checkReadyProcesses();	//Ready Procs -> CPUs
		updatePC();
	}
	
	int avrgWaitingTime = 0,contextSwitches=0,turnAround =0;
	float avgCpuUtil=0;
	
	
	for(i=0;i<numberOfProcesses;i++)
	{
		avrgWaitingTime+=processes[i].waitingTime;
		turnAround += (processes[i].endTime - processes[i].arrivalTime);
	}

		avrgWaitingTime = avrgWaitingTime/numberOfProcesses;
		turnAround = turnAround/numberOfProcesses;
		contextSwitches = cpu0.contextSwitches + cpu1.contextSwitches + cpu2.contextSwitches + cpu3.contextSwitches;
		avgCpuUtil = cpu0.processingTime/(clockTime-1) + 
				   cpu1.processingTime/(clockTime-1) +
				   cpu2.processingTime/(clockTime-1) +
				   cpu3.processingTime/(clockTime-1);
		avgCpuUtil*=100;
		
		printf("Average waiting time                 : %d.00 units\n",avrgWaitingTime);
		printf("Average turnaround time              : %d.00 units\n",turnAround);
		printf("Time all processes finished          : %.0f\n",clockTime -1);
		printf("Average CPU utilization              : %.1f %%\n",avgCpuUtil);
		printf("Number of context switches           : %d\n",contextSwitches);
		printf("PID(s) of last process(es) to finish :");
		for(i=0;i<numberOfProcesses;i++)
		{
			if(processes[i].endTime == (clockTime-1))
				printf(" %d",processes[i].pid);
		}
		printf("\n");
	return 0;
}	
