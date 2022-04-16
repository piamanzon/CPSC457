#include <iostream>
#include <pthread.h>  
#include <string> 
#include <unistd.h>   
#include <semaphore.h>
#include <deque>
#include <stdio.h>   
#include <sys/types.h>
#include <vector>
#include <cstring>
#include <sys/wait.h>
#include <algorithm>
#include <sstream>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <cmath>

using namespace std; 
// ===========================================================================================
// The following boilerplate code was provided by Alex Garcia in his tutorial and email
// The boilerplate has been modified to fit the assignment application
// ===========================================================================================

//Global Variables
ifstream file;
bool customersFinished = false; //Will be used to exit consumer threads
bool doneReading = false;
int totalCustomer = 0;
int customerLeft = 0;
int quantumTime = 0;
int totalThread = 0;


struct Customer{
    int id;
    int eatingTimeLeft;
    int totalEatingTime;
    chrono::time_point<std::chrono::high_resolution_clock>  beginTime;
    double  waitTime = 0; //Accumulated wait time
    bool fakeCustomer = false; //Will be used to exit the consumer thread
};

struct QueueMonitor{
    //Customers in the queue 
    deque<Customer*> customers;

    sem_t mutex;
    sem_t nextSem;
    int nextCount;

    //Need this for the wait and signal function, need a lock mutex and the conditional variable
    sem_t cond_notEmpty;
    int mutex_notEmpty;

    void init(){
        //Initialize all variables
        customers = deque<Customer*>();

        sem_init(&mutex, 0, 1);
        sem_init(&nextSem, 0, 0);
        nextCount = 0;

        sem_init(&cond_notEmpty, 0, 0);
        mutex_notEmpty = 0;
    }

    void destroy(){
        sem_destroy(&mutex);
        sem_destroy(&cond_notEmpty);
    }

    //Equivalent to pthread_cond_wait(&condition, &mutex)
    void conditionWait(sem_t &conditionSem, int &conditionCount){
        conditionCount++;
        if (nextCount > 0)
            sem_post(&nextSem);
        else
            sem_post(&mutex);
        sem_wait(&conditionSem);
        conditionCount--;
    }

    //Equivalent to pthread_cond_signal(&condition)
    void conditionPost(sem_t &conditionSem, int &conditionCount){
        if (conditionCount > 0){
            nextCount++;
            sem_post(&conditionSem);
            sem_wait(&nextSem);
            nextCount--;
        }
    }

    //Function to sit customer by taking them out of the queue
    Customer *getCustomer(){
        sem_wait(&mutex);

        //Wait for atleast one customer in the queue
        while(customers.size() < 1)
            conditionWait(cond_notEmpty, mutex_notEmpty);
        
        Customer *c = customers.front();
        //Remove customer from the queue
        customers.pop_front();
       
        //If there is atleast one customer in the queue, signal to the 'not empty' condition
        if(customers.size() >= 1)
            conditionPost(cond_notEmpty, mutex_notEmpty);

        if (nextCount > 0)
            sem_post(&nextSem);
        else
            sem_post(&mutex);

        return c;
    }
    
    //Function to add customer to the queue
    void addCustomer(Customer* c){
        sem_wait(&mutex);

        // if((*c).fakeCustomer){//Add fake customers to the queue
        //     customers.push_back(c);
        //     customers.push_back(c);
        //     customers.push_back(c);
        //     customers.push_back(c);
        // }
        // else//Add the customer to the back queue
            customers.push_back(c);

        //Signal to the 'not empty' condition
        conditionPost(cond_notEmpty, mutex_notEmpty);
        
        if (nextCount > 0)
            sem_post(&nextSem);
        else
            sem_post(&mutex);
    }
    
    //Function that checks if all customers have left the dining hall
    void checkForCustomers(Customer* c){
        sem_wait(&mutex);
        //If there are no more customers to sit, the queue is empty, no more customer eating and all have left the DC, end the program
        if(doneReading && customers.empty() && (customerLeft == totalCustomer))
            customersFinished = true;
    
        if (nextCount > 0)
            sem_post(&nextSem);
        else
            sem_post(&mutex);
    }

};

struct QueueMonitor queue;

