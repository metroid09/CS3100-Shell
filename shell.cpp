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
#include<string.h>
#include<vector>
#include<chrono>
#include<time.h>
#include<errno.h>
#include <algorithm>
#include <stdexcept>
#include "StringFunctions.h"

void printVec(std::vector<std::string> v);

void printHistory(std::vector<std::string>);
void printPtime(double ptime);
void changeDirectory(std::string dir);
void getWorkingDir();
void pushd(std::string dir);
void popd(std::vector<std::string> &dirHistory);
void printRunTime();
std::string getDirFromCommand(std::string dir);
void execCommand(std::string cmd, std::vector<std::string> &history, double &ptime);
int runBuiltInCommand(std::string &cmd, std::vector<std::string> &history, double &ptime, std::string firstWord, std::vector<std::string> &dirHistory);
std::vector<std::string> builtInCommands();
bool in(std::string check, std::vector<std::string> v);
std::vector<std::string> cleanCmdVector(std::vector<std::string> cmdVec);
void wasteTime(); //For checking if printRunTime is working

int main(void) {
	//Allocate a vector for storing history until we are done running commands
	std::vector<std::string> history;
	std::vector<std::string> dirHistory;
	double ptime = 0.0;
	char cwd[256];
	getcwd(cwd, sizeof(cwd));

	// forever (until "exit is entered by the user
	while (true) {
		// print a prompt
		std::cout << "[cmd:] ";
		std::string cmd = "";
		getline(std::cin, cmd); //At this point, I need to 'tokenize' cmd
		if (cmd.compare("\n") == 0 || cmd.compare("\r\n") == 0 || cmd.compare("") == 0) {
			continue;
		}
		trim(cmd); //Trims command in place (allows for any amount of trailing and leading whitespace)
		auto cmdVec = split(cmd, ' '); //Added for cd, ^ and other commands that start with a command and are built in.

		history.push_back(cmd); // Push history before we run command
		if (in(cmdVec[0], builtInCommands())) { //If commands is in builtInCommands
			auto result = runBuiltInCommand(cmd, history, ptime, cmdVec[0], dirHistory);
			if (result == 0) {
				continue;
			} else if (result == 1) {
				break;
			} else {
				std::cout << "Were you trying to break something? An error occured and I wasn't able to recover from it! Sorry about that." << std::endl;
				break; 
			}
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

void changeDirectory(std::string dir) {
	chdir(dir.c_str());
}

void getWorkingDir() {
	char cwd[256];
	getcwd(cwd, sizeof(cwd));
	std::cout << cwd << std::endl;
}

void pushd(std::string dir, std::vector<std::string> &dirHistory) {
	char cwd[256];
	getcwd(cwd, sizeof(cwd));
	std::string directory = cwd;
	dirHistory.push_back(directory);
	chdir(dir.c_str());
}

void popd(std::vector<std::string> &dirHistory) {
	if (dirHistory.empty()) {
		std::cout << "No stored directories" << std::endl;
		return;
	}
	chdir(dirHistory.back().c_str());
	dirHistory.pop_back();
}

void printRunTime() {
	std::clock_t timeRun = clock()/CLOCKS_PER_SEC;
	std::cout << timeRun << std::endl;
	int hours = (int)(timeRun / 3600);
	int minutes = (int)(((timeRun/3600)-hours)*60);
	double seconds = (((((timeRun/3600.0)-hours)*60.0)-minutes)*60.0);
	char minutesZero = (char)NULL;
	char secondsZero = (char)NULL;
	if (minutes < 10) {
		minutesZero = '0';
	}
	if (seconds < 10) {
		secondsZero = '0';
	}
	std::cout.precision(3);
	std::cout << "Time this process has spent in the running state: " << hours << ":" << minutesZero << minutes << ":" << secondsZero << std::fixed << seconds << std::endl;
}

void wasteTime() {
	std::cout << "Wasting a little time in the running state so that running_time will show it's working" << std::endl;
	bool done = false;
	int n = 1;
	for (int i = 0; i < 2; i++) {
		done = false;
		while (!done) {
			n++;
			if (n == 0) {
				done = true;
			}
		}
	}
}

void printDirs(std::vector<std::string> &dirHistory) {
	if (dirHistory.empty()) {
		getWorkingDir();
		return;
	}
	for (auto s : dirHistory) {
		std::cout << s << " ";
	}
	char cwd[256];
	getcwd(cwd, sizeof(cwd));
	std::cout << cwd << std::endl;
}

std::string getDirFromCommand(std::string dir) {
	auto cmdVec = split(dir, ' ');
	cmdVec.erase(cmdVec.begin());
	return cmdVec.front();
}

void execCommand(std::string cmd, std::vector<std::string> &history, double &ptime) {
	if (fork()) {
		int wstatus = 2;
		//parent
		auto start = std::chrono::high_resolution_clock::now();
		wait(&wstatus);
		auto end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> diff = end-start;
		ptime += diff.count();
	} else {
		// IF child, execute the user's input as a command
		auto vec = split(cmd, ' '); //Split command based on spaces (return vector)
		vec = cleanCmdVector(vec);
		char **args = new char*[vec.size()]; //

		for (int i = 0; i < (int)vec.size(); i++) { //Makes args for passing to execvp()
			args[i] = (char*)vec[i].c_str();
		}
		args[vec.size()] = (char*)NULL;

		int result = execvp(args[0], args);

		if (-1 == result) { //vect must be null string terminated
			// command not found, or similar error
			std::cout << "Command: " <<  args[0] << std::endl;
			std::cerr << "Error: " << strerror(errno) << std::endl;
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
		"^",
		"cd",
		"pwd",
		"pushd",
		"popd",
		"dirs",
		"running_time",
		"waste"
	};

	return v;
}


/*
	Returns 0 for continue, 1 for break
*/
int runBuiltInCommand(std::string &cmd, std::vector<std::string> &history, double &ptime, std::string firstWord, std::vector<std::string> &dirHistory) {
	if (firstWord.compare("history") == 0) {
		printHistory(history);
		return 0;
	} else if (firstWord.compare("ptime") == 0){
		printPtime(ptime);
		return 0;
	} else if (firstWord.compare("exit") == 0) {
		return 1;
	} else if (firstWord.compare("cd") == 0) {
		changeDirectory(getDirFromCommand(cmd));
		return 0;
	} else if (firstWord.compare("pwd") == 0) {
		getWorkingDir();
		return 0;
	} else if (firstWord.compare("dirs") == 0) {
		printDirs(dirHistory);
		return 0;
	} else if (firstWord.compare("pushd") == 0) {
		pushd(getDirFromCommand(cmd), dirHistory);
		return 0;
	} else if (firstWord.compare("popd") == 0) {
		popd(dirHistory);
		return 0;
	} else if (firstWord.compare("running_time") == 0) {
		printRunTime();
		return 0;
	} else if (firstWord.compare("waste") == 0) {
		wasteTime();
		return 0;
	}  else if (firstWord.compare("^") == 0) { // We're trying to run a previous command!!
		cmd = cmd.substr(1, cmd.size());
		trim(cmd);
		int index = 0;
		try {
			index = std::stoi(cmd);
		} catch (const std::invalid_argument& e) {
			std::cout << "You input " << cmd << ", this will not work!" << std::endl;
			std::cerr << "Invalid Argument: Requires an integer!" << std::endl;
			return 0;
		}
		if (index < (int)history.size()) {
			cmd = history[index-1];
			auto cmdVec = split(cmd, ' ');
			if (in(cmdVec[0], builtInCommands())) {
				runBuiltInCommand(cmd, history, ptime, cmdVec[0], dirHistory);
			} else {
				execCommand(cmd, history, ptime);
				return 0;
			}
		} else {
			std::cout << "History doesn't go back that far!" << std::endl;
			return 0;
		}
	}
	return 0;
}

std::vector<std::string> cleanCmdVector(std::vector<std::string> cmdVec) {
	std::vector<std::string> returnVec;
	for (auto s : cmdVec) {
		if (strlen(s.c_str()) == 1) {
			if (*s.c_str() == ' ' || *s.c_str() == (char)NULL) {
				continue;
			} else {
				returnVec.push_back(trim_copy(s));
				continue;
			}
		} else {
			returnVec.push_back(trim_copy(s));
			continue;
		}
	}
	return returnVec;
}

void printVec(std::vector<std::string> v) {
	for (auto s : v) {
		std::cout << s << std::endl;
	}
}