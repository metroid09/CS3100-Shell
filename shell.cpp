/**
* Chris Merkley
* A01126615
*
* The following contains references used in the making of this program along with the initial framework provided by Prof. Falor
* https://stackoverflow.com/a/236803 //String Split function, so much more elegant and clean than the ideas that I had in my head. Really this should be in the string class or something
* 
 */

#include<iostream>
#include<unistd.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<string>
#include<vector>
#include<chrono>
#include "StringFunctions.h"

void printHistory(std::vector<std::string>);
void execCommand(std::string cmd, std::vector<std::string> &history, double &ptime);
bool in(std::vector<std::string> v);

int main(void) {
	//Allocate a vector for storing history until we are done running commands
	std::vector<std::string> history;
	double ptime = 0.0;
	char cwd[256];
	getcwd(cwd, sizeof(cwd));

	// forever (until "exit is entered by the user
	while (true) {
		// print a prompt
		std::cout << "shell$ ";
		std::string cmd;
		getline(std::cin, cmd); //At this point, I need to 'tokenize' cmd

		//BUILT IN COMMANDS
		
		// we might decide if the input is a builtin command
		if (cmd == "exit") {
			break;
		} else {
			if (in(cmd, builtInCommands())) {

			} else {
				execCommand(cmd, history);
			}
		}
	}

	return 0;
}

void printHistory(std::vector<std::string> history) {
	for (int i = 0; i < (int)history.size(); i++) {
		std::cout << i+1 << " : " << history[i] << std::endl;
	}
}

void execCommand(std::string cmd, std::vector<std::string> &history, double &ptime) {
	if (fork()) {
		//parent
		auto start = std::chrono::high_resolution_clock::now();
		wait(0);
		auto end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> diff = end-start;
		ptime += diff.count();

		std::cout << "Ran for " << diff.count() << " seconds" << std::endl;
	} else {
		// IF child, execute the user's input as a command
		auto vec = split(cmd, ' '); //Split command based on spaces (return vector)
		char **args = new char*[vec.size() + 1]; //

		for (int i = 0; i < (int)vec.size(); i++) { //Makes args for passing to execvp()
			args[i] = (char*)vec[i].c_str();
		}
		args[vec.size() + 1] = (char*)NULL;

		if (cmd == "history") {
			printHistory(history);
			history.push_back(cmd);
			exit(0);
		}

		int result = execvp(args[0], args);

		if (-1 == result) { //vect must be null string terminated
			// command not found, or similar error
			std::cerr << args[0] << " failed!\n";
			exit(1);
		} else {
			history.push_back(cmd);
		}
	}
}

bool in(std::string check, std::vector<std::string> v) {
	for (auto s : v) {
		if (check.compare(s) == 0) {
			return true;
		}
	}
	return false;
}

std::vector<std::string> builtInCommands() {
	std::vector<std::string> v = {
		"history",
		"ptime",
		"exit",
	};
}