//Responsible for adding student to the queue
void *producer(void * arg){
    //Initialization
    int studentID = 0;
    queue.init();
    string line;

    if(file.is_open()){
        getline(file, line);
        string space_delimiter = " ";
        quantumTime = stoi(line); //Get quantum time
      
		while(true){ 
			// Read content
			getline(file, line);
			if(file.eof())
				break;
            
            //Parse line to read arrival time
            size_t pos = line.find(space_delimiter);
            int arrivalTime = stoi(line.substr(0, pos));

            sleep(arrivalTime);//Wait for customer to arrive before adding them o the queue
            Customer *ptrCustomer = new Customer;
            studentID++; 
            (*ptrCustomer).id = studentID; //Assign studentID
            
            

            printf("Arrive %d\n", (*ptrCustomer).id);
            
            //Parse line to read eating time
            line.erase(0, pos + space_delimiter.length());
            pos = line.find(" ");
            string s = line.substr(0, pos);
            (*ptrCustomer).eatingTimeLeft = stoi(line.substr(0, pos));
            (*ptrCustomer).totalEatingTime = stoi(line.substr(0, pos));
            
            //Set beginning of wait time
            chrono::time_point<std::chrono::high_resolution_clock> begin = chrono::high_resolution_clock::now();
            (*ptrCustomer).beginTime = begin;

            queue.addCustomer(ptrCustomer); // Add new customer to queue
            totalCustomer++;
		}
		file.close();
    }
    doneReading = true;
    pthread_exit(NULL);
}


//Responsible for removing student from the queue, sitting them anf readding them to the queue if needed
void *consumer(void *arg){
    //Initalization
    int turnAround, wait;
    double temp;
    using milli = std::chrono::duration<double, std::chrono::seconds::period>;

    while(true){
        
        Customer *chosenCustomer = queue.getCustomer(); //Sit customer
        chrono::time_point<std::chrono::high_resolution_clock> endWaitTime = chrono::high_resolution_clock::now();
        if((*chosenCustomer).fakeCustomer){ //All customers have left, exit the thread
            break;
        }
        //End of waiting time
        
        printf("Sit %d\n", (*chosenCustomer).id);
        
        //Readd them to the queue if their eating time is longer than the quantum time
        if((*chosenCustomer).eatingTimeLeft <= quantumTime)
            sleep((*chosenCustomer).eatingTimeLeft);
        else{
            int newEatingTime = (*chosenCustomer).eatingTimeLeft - quantumTime;
            sleep(quantumTime);
            printf("Preempt %d\n", (*chosenCustomer).id);
            (*chosenCustomer).eatingTimeLeft = newEatingTime; //Update new eating time
            temp = milli(endWaitTime - (*chosenCustomer).beginTime).count();
            (*chosenCustomer).waitTime = (*chosenCustomer).waitTime + temp; //Add the first wait time
            (*chosenCustomer).beginTime = chrono::high_resolution_clock::now(); //Start waiting again
            queue.addCustomer(chosenCustomer);
            continue;

        }
        //Calculate wait time and turn around time
        chrono::time_point<std::chrono::high_resolution_clock> endTurnAround = chrono::high_resolution_clock::now();
        wait = round((*chosenCustomer).waitTime) + round(milli(endWaitTime - (*chosenCustomer).beginTime).count());
        turnAround = wait + (*chosenCustomer).totalEatingTime;
        printf("Leave %d Turnaround %d Wait %d\n", (*chosenCustomer).id, turnAround, wait);
        
        customerLeft++;
       // queue.checkForCustomers(chosenCustomer);
          if(doneReading && queue.customers.empty() && (customerLeft == totalCustomer))
            customersFinished = true;
    }
    totalThread++;
    pthread_exit(NULL);
}

//Responsible for killing all the consumer threads
void *exitAll(void *arg){
    while(!customersFinished);
    for(int i = 0; i < 4; i++){
            Customer *ptrFake = new Customer;
            (*ptrFake).id = i;
            (*ptrFake).eatingTimeLeft = 1;
            (*ptrFake).fakeCustomer = true;
            queue.addCustomer(ptrFake);
            
    }
    //Wait for all the consumer threads to exit before exiting.
    while(totalThread != 4);
    pthread_exit(NULL);
}

int main()
{
    //Ask user for the file name
    string fileName;
    cout << "Please enter the input file name: ";
	getline(cin, fileName);

    file.open(fileName);
    vector <pthread_t> tids;

	for(int i = 1; i <= 6; i++){
		pthread_t tid;
		int * arg = new int;
		*arg = i;
        if (i == 1)
            pthread_create(&tid, NULL, producer, arg); //Producer thread
        else if(i == 6)
            pthread_create(&tid, NULL, exitAll, arg); //exit all threads thread
		else
            pthread_create(&tid, NULL, consumer, arg); //Consumer thread
		tids.push_back(tid);		
	}
	for(int i =0; i < tids.size(); i++){
		pthread_join(tids.at(i), NULL);
	}
    queue.destroy();
	return 1;
}