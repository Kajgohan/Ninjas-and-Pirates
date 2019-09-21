#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>

sem_t n;
sem_t p;
sem_t s;
sem_t c;
sem_t b;
sem_t q;
sem_t x;
sem_t printer;

int batch = 0;//served customers since last type switch
int waitCount = 0;//the amount of currently waiting customers
int served = 0;//the total amount of served customers
int inStore = 0;//customers currently in the store
int threshold = 4;//how many of each type to serve at a time
int ninjaCount = 0;//amt of ninjas before getting into the waiting room
int pirateCount = 0;//amt of pirates before getting into the waiting room
int arivedCustomers = 0;//used to do a full reset, opening both doors
int numTeams;//user entered number of staff teams
int threadStartTotal = 0;//total number of threads that ran
long totalTime = 0;//total time of the day




typedef struct{
	int goldPaid;//gold paid on a given visit
	int totalGold;//all of the gold paid over the day
	float costumeTime;//how long they need to take in the store/how much they must pay
	float arrivalTime;//how long they sleeo before trying the semaphores
	float waitTime;//how long the thread waited after trying to get into the store
	int comeBack;//odds they come back that day
	int hasReturned;//flag for if the thread came back
	float totalTimeOfVisit;//how much total time 
	int id; //sequential id of gernerated threads
	char type; //if they are a pirate ora  ninja
	int teamUsed; //the team that the thread gets served by
}customerStats;

typedef struct{
	int workerId;
	float idleTime;
	float busyTime;

}workerStats;

typedef struct{
	float avgQueLength;
	float totalWaitTime;
	float avgGoldPerVisit;
	int grossRevenue;
	int totalProfits;

}ledger;

//declares the ledger and all of the possible workers
ledger mainLedger;
workerStats minion1;
workerStats minion2;
workerStats minion3;
workerStats minion4;

