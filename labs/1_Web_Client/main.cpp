#include <cctype>
#include <cstdlib>
#include <iostream>
#include <unistd.h>

#include "Downloader.h"

using namespace std;

bool isNumber(std::string arg) {
	for (unsigned int i = 0; i < arg.size(); i++) {
		if (!isdigit(arg[i])) {
			return false;
		}
	}
	return true;
}

int main(int argc, char** argv) {
	// Parse options, set flags
	std::string hostname, url;
	unsigned int port;
	unsigned int count = 1;
	bool debug = false;

	int option;
	while ((option = getopt(argc, argv, "dc:")) != -1) {
		switch (option) {
		case 'd':
			debug = true;
			break;
		case 'c':
			count = atoi(optarg);
		}
	}

	if (argc - optind < 3) {
		std::cout << "USAGE: ./download <host_name> <port> <url> [-d] [-c:]" << std::endl;
		return 0;
	}

	if (!isNumber(std::string(argv[optind + 1]))) {
		std::cout << "USAGE: <port> should be an unsigned integer" << std::endl;
		return 0;
	}

	hostname = std::string(argv[optind]);
	port = atoi(argv[optind + 1]);
	url = std::string(argv[optind + 2]);

	// Execute main program
	Downloader(hostname, port, url, debug, count).run();
	return 0;
}
