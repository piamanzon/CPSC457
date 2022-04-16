
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <algorithm>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>


using namespace std;

//Execute command without the dollar sign using system()
//Used fork and wait for background process
void execvpNoDollar(const char* systemInput, bool background){
	int childPID = fork();
	if(childPID == -1){
        	cout <<"Error forking child\n";
                return ;
        }
	else if(childPID == 0){
		int sysOut = system(systemInput);
		if(sysOut < 0){
			cout <<"Error with executing commands!\n";
			return;
		}	
	}
	else{
		if(!background)
			wait(NULL);
	}
	return;
}

//Parse string input then convert output to vector
vector<string> parseInput (string input, string delimiter){
	int inputIndex;
	int storageIndex = 0;
	vector <string> tokens;
	while((inputIndex = input.find(delimiter)) != string::npos){
		tokens.push_back(input.substr(0, inputIndex));
		input.erase(0, inputIndex+1);
	}
	tokens.push_back(input.substr(0, inputIndex));
	return tokens;
}

//Convert one vect command to char* []
void vectToCharPtr (vector <string> inputVect, char* outputCharPtr[], int startingIndex){
	outputCharPtr[0] = new char[inputVect.at(startingIndex).length() + 1];
	strcpy(outputCharPtr[0], inputVect.at(startingIndex).c_str());
	outputCharPtr[1] = NULL;      
}