void *runStore(void *arg){

	//protected counter of all the threads that run during the day
	sem_wait(&b);
	threadStartTotal+=1;
	sem_post(&b);

	struct timeval start;
	struct timeval end;
	struct timeval timeServed;

	//initializes customerstats struct for the current thread
	customerStats *stats =  (customerStats*)arg;
	//simulates arrival time
	sleep(stats->arrivalTime);
	//gets start time of each thread once it arrives at the store
	gettimeofday(&start, NULL);

	//this if else increments counters, and checks if it is the first thread, or on a total reset. If so it closes the other types door
	if(stats->type == 'n'){
		printf("A ninja #%d has arrived!\n", stats->id);
		sem_wait(&b);
		if(arivedCustomers == 0){
			sem_wait(&p);
		}
		ninjaCount+=1;
		arivedCustomers +=1;
		sem_post(&b);
	}
	else if (stats->type == 'p'){

		printf("A pirate #%d has arrived!\n", stats->id);
		sem_wait(&b);
		if(arivedCustomers == 0){
			sem_wait(&n);
		}
		pirateCount+=1;
		arivedCustomers +=1;
		sem_post(&b);
	}



if (stats->type == 'n'){
	//this is to get into thte waiting room
	sem_wait(&q);
	sem_wait(&n);
	sem_wait(&b);
	sem_post(&n);
	printf("A ninja #%d has entered the waiting room!\n", stats->id);
	waitCount +=1;
	ninjaCount -=1;
	batch +=1;
	if(batch == threshold){
		sem_wait(&n);
		printf("Ninja #%d is the last ninja to be served in this batch!\n", stats->id);
	}
	sem_post(&q);

	sem_post(&b);


	sem_wait(&s);
	sem_wait(&c);
	//this is the beginning of the critical section of the store
	sem_wait(&b);
	waitCount -=1;
	//printf("A ninja #%d has left the waiting room!\nWait count: %d\n", stats->id, waitCount);
	sem_post(&b);
	served+=1;

	if (stats -> hasReturned){
	printf("Ninja %d came back\n",stats->id);
	}
	//determines which staff member gets to serve the next customer
	if(served%numTeams == 0){
		minion1.busyTime += stats -> costumeTime;
		stats -> teamUsed = minion1.workerId;

	}
	else if(served%numTeams == 1){
		minion2.busyTime += stats -> costumeTime;
		stats -> teamUsed = minion2.workerId;
	}
	else if(served%numTeams == 2){
		minion3.busyTime += stats -> costumeTime;
		stats -> teamUsed = minion3.workerId;
	}
	else if(served%numTeams == 3){
		minion4.busyTime += stats -> costumeTime;
		stats -> teamUsed = minion4.workerId;
	}

	inStore +=1;
	stats -> goldPaid = stats -> costumeTime;
	//end time to calculate wait time of the thread
	gettimeofday(&timeServed, NULL);

	//saving how long the thread waited to get into the store
	stats -> waitTime = (timeServed.tv_sec - start.tv_sec);
	//calculates the total time of the visit for the thread
	stats -> totalTimeOfVisit = (stats -> waitTime + stats -> costumeTime);

	//check if they get a free costume
	if (timeServed.tv_sec - start.tv_sec > 30){
		printf("Ninja %d got a free costume\n", stats->id);
		stats -> goldPaid = 0;
	}
	//reports gold paid 
	stats -> totalGold+= stats -> goldPaid;
	//initializes another thread to act as the returning thread. 
	if (stats -> comeBack <= 25){
		pthread_t *newGuy = (pthread_t*)malloc(sizeof(pthread_t));
		customerStats *newStats = malloc(sizeof(customerStats));
		newStats -> id = stats ->id;
		newStats -> costumeTime = abs(stats ->costumeTime +(sqrt(-2 * log(drand48())) * cos(2 * M_PI * drand48())));
		newStats ->waitTime = 0;
		newStats -> comeBack = rand() % 100 + 1;
		newStats ->type = 'n';
		newStats -> teamUsed = 0;
		newStats -> goldPaid = 0;
		newStats -> arrivalTime = abs(stats ->arrivalTime +(sqrt(-2 * log(drand48())) * cos(2 * M_PI * drand48())));
		newStats -> totalTimeOfVisit = 0;
		newStats -> totalGold = 0;
		newStats->hasReturned = 1;
		pthread_create(newGuy, NULL, runStore, newStats);
	}


	sem_post(&c);

	//this simulates the amount of time the thread spends in the store
	sleep(stats -> costumeTime);

	sem_wait(&c);
	inStore -=1;
	//end of the in store critical section
	sem_post(&c);

	//under this condition we do a batch type switch
	if(inStore == 0 && waitCount == 0){
		sem_wait(&b);
		batch = 0;
		sem_post(&b);
		//either type can enter after this switch
		if(ninjaCount == 0 && pirateCount == 0){
			//printf("We are doing a total reset\n");
			sem_wait(&b);
			arivedCustomers = 0;
			sem_post(&b);
			sem_post(&p);
			sem_post(&n);
		}
		//ninjas are up after this switch
		else if (pirateCount == 0){
			sem_post(&n);
		}
		//Pirates are up after this switch
		else{
			sem_post(&p);
		}
	}
	
	printf("Ninja #%d has been served and is leaving\n", stats->id);
	sem_post(&s);

}
if(stats->type == 'p'){
//this is to get into thte waiting room

	sem_wait(&x);
	//this sequence may not make sence, but it allows us to lock p and still procede using b
	sem_wait(&p);
	sem_wait(&b);
	sem_post(&p);
	printf("A pirate #%d has entered the waiting room!\n", stats->id);
	waitCount +=1;
	pirateCount -=1;
	batch +=1;
	if(batch == threshold){
		sem_wait(&p);
		printf("Pirate #%d is the last pirate to be served!\n", stats->id);
	}
	sem_post(&x);

	sem_post(&b);


	sem_wait(&s);
	sem_wait(&c);
	//start of the critical section for being in the store
	sem_wait(&b);
	waitCount -=1;
	sem_post(&b);

	served+=1;	

	if (stats -> hasReturned){
		printf("Pirate %d came back\n",stats->id);
	}

	//determines which staff member serves the next customer
	if(served%numTeams == 0){
		minion1.busyTime += stats -> costumeTime;
		stats -> teamUsed = minion1.workerId;
	}
	else if(served%numTeams == 1){
		minion2.busyTime += stats -> costumeTime;
		stats -> teamUsed = minion2.workerId;
	}
	else if(served%numTeams == 2){
		minion3.busyTime += stats -> costumeTime;
		stats -> teamUsed = minion3.workerId;
	}
	else if(served%numTeams == 3){
		minion4.busyTime += stats -> costumeTime;
		stats -> teamUsed = minion4.workerId;
	}

	inStore +=1;
	stats -> goldPaid = stats -> costumeTime;
	//end time to derive wait time of the thread
	gettimeofday(&timeServed, NULL);

	//this will save how long this thread waited
	stats -> waitTime = (timeServed.tv_sec - start.tv_sec);
	//saves the total time of the visit.
	stats -> totalTimeOfVisit = stats -> waitTime + stats -> costumeTime;

	//check to see if they waited over 30 minutes, and give them a free costume
	if (timeServed.tv_sec - start.tv_sec > 30){
		printf("Pirate %d got a free costume\n", stats->id);
		stats -> goldPaid = 0;
	}
	stats -> totalGold+= stats -> goldPaid;
	//generates another thread to act as though this thread is coming back
	if (stats -> comeBack <= 25){
		pthread_t *newGuy = (pthread_t*)malloc(sizeof(pthread_t));
		customerStats *newStats = malloc(sizeof(customerStats));
		newStats -> id = stats ->id;
		newStats -> costumeTime = abs(stats ->costumeTime +(sqrt(-2 * log(drand48())) * cos(2 * M_PI * drand48())));
		newStats ->waitTime = 0;
		newStats -> comeBack = rand() % 100 + 1;
		newStats ->type = 'p';
		newStats -> teamUsed = 0;
		newStats -> goldPaid = 0;
		newStats -> arrivalTime = abs(stats ->arrivalTime +(sqrt(-2 * log(drand48())) * cos(2 * M_PI * drand48())));
		newStats -> totalTimeOfVisit = 0;
		newStats -> totalGold = 0;
		newStats->hasReturned = 1;
		pthread_create(newGuy, NULL, runStore, newStats);
	}

	sem_post(&c);

	//simulates time in store
	sleep(stats -> costumeTime);
	//we do this here so that the thread has already simulated its instore time.
	sem_wait(&c);
	inStore -=1;
	sem_post(&c);

	//checks if we need to do a type switch
	if(inStore == 0 && waitCount == 0){
		sem_wait(&b);
		batch = 0;
		sem_post(&b);
		//switches to both open and lets the next to show close the others door
		if(ninjaCount == 0 && pirateCount == 0){
			sem_wait(&b);
			arivedCustomers = 0;
			sem_post(&b);
			sem_post(&p);
			sem_post(&n);
		}
		//switches to pirates
		else if (ninjaCount == 0){
			sem_post(&p);
		}
		//switches to ninjas
		else{
			sem_post(&n);
		}
	}
	
	printf("Pirate %d has been served and is leaving\n", stats->id);
	sem_post(&s);
}
gettimeofday(&end, NULL);
//if we have served everyone, start printing
if (served == threadStartTotal){
	sem_post(&printer);
}
//makes the threads wait to print until they have all been served
sem_wait(&printer);
	//calculate agregate data for the day
	mainLedger.grossRevenue += stats -> totalGold;
	mainLedger.totalWaitTime += stats -> waitTime;
	mainLedger.avgQueLength = ((mainLedger.totalWaitTime) / threadStartTotal);
	mainLedger.avgGoldPerVisit = (mainLedger.grossRevenue / threadStartTotal);
	//prints data for each thread 
if(stats -> type == 'p'){
	printf("Pirate %d Spent %.2f minutes waiting before being served, spent %d gold on his costume and spent a total of %.2f minutes on the trip\n\n", stats -> id, stats -> waitTime, stats -> goldPaid, stats -> totalTimeOfVisit);
}
else if(stats -> type == 'n'){
	printf("Ninja %d Spent %.2f minutes before being served, spent %d gold on his costume and spent a total of %.2f minutes on the trip\n\n", stats -> id, stats -> waitTime, stats -> goldPaid, stats -> totalTimeOfVisit);
}
sem_post(&printer);
return 0;
}


