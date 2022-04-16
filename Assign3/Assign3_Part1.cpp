//This boilerplate is provided by a combination of code from all the TAs and the textbook

#include <stdio.h>   
#include <pthread.h>  
#include <string> 
#include <unistd.h>   
#include <sys/types.h>
#include <semaphore.h>
#include <vector>
#include <iostream>
#include <cstring>
#include <sys/wait.h>
#include <algorithm>
#include <sstream>
#include <chrono>
#include <cstdlib>
#include <random>
#include <iterator>
#define NUM_PHILOSOPHERS 5

using namespace std;

//Global variables
int numChopsticks;
vector <double> avgOnePhil(NUM_PHILOSOPHERS);

struct WaiterMonitor{
    
    int availableChopsticks;
	
    sem_t mutex;
    sem_t nextSem;
    int nextCount;

    //Need this for the wait and signal function, need a lock mutex and the conditional variable
    sem_t cond_oneChopstick;
    int mutex_oneChopstick;
    sem_t cond_twoChopsticks;
    int mutex_twoChopsticks;

    void init(){
        //Initialize all variables
        availableChopsticks = numChopsticks;

        sem_init(&mutex, 0, 1);
        sem_init(&nextSem, 0, 0);
        nextCount = 0;

        sem_init(&cond_oneChopstick, 0, 0);
        mutex_oneChopstick = 0;
        sem_init(&cond_twoChopsticks, 0, 0);
        mutex_twoChopsticks = 0;
    }

    void destroy(){
        sem_destroy(&mutex);
        sem_destroy(&cond_oneChopstick);
        sem_destroy(&cond_twoChopsticks);
    }

    //Equivalent to pthread_cond_wait(&condition, &mutex)
    void condition_wait(sem_t &condition_sem, int &condition_count){
        condition_count++;
        if (nextCount > 0)
            sem_post(&nextSem);
        else
            sem_post(&mutex);

        sem_wait(&condition_sem);
        condition_count--;
    }

    //Equivalent to pthread_cond_signal(&condition)
    void condition_post(sem_t &condition_sem, int &condition_count){
        if (condition_count > 0){
           
            nextCount++;
            sem_post(&condition_sem);
            sem_wait(&nextSem);
            nextCount--;
        }
    }

    void request_left_chopstick(){
        sem_wait(&mutex);

        //Wait for atleast two available chopsticks before giving it to the philosopher
        while( availableChopsticks < 2)
            condition_wait(cond_twoChopsticks, mutex_twoChopsticks);

        availableChopsticks--;

        //If at least one chopstick remains, signal to the 'at least one chopstick available' condition
        if( availableChopsticks >= 1){
            condition_post(cond_oneChopstick, mutex_oneChopstick);
           
            //If at least two chopstick remains, signal to the 'at least two chopstick available' condition
            if( availableChopsticks >= 2)
                condition_post(cond_twoChopsticks, mutex_twoChopsticks);
        }

        if (nextCount > 0)
            sem_post(&nextSem);
        else
            sem_post(&mutex);
    }

    void request_right_chopstick(){
        sem_wait(&mutex);

		//Wait for atleast one available chopsticks before giving it to the philosopher
        while( availableChopsticks < 1)
            condition_wait(cond_oneChopstick, mutex_oneChopstick);

        availableChopsticks--;

        //If at least one chopstick remains, signal to the 'at least one chopstick available' condition
        if( availableChopsticks >= 1){
            condition_post(cond_oneChopstick, mutex_oneChopstick);
           
            //If at least two chopstick remains, signal to the 'at least two chopstick available' condition
            if( availableChopsticks >= 2)
                condition_post(cond_twoChopsticks, mutex_twoChopsticks);
        }
		
        if (nextCount > 0)
            sem_post(&nextSem);
        else
            sem_post(&mutex);
    }

    void return_chopsticks(){
        sem_wait(&mutex);

		//put back two chopsticks
        availableChopsticks += 2;

        //If at least one chopstick remains, signal to the 'at least one chopstick available' condition
        if( availableChopsticks >= 1){
            condition_post(cond_oneChopstick, mutex_oneChopstick);
           
            //If at least two chopstick remains, signal to the 'at least two chopstick available' condition
            if( availableChopsticks >= 2)
                condition_post(cond_twoChopsticks, mutex_twoChopsticks);
        }

        if (nextCount > 0)
            sem_post(&nextSem);
        else
            sem_post(&mutex);
    }
};


struct WaiterMonitor waiter;
//Function for the threads
void * philosopher(void * arg){

    int id = *(int*) arg;
    srand(time(NULL) + id);
    chrono::duration<double> diff;
    chrono::duration<double> totalTime;
    
    //Only eat three times for each philosopher then print avg time
    for(int i = 0; i < 3; i++){
        //Thinking
        printf("Philosopher %d is thinking\n", id);
        int think_time = 1 + (rand() % 5);
        sleep(think_time);

        //Hungry
         printf("Philospher %d is hungry\n", id);

        //Start timer
        auto start = chrono::system_clock::now();
        
        waiter.request_left_chopstick();
        printf("Philospher %d has picked up left chopstick\n", id);
        waiter.request_right_chopstick();
        printf("Philospher %d has picked up right chopstick\nPhilosopher %d is eating\n", id, id);
        
        //End timer
        auto end = chrono::system_clock::now();
	    diff = end - start;
        //Eating
        sleep(5);

        printf("Philospher %d is done eating\n", id);
        waiter.return_chopsticks();
        totalTime += diff;
    }
    //Average time (s) of each philosopher after eating 3 times
    avgOnePhil.at(id-1) = totalTime.count()/3;
    pthread_exit(NULL);
}

int main(int argc, char* argv[]){

    if (argc < 2 || argc > 2 ) {
        cerr << "Please enter one input only";
        return 1;
    }
    int inputChopstick = atoi(argv[1]);

    if(inputChopstick < 5 || inputChopstick > 10 ){
        cerr << "Please enter the number of chopsticks between 5 - 10";
        return 1;
    }
    numChopsticks = inputChopstick;
   
    waiter.init();
    vector <pthread_t> tids;

	for(int i = 1; i <= NUM_PHILOSOPHERS; i++){
		pthread_t tid;
		int * arg = new int;
		*arg = i;
		pthread_create(&tid, NULL, philosopher, arg);
		tids.push_back(tid);		
	}
	for(int i =0; i < tids.size(); i ++){
		pthread_join(tids.at(i), NULL);
	}

    waiter.destroy();
    cout << "\n\nNumber of Chopsticks: " << numChopsticks << '\n';
    
    double totalAvgTime = 0;
    for(int j =0; j < NUM_PHILOSOPHERS; j ++){
        cout << " \nAvg waiting time for Philosopher " << j+1  << ": " << avgOnePhil.at(j) << "s\n";
        totalAvgTime+= avgOnePhil.at(j);
    }
    totalAvgTime = double(totalAvgTime /NUM_PHILOSOPHERS);
    cout << "\nTotal average time: " << totalAvgTime << "s\n";
    
    return 0;
}