//Execute command with dollar sign
//Used fork and wait for background process
int execvpPipe(vector <string> tokenVect, bool background){
	vector <string> leftSubstring;
	vector <string> rightSubstring;
	int res;
	char* cmd1[2];
	char* cmd2[2];
	char* cmd3[2]; //one command (no args) + NULL

	//Find index of $
	//==========================================================================
	//The following block of code was modified from code I found at the following URL:
	//https://thispointer.com/c-how-to-find-an-element-in-vector-and-get-its-index/
	
	int dollarIndex = -1;
	int i;
	for( i =0; i< tokenVect.size(); i++){
		if (tokenVect[i] == "$"){
			dollarIndex = i;
			break;
		}
	}
	if(dollarIndex == -1){
		cout <<"No dollar sign inside execpipe \n";	
		res = -1;
		return res;
	}
	//=========================== end of attributed code========================

	//Split vector to left side of $ and right side of $
	for (int i = 0; i < tokenVect.size(); i++){
		if (i < dollarIndex)
			leftSubstring.push_back(tokenVect[i]);
		else if(i > dollarIndex)
			rightSubstring.push_back(tokenVect[i]);
	}
	
	//Depending on the number of commands on the left and right side of the dollar sign, we will execute commands accordingly
	//Will use file redirection instead of pipe

	//==========================================================================
        //The following code outline was from Alex's tutorial (Week5)
	//Only the skeleton was used (primarily for cmd $ cmd cmd), 
	//the rest which includes the forking order, loop and the implementation of the other conditions are written by myself

		
	// cmd $ cmd cmd
	if (leftSubstring.size() == 1 && rightSubstring.size()	== 2){
		for (int i = 0; i< 2; i++){
                	int fds2[2];
               		pipe(fds2);

			int childPID_1 = fork();
               		if(childPID_1 == -1){
                        	cout <<"Error forking child1\n";
                 		return -1;
               		}

            		else if(childPID_1 == 0){
                         	vectToCharPtr(leftSubstring, cmd1, 0);
                         	dup2(fds2[1], fileno(stdout));
                         	close(fds2[0]);
                         	close(fds2[1]);

				if(execvp(cmd1[0], cmd1) < 0){
                                	cout << "Error with execvp \n";
                                	return -1;
                        	}
 			}
                	else{
                        	int childPID_2 = fork();
                        	if(childPID_2 == -1){
                                	cout <<"Error forking child2\n";
                                	return -1;
                        	}	
                        	else if(childPID_2 == 0){
                        		dup2(fds2[0], fileno(stdin));
                        		close(fds2[0]);
                        		close(fds2[1]);	
				
					vectToCharPtr(rightSubstring, cmd2, i);
 					if(execvp(cmd2[0], cmd2) < 0){
                                        	cout << "Error with execvp \n";
                                        	return -1;
                                	}
                        	}	
                        	else{
					close(fds2[0]);
					close(fds2[1]);
					if(!background)
                                                wait(NULL);
				}
				if(!background)
                                	wait(NULL);
			}
		}
		res = 1;

	} 
	// cmd cmd $ cmd 
	else if ( leftSubstring.size() == 2 && rightSubstring.size() == 1){
		int fds[2];
		pipe(fds);

		int childPID_1 = fork();

		if(childPID_1 == -1){
			cout <<"Error forking child1\n";
			return -1;
		}

		else if(childPID_1 == 0){
			
			vectToCharPtr(leftSubstring, cmd1, 0);
			dup2(fds[1], fileno(stdout));
			close(fds[0]);
			close(fds[1]);	
			
			if(execvp(cmd1[0], cmd1) < 0){
                        	cout << "Error with execvp \n";
                                return -1;
                        }
		}
		else{
			int childPID_2 = fork();
			if(childPID_2 == -1){
				cout<< "Error forking child2\n";
				return -1; 
			}
			else if(childPID_2 == 0){
				dup2(fds[1], fileno(stdout));
				close(fds[0]);
				close(fds[1]);
				vectToCharPtr(leftSubstring, cmd2, 1);
				
				if(execvp(cmd2[0], cmd2) < 0){
			      		cout << "Error with execvp \n";
					return -1;
				}
			}
			else{
				int childPID_3 = fork();
				if(childPID_3 == -1){
				        cout <<"Error forking child2\n"; 
					return -1;
				}
				else if(childPID_3 == 0){ 
					vectToCharPtr(rightSubstring, cmd3, 0);
					dup2(fds[0], fileno(stdin));
					close(fds[0]);
					close(fds[1]);
					
					if(execvp(cmd3[0], cmd3) < 0){
						cout << "Error with execvp \n";
						return -1; 
					}
				}
				else{
					close(fds[0]);
					close(fds[1]);
					if(!background)                                                                                                                     	                                                         wait(NULL);
				}
				if(!background)                                                                                                 	                                                                             wait(NULL);
			}
			if(!background)                                                                                 	                                                                                             wait(NULL);
		}
		res = 1;
		
	}
       	// cmd cmd $ cmd cmd 	
	else if ( leftSubstring.size() == 2 && rightSubstring.size() == 2){
		for (int i = 0; i< 2; i++){	
			int fds2[2];
			pipe(fds2);
			
			int childPID_1 = fork();
               		if(childPID_1 == -1){
                        	cout <<"Error forking child1\n";
                        	return -1;
                	}

			else if(childPID_1 == 0){
				vectToCharPtr(leftSubstring, cmd1, 0);
				dup2(fds2[1], fileno(stdout));
				close(fds2[0]);
				close(fds2[1]);
				
				if(execvp(cmd1[0], cmd1) < 0){
                        		cout << "Error with execvp \n";
                                	return -1;
                        	}
                	}
                	else{
                        	int childPID_2 = fork();
                        	if(childPID_2 == -1){
                                	cout <<"Error forking child2\n";
                                	return -1;
                        	}
                        	else if(childPID_2 == 0){
              				dup2(fds2[1], fileno(stdout));
					close(fds2[0]); 
		  			close(fds2[1]);
			 		vectToCharPtr(leftSubstring, cmd2, 1);	
				
					if(execvp(cmd2[0], cmd2) < 0){
                                        	cout << "Error with execvp \n";
                                        	return -1;
                                	}
                        	}
                       		else{
                                	int childPID_3 = fork();
                                	if(childPID_3 == -1){
                                        	cout <<"Error forking child2\n";
                                        	return -1;
                                	}
                                	else if(childPID_3 == 0){
                                        	vectToCharPtr(rightSubstring, cmd3, i);
						dup2(fds2[0], fileno(stdin));
						close(fds2[0]);
                                        	close(fds2[1]);

                                        	if(execvp(cmd3[0], cmd3) < 0){
                                                	cout << "Error with execvp \n";
                                                	return -1;
                                        	}
                                	}
                                	else{
                                        	close(fds2[0]);
                                        	close(fds2[1]);

						if(!background)
							wait(NULL);
					}	
					if(!background)
						wait(NULL);				
			       	}

				if(!background)
					wait(NULL);
			}
		}
                res = 1;
	}
	
	res = 1;

	return res;

}

int main(){
	// Parse user inout
	string userInput;
	vector <string> tokenVect;
	string spaceDelimiter = " ";

	//Background process
	bool background = false;
	
	//System
	const char* systemInput;

	// For piping
	bool hasDollarSign = false;
	int executePiping;

	while(true){
		cout << "Assign1_Shell> ";
		getline(cin, userInput);
		
		//If user enter exit, quit the program!
		if ((userInput == "exit") || cin.eof()){
			exit(0);
		}
		else{
	
			tokenVect = parseInput(userInput, spaceDelimiter);
			//Check for dollar sign
			if(std::find(tokenVect.begin(), tokenVect.end(), "$") != tokenVect.end())
				hasDollarSign = true;
		
			//Check for ampersand (&)
			if(std::find(tokenVect.begin(), tokenVect.end(), "&") != tokenVect.end()){
				background = true;
				tokenVect.erase(std::remove(tokenVect.begin(), tokenVect.end(), "&"), tokenVect.end());
			}
		
			if(!hasDollarSign){
				systemInput = userInput.c_str();
				execvpNoDollar(systemInput, background);
			}
			else{
				executePiping = execvpPipe(tokenVect, background);
				if(executePiping == -1)
					cout << "Error in executing the pipe commands \n";
			}
			
		}
		hasDollarSign = false;
		background = false;
	}
	return 0;
}