void printAgregateDataAndWorkerDataI(){
	//prints worker data
	printf("Worker Data:\n");
	printf("|Worker ID|Idle Time|Busy Time|\n");
	printf("-------------------------------\n");
	if(numTeams == 2){
		printf("|%9d|%9.2f|%9.2f|\n", minion1.workerId, minion1.idleTime, minion1.busyTime);
		printf("|%9d|%9.2f|%9.2f|\n", minion2.workerId, minion2.idleTime, minion2.busyTime);
	}
	if(numTeams == 3){
		printf("|%9d|%9.2f|%9.2f|\n", minion1.workerId, minion1.idleTime, minion1.busyTime);
		printf("|%9d|%9.2f|%9.2f|\n", minion2.workerId, minion2.idleTime, minion2.busyTime);
		printf("|%9d|%9.2f|%9.2f|\n", minion3.workerId, minion3.idleTime, minion3.busyTime);
	}
	if(numTeams == 4){
		printf("|%9d|%9.2f|%9.2f|\n", minion1.workerId, minion1.idleTime, minion1.busyTime);
		printf("|%9d|%9.2f|%9.2f|\n", minion2.workerId, minion2.idleTime, minion2.busyTime);
		printf("|%9d|%9.2f|%9.2f|\n", minion3.workerId, minion3.idleTime, minion3.busyTime);
		printf("|%9d|%9.2f|%9.2f|\n", minion4.workerId, minion4.idleTime, minion4.busyTime);
	}

	//time for the agregate data of the day
	printf("\n");
	printf("Store Day Data:\n");
	printf("|Total gold earned|Gross Profit|Average Que Time|Avg Gold per Visit\n");
	printf("|%17d|%12d|%16.2f|%18.2f\n", mainLedger.grossRevenue, mainLedger.grossRevenue - (5*numTeams), mainLedger.avgQueLength, mainLedger.avgGoldPerVisit);


	}



