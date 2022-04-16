#include<iostream>
#include<sys/mman.h>
#include<semaphore.h>
#include<sys/types.h>
#include<unistd.h>
#include <random>
using namespace std;
#define MAX_BUFFER_SIZE 10

//==========================================================================
// The following function was adapted from this URL
// https://stackoverflow.com/questions/23838194/c-fast-random-number-generator/23838367#23838367
//
int getRandomNum(int low, int high){
    random_device rDevice;
    mt19937 gen(rDevice());
    uniform_int_distribution<> dis(low, high);

    return dis(gen);
}
//======================end of attributed code===========================

int main(){
	//Init for the non vegan tray/buffer
	int *nonVeganTray = (int*) mmap(NULL, sizeof(int)*MAX_BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1 ,0);
	int	*inNonVegan = (int*)mmap(NULL, sizeof(int*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    int	*outNonVegan = (int*)mmap(NULL, sizeof(int*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	int *nonVeganCounter = (int*) mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1 ,0);
	*(nonVeganCounter), *inNonVegan, *outNonVegan = 0;
	
	sem_t *mutexNonVegan = (sem_t*) mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	sem_t *fullNonVegan = (sem_t*)mmap(NULL, sizeof(sem_t*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_t *emptyNonVegan = (sem_t*)mmap(NULL, sizeof(sem_t*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	sem_init(mutexNonVegan, 1, 1);
    sem_init(emptyNonVegan, 1, MAX_BUFFER_SIZE);
    sem_init(fullNonVegan, 1, 0);

	//Init for the vegan tray/buffer
	int *veganTray = (int*) mmap(NULL, sizeof(int)*MAX_BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1 ,0);
	int	*inVegan = (int*)mmap(NULL, sizeof(int*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    int	*outVegan = (int*)mmap(NULL, sizeof(int*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	int *veganCounter = (int*) mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1 ,0);
    *(veganCounter),  *inVegan, *outVegan = 0;

	sem_t *mutexVegan = (sem_t*) mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	sem_t *fullVegan = (sem_t*)mmap(NULL, sizeof(sem_t*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	sem_t *emptyVegan = (sem_t*)mmap(NULL, sizeof(sem_t*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	sem_init(mutexVegan, 1, 1);
	sem_init(fullVegan, 1, 0);
	sem_init(emptyVegan , 1, MAX_BUFFER_SIZE);

	//Initialize buffers to be all 0's
    for (int i = 0; i < MAX_BUFFER_SIZE; i++){
        veganTray[i] = 0;
		nonVeganTray[i] = 0;
    }

	//Init for 5 children
	pid_t donatelloProducer;
	pid_t portecelliProducer;
	pid_t veganConsumer;
	pid_t nonVeganConsumer;
	pid_t hybridConsumer;

    //Child1 producer process
	donatelloProducer = fork();
	if(donatelloProducer == -1){
		cout <<"Error forking Donatello Producer\n" ;
		return -1 ;
	}
	else if(donatelloProducer == 0){
		while(1){
			int nonVeganProduce = getRandomNum(1,2);
		
			sem_wait(emptyNonVegan);
			sem_wait(mutexNonVegan);

			nonVeganTray[*inNonVegan] = nonVeganProduce;
			*inNonVegan = (*inNonVegan + 1) % MAX_BUFFER_SIZE;
			*(nonVeganCounter) =*(nonVeganCounter) + 1;
			if(nonVeganProduce == 1)
				cout <<"Donatello creates a non-vegan dish: Fettuccine Chicken Alfredo \n";
			else if(nonVeganProduce == 2)
				cout <<"Donatello creates a non-vegan dish: Garlic Sirloin Steak \n";
			
			sem_post(mutexNonVegan);
			sem_post(fullNonVegan);
	
			sleep(getRandomNum(1,5));
		}
	}
	else{			
		portecelliProducer = fork();
		if(portecelliProducer == -1){
			cout <<"Error forking Portecelli Producer\n";
			return -1 ;
		}
		else if (portecelliProducer == 0){
			while(1){
				int veganProduce = getRandomNum(1,2);
			
				sem_wait(emptyVegan);
				sem_wait(mutexVegan);

				veganTray[*inVegan] = veganProduce;
				*inVegan = (*inVegan + 1) % MAX_BUFFER_SIZE;
				*(veganCounter) = *(veganCounter) + 1;
				
				if(veganProduce == 1)
					cout <<"Portecelli creates a vegan dish: Pistachio Pesto Pasta \n";
				else if(veganProduce == 2)
					cout <<"Portecelli creates a vegan dish: Avocado Fruit Salad \n";

				sem_post(mutexVegan);
				sem_post(fullVegan);
				
				sleep(getRandomNum(1,5));
			}
		}
		else{
			veganConsumer = fork();
			if(veganConsumer == -1){
				cout <<"Error forking Vegan Consumer\n";
					return -1;
			}
			else if(veganConsumer == 0){
				while(1){	
					int veganItem;
					
					sem_wait(fullVegan);
					sem_wait(mutexVegan);

					veganItem = veganTray[*outVegan];
					veganTray[*outVegan] = 0;
					*outVegan = (*outVegan + 1) % MAX_BUFFER_SIZE;
					*(veganCounter) =*(veganCounter) - 1;
					
					if(veganItem == 1)
						cout <<"Vegan customer removes vegan dish: Pistachio Pesto Pasta \n" ;
					else if(veganItem ==2)
						cout <<"Vegan customer removes vegan dish: Avocado Fruit Salad \n" ;
					else 
						cout <<"Wrong item: Vegan customer \n";
					
					sem_post(mutexVegan);
					sem_post(emptyVegan);
					
					sleep(getRandomNum(10,15));
				}
			}
			else{
				nonVeganConsumer = fork();
				if(nonVeganConsumer == -1){
					cout <<"Error forking Non Vegan Consumer\n";
					return -1 ;
				}
				else if(nonVeganConsumer == 0){
					while(1){
						int nonVeganItem;
						sem_wait(fullNonVegan);
						sem_wait(mutexNonVegan);

						nonVeganItem = nonVeganTray[*outNonVegan];
						nonVeganTray[*outNonVegan] = 0;
						*outNonVegan = (*outNonVegan + 1) % MAX_BUFFER_SIZE;
						*(nonVeganCounter) =*(nonVeganCounter) - 1;
						
						if(nonVeganItem == 1)
							cout <<"Non vegan customer removes non vegan dish: Fettucine Chicken Alfredo \n" ;
						else if(nonVeganItem == 2)
							cout <<"Non vegan customer removes non vegan dish: Garlic Sirloin Steak \n" ;
						else 
							cout <<"Wrong item: Non Vegan customer \n";
						
						sem_post(mutexNonVegan);
						sem_post(emptyNonVegan);

						sleep(getRandomNum(10,15));
					}
				}
				else{
					hybridConsumer = fork();
					if(hybridConsumer == -1){
						cout <<"Error forking Hybrid Consumer\n";
						return -1 ;
					}
					else if(hybridConsumer == 0){
						while(1){
							int hybridVeganItem;
							int hybridNonVeganItem;
							string message = "";
							sem_wait(fullNonVegan);
							sem_wait(mutexNonVegan);

							sem_wait(fullVegan);
							sem_wait(mutexVegan);

							hybridNonVeganItem = nonVeganTray[*outNonVegan];
							nonVeganTray[*outNonVegan] = 0;
							*outNonVegan = (*outNonVegan + 1) % MAX_BUFFER_SIZE;
							*(nonVeganCounter) =*(nonVeganCounter) - 1;
							
							if(hybridNonVeganItem == 1)
								message = "Hybrid customer removes non vegan dish: Fettucine Chicken Alfredo and " ;
							else if(hybridNonVeganItem ==2)
								message = "Hybrid customer removes non vegan dish: Garlic Sirloin Steak and " ;
							else 
								cout<< "Wrong item: Hybrid non vegan customer \n";

							hybridVeganItem = veganTray[*outVegan];
							veganTray[*outVegan] = 0;
							*outVegan = (*outVegan + 1) % MAX_BUFFER_SIZE;
							*(veganCounter) =*(veganCounter) - 1;
							
							if(hybridVeganItem == 1)
								message = message + "vegan dish: Pistachio Pesto Pasta \n" ;
							else if(hybridVeganItem ==2)
								message = message + "vegan dish: Avocado Fruit Salad \n" ;
							else 
								cout <<"Wrong item: Hybrid Vegan customer \n";
								
							cout << message;
							sem_post(mutexVegan);
							sem_post(emptyVegan);
							sem_post(mutexNonVegan);
							sem_post(emptyNonVegan);
							
							sleep(getRandomNum(10,15));
						}
					}
					else{
						while(1){
							sleep(10);
							cout << "Items in the vegan tray: " << *(veganCounter) << "/" << MAX_BUFFER_SIZE << "\n";
							cout << "Items in the non vegan tray: " << *(nonVeganCounter) << "/" << MAX_BUFFER_SIZE << "\n";
							
						}
					}
				}
			}
		}
	}
					
return 0;
}
