#include<iostream>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdio.h> 
#include <unistd.h>
#include <semaphore.h>
#include <time.h> 
#include <pthread.h>
#include <random>

using namespace std;

#define MAX_BUFFER_SIZE 10
//Vegan variables initialization
sem_t mutexVegan;
sem_t fullVegan;
sem_t emptyVegan;
void *veganConsumer(void* arg);
void *nonVeganConsumer(void* arg);
void *hybridConsumer(void* arg);
int inVegan, outVegan, veganCount;
int* veganTray;

//Non Vegan variables initialization
sem_t mutexNonVegan;
sem_t fullNonVegan;
sem_t emptyNonVegan;
void *donatelloProducer(void* arg);
void *portecelliProducer(void* arg);
int inNonVegan, outNonVegan, nonVeganCount;
int* nonVeganTray;

//Print buffer every 10 seconds
void *printTray(void* arg);

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

int main()
{

    veganTray = (int*)malloc(MAX_BUFFER_SIZE*sizeof(int));
    nonVeganTray = (int*)malloc(MAX_BUFFER_SIZE*sizeof(int));
    inVegan, outVegan, inNonVegan, outNonVegan, veganCount, nonVeganCount = 0;
    
    
    //Initialize buffer to be all 0's
    for (int i = 0; i < MAX_BUFFER_SIZE; i++){
        veganTray[i] = 0;
        nonVeganTray[i] = 0;
    }
    
    sem_init(&mutexNonVegan, 0, 1);
    sem_init(&emptyNonVegan, 0, MAX_BUFFER_SIZE);
    sem_init(&fullNonVegan, 0, 0);

    sem_init(&mutexVegan, 0, 1);
    sem_init(&emptyVegan, 0, MAX_BUFFER_SIZE);
    sem_init(&fullVegan, 0, 0);
   
    //Create Threads
    pthread_t donatelloID, portecelliID, veganID, nonVeganID, hybridID, printTrayID;

    int result = pthread_create(&donatelloID, NULL, donatelloProducer, NULL);
    if (result != 0)
       cout <<"Error creating Donatello producer thread\n";
    
    result = pthread_create(&portecelliID, NULL, portecelliProducer, NULL);
    if (result != 0)
        cout << "Error creating Portecelli producer thread\n";
    
    result = pthread_create(&veganID, NULL, veganConsumer, NULL);
    if (result != 0)
        cout << "Error creating Vegan consumer thread\n";
    
    result = pthread_create(&nonVeganID, NULL, nonVeganConsumer, NULL);
    if (result != 0)
        cout << "Error creating Non-Vegan consumer thread\n";
    
    result = pthread_create(&hybridID, NULL, hybridConsumer, NULL);
    if (result != 0)
        cout << "Error creating Hybrid consumer thread\n";

    result = pthread_create(&printTrayID, NULL, printTray, NULL);
    if (result != 0)
        cout << "Error creating Print tray thread\n";

    //Join threads
    pthread_join(donatelloID, NULL);
    pthread_join(portecelliID, NULL);
    pthread_join(veganID, NULL);
    pthread_join(nonVeganID, NULL);
    pthread_join(hybridID, NULL);
    pthread_join(printTrayID, NULL);

    free(veganTray);
    free(nonVeganTray);

    exit(0);
}

void *donatelloProducer(void* arg)
{
    while(1)
    {
        //Produce one out of the two items randomly
        int nonVeganProduce = getRandomNum(1,2);
        
        sem_wait(&emptyNonVegan);
        sem_wait(&mutexNonVegan);

        nonVeganTray[inNonVegan] = nonVeganProduce;
        inNonVegan = (inNonVegan+1) % MAX_BUFFER_SIZE;
        nonVeganCount++;

        if(nonVeganProduce == 1)
			cout <<"Donatello creates a non-vegan dish: Fettuccine Chicken Alfredo \n";
		else if(nonVeganProduce == 2)
			cout <<"Donatello creates a non-vegan dish: Garlic Sirloin Steak \n";
			
        sem_post(&mutexNonVegan);
        sem_post(&fullNonVegan);

        //Sleep between 1 and 5 seconds
        sleep(getRandomNum(1,5));
    }

    pthread_exit (NULL);
}

