#include <cstdlib>
#include <iostream>
#include <unistd.h>

#include "Downloader.h"

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

	if (argc < 4) {
		std::cout << "USAGE: ./download <host_name> <port> <url> [-d] [-c:]" << std::endl;
		return 0;
	}

	if (!isNumber(std::string(argv[2]))) {
		std::cout << "USAGE: <port> should be an unsigned integer" << std::endl;
		return 0;
	}
	hostname = std::string(argv[1]);
	port = atoi(argv[2]);
	url = std::string(argv[3]);

	int c;

	while ((c = getopt(argc, argv, "dc:")) != -1) {
		switch (c) {
		case 'd':
			debug = true;
			break;
		case 'c':
			count = atoi(optarg);
		}
	}

	// Execute main program
	Downloader(hostname, port, url, debug, count).run();
	return 0;
}
