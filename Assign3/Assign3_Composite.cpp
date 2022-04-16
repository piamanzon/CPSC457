
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <algorithm>
#include <stdio.h>
#include <sstream>
#include <chrono>
#include <cstdlib>
#include <random>
#include <iterator>



using namespace std;
#define VECTOR_SIZE 10000000
#define NUMBER_RANGE 100000

vector <pthread_t> tids;
vector <int> inputIntegers(VECTOR_SIZE);
vector <int> local(VECTOR_SIZE);

void* getTotalComposite(void * arg);

int getRandomNum(int low, int high){
    random_device rDevice;
    mt19937 gen(rDevice());
    uniform_int_distribution<> dis(low, high);

    return dis(gen);
}

void genRandomVector (){
    random_device rDevice;
    mt19937 gen(rDevice());
    uniform_int_distribution<> dis(0, 100000);
	for(int i =0; i < VECTOR_SIZE; i ++){
        
		inputIntegers[i] = dis(gen);
	}
}

int totalComposite = 0;
int totalThreads;
//cite: https://www.geeksforgeeks.org/composite-number/  

void* getTotalComposite(void* arg){

    int index = *(int*) arg;
	int lower = index * (VECTOR_SIZE/totalThreads);
	int upper = min((index+1)*((VECTOR_SIZE/totalThreads)), VECTOR_SIZE);
	for(int i =lower; i < upper ; i ++){
		
        if (inputIntegers.at(i) <= 3)
            continue;
        
        if (inputIntegers.at(i)%2 == 0 || inputIntegers.at(i)%3 == 0){ 
            local.at(i) = 1;
            continue;
        }
        for (int j = 5; j*j <= inputIntegers.at(i); j = j+6){
            if (inputIntegers.at(i)%j == 0 || inputIntegers.at(i)%(j+2) == 0){
                local.at(i) = 1;
                continue;
            }
            j=j+6;
        }
	}
	pthread_exit(0);       
}


int main(int argc, char* argv[]){

	
    if (argc < 2 || argc > 2 ) {
        cerr << "Please enter one input : number of threads \n";
        return 1;
    }
    int inputThread = atoi(argv[1]);

    if(inputThread < 1){
        cerr << "Please enter a valid thread number \n";
        return 1;
    }
    totalThreads = inputThread;
    genRandomVector();

    //Start timer
    auto start = chrono::system_clock::now();

    for(int i =0; i < totalThreads; i++){
		pthread_t tid;
		int *arg = new int;
		*arg = i;
		pthread_create(&tid, NULL, getTotalComposite, arg);
		tids.push_back(tid);		
	}

	for(int i =0; i < tids.size(); i ++){
		pthread_join(tids[i], NULL);  
	}
    //End timer
    auto end = chrono::system_clock::now();
	chrono::duration<double> diff = end - start;

    for(int j =0; j < VECTOR_SIZE; j ++){
        totalComposite += local.at(j);
    }

    cout << "Number of threads: " << totalThreads << "\n";
    cout << "Size of vector: " << VECTOR_SIZE << "\n";
    cout << "Range of numbers: " << NUMBER_RANGE << "\n";
    cout << "Total number of composites: " << totalComposite << "\n";
    cout << "time elapsed(ms): " << int(diff.count()*1000) << '\n';
	return 0;
}