void *portecelliProducer(void* arg)
{
    while(1)
    {
        //Produce one out of the two items randomly
        int veganProduce = getRandomNum(1,2);
        sem_wait(&emptyVegan);
        sem_wait(&mutexVegan);

        veganTray[inVegan] = veganProduce;
        inVegan = (inVegan+1) % MAX_BUFFER_SIZE;
        veganCount++;

        if(veganProduce == 1)
            cout <<"Portecelli creates a vegan dish: Pistachio Pesto Pasta \n";
        else if(veganProduce == 2)
            cout <<"Portecelli creates a vegan dish: Avocado Fruit Salad \n";
			
        sem_post(&mutexVegan);
        sem_post(&fullVegan);
    
        //Sleep between 1 and 5 seconds
        sleep(getRandomNum(1,5));
    }
   
    pthread_exit (NULL);
}

void *veganConsumer(void* arg)
{
    while(1)
    {
        sem_wait(&fullVegan);
		sem_wait(&mutexVegan);
        
        //Take the food item and set the value at the position equal to 0 (unoccupied)
        int veganItem = veganTray[outVegan];
        veganTray[outVegan] = 0;
        outVegan = (outVegan+1) % MAX_BUFFER_SIZE;
        veganCount--;

        if(veganItem == 1)
            cout <<"Vegan customer removes vegan dish: Pistachio Pesto Pasta \n" ;
        else if(veganItem ==2)
            cout <<"Vegan customer removes vegan dish: Avocado Fruit Salad \n" ;
        else 
            cout <<"Wrong item: Vegan customer \n";
			
        sem_post(&mutexVegan);
        sem_post(&emptyVegan);

        //Sleep between 10 and 15 seconds
        sleep(getRandomNum(10,15));
    }
    pthread_exit (NULL);
}

void *nonVeganConsumer(void* arg)
{
    while(1)
    {
        sem_wait(&fullNonVegan);
		sem_wait(&mutexNonVegan);
        
        int nonVeganItem = nonVeganTray[outNonVegan];
        nonVeganTray[outNonVegan] = 0;
        outNonVegan = (outNonVegan+1) % MAX_BUFFER_SIZE;
        nonVeganCount--;

        if(nonVeganItem == 1)
            cout << "Non vegan customer removes non vegan dish: Fettucine Chicken Alfredo \n" ;
        else if(nonVeganItem == 2)
            cout <<"Non vegan customer removes non vegan dish: Garlic Sirloin Steak \n" ;
        else 
            cout <<"Wrong item: Non Vegan customer \n";
        
        sem_post(&mutexNonVegan);
        sem_post(&emptyNonVegan);

        //Sleep between 0 and 5 seconds
        sleep(getRandomNum(10,15));
    }
    pthread_exit (NULL);
}

void *hybridConsumer(void* arg)
{
    while(1)
    {
        string message = "";
        sem_wait(&fullNonVegan);
		sem_wait(&mutexNonVegan);

        sem_wait(&fullVegan);
		sem_wait(&mutexVegan);
        
        int nonVeganItem = nonVeganTray[outNonVegan];
        nonVeganTray[outNonVegan] = 0;
        outNonVegan = (outNonVegan+1) % MAX_BUFFER_SIZE;
        nonVeganCount--;

        if(nonVeganItem == 1)
            message = "Hybrid customer removes non vegan dish: Fettucine Chicken Alfredo and " ;
        else if(nonVeganItem == 2)
            message ="Hybrid customer removes non vegan dish: Garlic Sirloin Steak and " ;
        else 
            cout <<"Wrong item: Hybrid non vegan customer \n";

        
        int veganItem = veganTray[outVegan];
        veganTray[outVegan] = 0;
        outVegan = (outVegan+1) % MAX_BUFFER_SIZE;
        veganCount--;

        if(veganItem == 1)
            message = message + "vegan dish: Pistachio Pesto Pasta \n" ;
        else if(veganItem ==2)
            message = message + "vegan dish: Avocado Fruit Salad \n" ;
        else 
            cout << "Wrong item: Hybrid Vegan customer \n";
			
        cout << message;
        
        sem_post(&mutexVegan);
        sem_post(&emptyVegan);
        
        sem_post(&mutexNonVegan);
        sem_post(&emptyNonVegan);

        sleep(getRandomNum(10,15));
    }
    pthread_exit (NULL);
}

void *printTray(void* arg){
    while(1){
        sleep(10);
        cout << "Items in the vegan tray: " << veganCount << "/" << MAX_BUFFER_SIZE << "\n";
        cout << "Items in the non vegan tray: " << nonVeganCount << "/" << MAX_BUFFER_SIZE << "\n";                       
    }  
    pthread_exit (NULL); 
}