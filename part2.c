#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#define NorthEast 1
#define NorthWest 2
#define SouthWest 3
#define SouthEast 4
#define Straight 1
#define Left 2
#define Right 3


//first four are locks for the actual quadrants (inside the intersection), next four are for the head of each linked list
pthread_mutex_t nw = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ne = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sw = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t se = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t qnw = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t qne = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t qsw = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t qse = PTHREAD_MUTEX_INITIALIZER;
//maybe use a linked list...







struct CarNode{
	//carInfo *info;
	int id;
	int d;
	int t;
	char dir[20];
	char turn[10];
	int didTurn;
	struct CarNode *next;
};

struct CarNode *NWHead = NULL;
struct CarNode *NEHead = NULL;
struct CarNode *SWHead = NULL;
struct CarNode *SEHead = NULL;


//adds the newly randomized car to the end of the que coming from its specified direction.
int addToList(struct CarNode *car){
	struct CarNode *current;
	if(car -> d == NorthEast){
		current = NEHead;
		if(NEHead == NULL){
			NEHead = car;
		}
		else{
			while(current->next != NULL){
				current = current->next;
			}
			current->next = car;
			car->next = NULL;
		}
	}
	else if(car -> d == NorthWest){
		current = NWHead;
		if(NWHead == NULL){
			NWHead = car;
		}
		else{
			while(current->next != NULL){
				current = current->next;
			}
			current->next = car;
			car->next = NULL;
		}
	}
	else if(car -> d == SouthEast){
		current = SEHead;
		if(SEHead == NULL){
			SEHead = car;
		}
		else{
			while(current->next != NULL){
				current = current->next;
			}
			current->next = car;
			car->next = NULL;
		}
	}
	else{	
		current = SWHead;
		if(SWHead == NULL){
			SWHead = car;
		}
		else{
			while(current->next != NULL){
				current = current->next;
			}
			current->next = car;
			car->next = NULL;
		}	
	}

	return 0;
}
//pops the head of the linked list representing the que of cars coming from each direction
int removeFromList(struct CarNode *car){
	if(car->next->d == NorthWest){
		NWHead = car->next;
	}
	else if(car->next->d == NorthEast){
		NEHead = car->next;
	}
	else if(car->next->d == SouthWest){
		SWHead = car->next;
	}
	else if(car->next->d == SouthEast){
		SEHead = car->next;
	}
	return 0;
}


