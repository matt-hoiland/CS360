#include <iostream>
#include <sstream>
#include <stdexcept>

#include <signal.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "Server.h"
#include "DirectoryListing.h"

using namespace std;

struct args_t {
	bool debug;
	unsigned int port;
	const char* dir;
} args;

void parse_args(int argc, char** argv, struct args_t* args);
void setUpHandlers();

//Handlers
void mainHandler(int status);
void handleSegFault();
void handleInterupt();
void handlePipe();
void handleHangUp();

int main(int argc, char** argv) {

	parse_args(argc, argv, &args);
	if (args.debug) cout << "Arguments successfully parsed, port: " << args.port << ", dir: " << args.dir << endl;

	setUpHandlers();
	if (args.debug) cout << "Handlers created." << endl;

	Server(args.debug, args.port, args.dir).run();

	return 0;
}

void parse_args(int argc, char** argv, struct args_t* args) {
	if (argc < 3 || argc > 4) {
		cout << "USAGE: <port> <dir> [-d]" << endl;
		exit(-1);
	}

	bool debug = false;
	int option;
	while ((option = getopt(argc, argv, "d")) != -1) {
		switch (option) {
		case 'd' :
			debug = true;
			break;
		default:
			cout << "USAGE: <port> <dir> [-d]" << endl;
			exit(-1);
		}
	}

	int port;
	istringstream num(argv[optind]);
	if (!(num >> port) || port < 0) {
		cout << "USAGE: <port> should be a non-negative number, given: " << argv[optind] << endl;
		exit(-1);
	}

	const char* dir = argv[optind + 1];
	struct stat dirstat;

	if (stat(dir, &dirstat)) {
		cout << "ERROR: failed to stat " << dir << endl;
		exit(-1);
	}
	if (!S_ISDIR(dirstat.st_mode) || !(dirstat.st_mode & S_IROTH)) {
		cout << "USAGE: <dir> should be a public directory, given: " << dir << endl;
		exit(-1);
	}

	args->debug = debug;
	args->port = port;
	args->dir = dir;
}

void setUpHandlers() {
	struct sigaction sigold, signew;

	signew.sa_handler = mainHandler;
	sigaction(SIGSEGV, &signew, &sigold);
	sigaction(SIGINT, &signew, &sigold);
	sigaction(SIGPIPE, &signew, &sigold);
	sigaction(SIGHUP, &signew, &sigold);
}


void mainHandler(int status) {
	cout << "status: " << status << endl;
	switch(status) {
	case SIGSEGV:
		handleSegFault();
		break;
	case SIGINT:
		handleInterupt();
		break;
	case SIGPIPE:
		handlePipe();
		break;
	case SIGHUP:
		handleHangUp();
		break;
	default:
		if (args.debug) cout << "Unknown signal received: " << status << endl;
	}
}

void handleSegFault() {
	if (args.debug) cout << "Segmentation fault caught, shutting down." << endl;
	exit(-1);
}

void handleInterupt() {
	if (args.debug) cout << "Interrupt signal caught, pressing forward." << endl;
}

void handlePipe() {
	if (args.debug) cout << "Pipe signal caught, pressing forward." << endl;
}

void handleHangUp() {
	if (args.debug) cout << "Hang up signal caught, pressing forward." << endl;
}