int main(int argc, char *argv[]){
	srand48(time(NULL));
	int numPirates;
	int numNinjas;
	double avgPStore;
	double avgNStore;
	double avgPArrival;
	double avgNArrival;
	struct timeval dayStart;
	struct timeval dayEnd;
	//initalize all of the workers and ledger
	mainLedger.avgQueLength = 0;
	mainLedger.totalWaitTime = 0;
	mainLedger.avgGoldPerVisit = 0;
	mainLedger.grossRevenue = 0;
	mainLedger.totalProfits = 0;

	minion1.workerId = 1;
	minion1.idleTime = 0;
	minion1.busyTime = 0;

	minion2.workerId = 2;
	minion2.idleTime = 0;
	minion2.busyTime = 0;
	
	minion3.workerId = 3;
	minion3.idleTime = 0;
	minion3.busyTime = 0;
	
	minion4.workerId = 4;
	minion4.idleTime = 0;
	minion4.busyTime = 0;

	//accepting command line arguments
	if(argc > 8){
		printf("Too many arguments!!\n");
		EXIT_FAILURE;
	}
	else if(argc < 8){
		printf("Too little arguments!!\n");
		EXIT_FAILURE;
	}
	else{
		numTeams = atoi(argv[1]);
		numPirates = atoi(argv[2]);
		numNinjas = atoi(argv[3]);
		avgPStore = atof(argv[4]);
		avgNStore = atof(argv[5]);
		avgPArrival = atof(argv[6]);
		avgNArrival = atof(argv[7]);
	}
	//makes space for all of the threads that we are going to make
	pthread_t *customer = (pthread_t*)malloc((numNinjas+numPirates) * sizeof(pthread_t));
	gettimeofday(&dayStart, NULL);
	//initalizes all of the semaphores
	sem_init(&n, 0, 1);
	sem_init(&p, 0, 1);
	sem_init(&s, 0, numTeams);
	sem_init(&c, 0, 1);
	sem_init(&b, 0, 1);
	sem_init(&q, 0, 1);
	sem_init(&x, 0, 1);
	sem_init(&printer, 0, 0);
	customerStats *Nstats;
	//initalizes all of the customer structs
	for(int i = 0; i < (numNinjas+numPirates); i++){
		Nstats = malloc(sizeof(customerStats));
		Nstats->goldPaid = 0;
		Nstats -> totalGold = 0;
		Nstats->waitTime = 0;
		Nstats -> hasReturned = 0;
		Nstats->totalTimeOfVisit = 0;
		Nstats->teamUsed = 0;
		Nstats->id = i;
		Nstats->comeBack = rand() % 100 + 1;
		if(i<numNinjas){
			Nstats->arrivalTime = abs(avgNArrival + (sqrt(-2 * log(drand48())) * cos(2 * M_PI * drand48())));
			Nstats->costumeTime = abs(avgNStore + (sqrt(-2 * log(drand48())) * cos(2 * M_PI * drand48())));
			Nstats->type = 'n';
		}	
		else{
			Nstats->arrivalTime = abs(avgPArrival + (sqrt(-2 * log(drand48())) * cos(2 * M_PI * drand48())));
			Nstats->costumeTime = abs(avgPStore + (sqrt(-2 * log(drand48())) * cos(2 * M_PI * drand48())));
			Nstats->type = 'p';	
		}
		pthread_create(&customer[i], NULL, runStore, Nstats);
	}
	for(int i = 0; i < (numNinjas+numPirates); i++){
		pthread_join(customer[i],NULL);
	}
	//gets idle time for the minions
	gettimeofday(&dayEnd, NULL);
	totalTime = dayEnd.tv_sec - dayStart.tv_sec;
	minion1.idleTime = ((float)totalTime) - minion1.busyTime;
	minion2.idleTime = ((float)totalTime) - minion2.busyTime;
	minion3.idleTime = ((float)totalTime) - minion3.busyTime;
	minion4.idleTime = ((float)totalTime) - minion4.busyTime;

	printAgregateDataAndWorkerDataI();
	return 0;
}