//function to re-randomize each thread's parameters
int randomize(struct CarNode *info){
	info->d = rand() % 4+1;
	info->t= rand() % 3+1;
	char *dirBuf;
	char *turnBuf;
	if(info->d == NorthEast){
		dirBuf = "North East";
		strcpy(info->dir, dirBuf);
	}
	if(info->d == NorthWest){
		dirBuf = "North West";
		strcpy(info->dir, dirBuf);
	}
	if(info->d == SouthWest){
		dirBuf = "South West";
		strcpy(info->dir, dirBuf);
	}
	if(info->d == SouthEast){
		dirBuf = "South East";
		strcpy(info->dir, dirBuf);
	}

	if(info->t == Straight){
		turnBuf = "straight";
		strcpy(info->turn, turnBuf);
	}
	if(info->t == Left){
		turnBuf = "left";
		strcpy(info->turn, turnBuf);
	}
	if(info->t == Right){
		turnBuf = "right";
		strcpy(info->turn, turnBuf);
	}
	info->didTurn = 0;
	return 0;
}
//outter if checks if the car is first in line in its direction
//inner if checks the type of turn the car wants to make
//try lock checks if all of the quadrants of the intersection needed for the desired turn are available, thus making the turn possible.
int makeTurn(struct CarNode *info){
	if(NEHead == info){
		if(info -> t == Left){
			if(pthread_mutex_trylock(&ne) == 0){
				if(pthread_mutex_trylock(&nw) == 0){
					if(pthread_mutex_trylock(&sw) == 0){
						printf("Car #%d is making a %s turn from the %s\n", info -> id, info -> turn, info -> dir);
						sleep(2);
						printf("Car #%d is leaving the intersection!\n\n",info->id);
						info->didTurn = 1;
						pthread_mutex_unlock(&ne);
						pthread_mutex_unlock(&nw);
						pthread_mutex_unlock(&sw);						
					}
					else{
						pthread_mutex_unlock(&nw);
						pthread_mutex_unlock(&ne);
					}
				}
				else{
					pthread_mutex_unlock(&ne);
				}
			}
			else{
				sleep(0.1);
			} 
		}
		
		else if(info -> t == Straight){
			if(pthread_mutex_trylock(&ne) == 0){
				if(pthread_mutex_trylock(&nw) == 0){
					printf("Car #%d is making a %s turn from the %s\n", info -> id, info -> turn, info -> dir);
					sleep(2);
					printf("Car #%d is leaving the intersection!\n\n",info->id);
					info->didTurn = 1;
					pthread_mutex_unlock(&ne);
					pthread_mutex_unlock(&nw);
				}
				else{
					pthread_mutex_unlock(&ne);
				}
			} 
			else{
				sleep(0.1);
			}	
		}
		else{
			if(pthread_mutex_trylock(&ne) == 0){
				printf("Car #%d is making a %s turn from the %s\n", info -> id, info -> turn, info -> dir);
				sleep(2);
				info->didTurn = 1;
				pthread_mutex_unlock(&ne);
				}
				else{
					sleep(0.1);
				}
		}
	}
	else if(NWHead == info){
		if(info -> t == Left){
			if(pthread_mutex_trylock(&nw) == 0){
				if(pthread_mutex_trylock(&sw) == 0){
					if(pthread_mutex_trylock(&se) == 0){
						printf("Car #%d is making a %s turn from the %s\n", info -> id, info -> turn, info -> dir);
						sleep(2);
						printf("Car #%d is leaving the intersection!\n\n",info->id);
						info->didTurn = 1;
						pthread_mutex_unlock(&nw);
						pthread_mutex_unlock(&sw);
						pthread_mutex_unlock(&se);
					}
					else{
						pthread_mutex_unlock(&sw);
						pthread_mutex_unlock(&nw);
					}
				}
				else{
					pthread_mutex_unlock(&nw);
				}
			}
			else{
				sleep(0.1);
			}
		}
		else if(info -> t == Straight){
			if(pthread_mutex_trylock(&nw) == 0){
				if(pthread_mutex_trylock(&sw) == 0){
					printf("Car #%d is making a %s turn from the %s\n", info -> id, info -> turn, info -> dir);
					sleep(2);
					printf("Car #%d is leaving the intersection!\n\n",info->id);
					info->didTurn = 1;
					pthread_mutex_unlock(&nw);
					pthread_mutex_unlock(&sw);
				}
				else{
					pthread_mutex_unlock(&nw);
				}
			}
			else{
				sleep(0.1);
			}
		}
		else{
			if(pthread_mutex_trylock(&nw) == 0){
				printf("Car #%d is making a %s turn from the %s\n", info -> id, info -> turn, info -> dir);
				sleep(2);
				printf("Car #%d is leaving the intersection!\n\n",info->id);
				info->didTurn = 1;
				pthread_mutex_unlock(&nw);
			}
			else{
				sleep(0.1);
			}
		}
	}

	else if(SWHead == info){
		if(info -> t == Left){
			if(pthread_mutex_trylock(&sw) == 0){
				if(pthread_mutex_trylock(&se) == 0){
					if(pthread_mutex_trylock(&ne) == 0){
						printf("Car #%d is making a %s turn from the %s\n", info -> id, info -> turn, info -> dir);
						sleep(2);
						printf("Car #%d is leaving the intersection!\n\n",info->id);
						info->didTurn = 1;
						pthread_mutex_unlock(&sw);
						pthread_mutex_unlock(&se);
						pthread_mutex_unlock(&ne);
					}
					else{
						pthread_mutex_unlock(&se);
						pthread_mutex_unlock(&sw);
					}
				}
				else{
					pthread_mutex_unlock(&sw);
				}
			}
			else{
				sleep(0.1);
			}
		}
		
		else if(info -> t == Straight){
			if(pthread_mutex_trylock(&sw) == 0){
				if(pthread_mutex_trylock(&se) == 0){
					printf("Car #%d is making a %s turn from the %s\n", info -> id, info -> turn, info -> dir);
					sleep(2);
					printf("Car #%d is leaving the intersection!\n\n",info->id);
					info->didTurn = 1;
					pthread_mutex_unlock(&sw);
					pthread_mutex_unlock(&se);
				}
				else{
					pthread_mutex_unlock(&sw);
				}
			}
			else{
				sleep(0.1);
			}
		}
		else{
			if(pthread_mutex_trylock(&sw) == 0){
				printf("Car #%d is making a %s turn from the %s\n", info -> id, info -> turn, info -> dir);
				sleep(2);
				printf("Car #%d is leaving the intersection!\n\n",info->id);
				info->didTurn = 1;
				pthread_mutex_unlock(&sw);
			}
			else{
				sleep(0.1);
			}
		}
	}


	else if(SEHead == info){
		if(info -> t == Left){
			if(pthread_mutex_trylock(&se) == 0){
				if(pthread_mutex_trylock(&ne) == 0){
					if(pthread_mutex_trylock(&nw) == 0){
						printf("Car #%d is making a %s turn from the %s\n", info -> id, info -> turn, info -> dir);
						sleep(2);
						printf("Car #%d is leaving the intersection!\n\n",info->id);
						info->didTurn = 1;
						pthread_mutex_unlock(&se);
						pthread_mutex_unlock(&ne);
						pthread_mutex_unlock(&nw);
					}
					else{
						pthread_mutex_unlock(&ne);
						pthread_mutex_unlock(&se);
					}
				}
				else{
					pthread_mutex_unlock(&se);
				}
			}
			else{
				sleep(0.1);
			}
		}
		else if(info -> t == Straight){
			if(pthread_mutex_trylock(&se) == 0){
				if(pthread_mutex_trylock(&ne) == 0){
					printf("Car #%d is making a %s turn from the %s\n", info -> id, info -> turn, info -> dir);
					sleep(2);
					printf("Car #%d is leaving the intersection!\n\n",info->id);
					info->didTurn = 1;
					pthread_mutex_unlock(&se);
					pthread_mutex_unlock(&ne);
				}
				else{
					pthread_mutex_unlock(&se);
				}
			}
			else{
				sleep(0.1);
			}
		}
		else{
			if(pthread_mutex_trylock(&se) == 0){
				printf("Car #%d is making a %s turn from the %s\n", info -> id, info -> turn, info -> dir);
				sleep(2);
				printf("Car #%d is leaving the intersection!\n\n",info->id);
				info->didTurn = 1;
				pthread_mutex_unlock(&se);
			}
			else{
				sleep(0.1);
			}
		}
	}
return 0;
}

//every thread loops getting new random data 
void *runTraffic(void *arg){
	struct CarNode *stats = (struct CarNode*)arg;
	pthread_mutex_lock(&qne);
	addToList(stats);
	pthread_mutex_unlock(&qne);
	while(1){
			makeTurn(stats);
			if(stats->didTurn == 1){
				struct CarNode *copy;
				copy = malloc(sizeof(struct CarNode));
				*copy = *stats;
				pthread_mutex_lock(&qne);
				removeFromList(stats);
				pthread_mutex_unlock(&qne);
				randomize(copy);
				pthread_mutex_lock(&qne);
				addToList(copy);
				pthread_mutex_unlock(&qne);
			}
			sleep(1);
	}
	return 0;
}


//initalize car threads to an initial random value
int main(){
	srand(time(0));
	struct CarNode *stats;
	pthread_t *car = (pthread_t*)malloc(20 * sizeof(pthread_t));
	for(int i = 0; i < 20; i++){
		stats = malloc(sizeof(struct CarNode));
		stats -> id = i;
		randomize(stats);
		randomize(stats);
		pthread_create(&car[i], NULL, runTraffic, stats);
	}
	for(int i = 0; i < 20; i ++){
		pthread_join(car[i], NULL);
	}
	return 0;
}