/**
* Chris Merkley
* A01126615
*
* The following contains references used in the making of this program along with the initial framework provided by Prof. Falor
* https://stackoverflow.com/a/236803 //String Split function, so much more elegant and clean than the ideas that I had in my head. Really this should be in the string class or something
* https://stackoverflow.com/a/217605 //String Trim function, Trime should really be in the standard library for string, but this does come from a guy who primarily programs in python and php, fully understanding that c++ is all about doing things differently for different implementations
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
void printPtime(double ptime);
void execCommand(std::string cmd, std::vector<std::string> &history, double &ptime);
std::vector<std::string> builtInCommands();
bool in(std::string check, std::vector<std::string> v);
std::vector<std::string> cleanCmdVector(std::vector<std::string> cmdVec);

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
		trim(cmd); //Trims command in place (allows for any amount of trailing and leading whitespace)

		history.push_back(cmd); // Push history before we run command
		if (in(cmd, builtInCommands()) || cmd[0] == '^') { //If commands is in builtInCommands
			if (cmd.compare("history") == 0) {
				printHistory(history);
			} else if (cmd.compare("ptime") == 0){
				printPtime(ptime);
			} else if (cmd.compare("exit") == 0) {
				break;
			} else if (cmd[0] == '^') { // We're trying to run a previous command!!
				cmd = cmd.substr(1, cmd.size());
				trim(cmd);
				int index = std::stoi(cmd);
				if (index < (int)history.size()) {
					cmd = history[index];
					execCommand(cmd, history, ptime);
					continue;
				}
				std::cout << "History doesn't go back that far!" << std::endl;
			}
			continue;
		} else {
			execCommand(cmd, history, ptime);
		}
	}

	return 0;
}

void printPtime(double ptime) {
	std::cout << "This shell has spent " << ptime << " seconds running commands" << std::endl;
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
	} else {
		// IF child, execute the user's input as a command
		auto vec = split(cmd, ' '); //Split command based on spaces (return vector)
		vec = cleanCmdVector(vec);
		char **args = new char*[vec.size() + 1]; //

		for (int i = 0; i < (int)vec.size(); i++) { //Makes args for passing to execvp()
			args[i] = (char*)vec[i].c_str();
		}
		args[vec.size() + 1] = (char*)NULL;

		std::cout << std::endl << std::endl;
		std::cout << "Here's what we're trying to run right now" << std::endl;
		for (auto i : vec) {
			std::cout << i << std::endl;
		}
		std::cout << std::endl << std::endl;

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

	return v;
}

std::vector<std::string> cleanCmdVector(std::vector<std::string> cmdVec) {
	std::vector<std::string> returnVec;
	for (auto s : cmdVec) {
		if (s.compare(" ") == 0) {
			continue;
		} else {
			returnVec.push_back(trim_copy(s));
		}
	}
	return returnVec;
}