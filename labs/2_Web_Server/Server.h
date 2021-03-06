/*
 * Server.h
 *
 *  Created on: Jan 26, 2016
 *      Author: matt
 */

#ifndef SERVER_H_
#define SERVER_H_

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "ClientHandler.h"

using namespace std;

class Server {
private:
	bool debug;
	unsigned int port;
	const char* dir;

	int server;
	bool serving;

	void create() {
		struct sockaddr_in serverAddress;

		memset(&serverAddress, 0, sizeof(serverAddress));
		serverAddress.sin_family = AF_INET;
		serverAddress.sin_port = htons(port);
		serverAddress.sin_addr.s_addr = INADDR_ANY;

		server = socket(PF_INET, SOCK_STREAM, 0);
		if (!server) {
			perror("socket");
			exit(-1);
		}

		int reuse = 1;
		if (setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
			perror("setsockopt");
			exit(-1);
		}

		if (bind(server, (const struct sockaddr*) &serverAddress, sizeof(serverAddress)) < 0) {
			perror("bind");
			exit(-1);
		}

		if (listen(server, SOMAXCONN) < 0) {
			perror("listen");
			exit(-1);
		}

		if (debug) cout << "Socket successfully created! Ready to server!" << endl;
	}

	void serve() {
		while (serving) {
			struct sockaddr_in clientAddress;
			socklen_t clientLength = sizeof(clientAddress);

			int client = accept(server, (struct sockaddr*) &clientAddress, &clientLength);
			if (!client) {
				perror("accept");
				//exit(-1);
			}
			if (debug) cout << "Client accepted!" << endl;
			ClientHandler(client, dir, debug).handle();
			close(client);
		}

		closeServer();
	}

	void closeServer() {

	}

public:
	Server(bool debug, unsigned int port, const char* dir) :
		debug(debug), port(port), dir(dir), server(0), serving(true) {}
	~Server() {}

	void run() {
		create();
		serve();
	}

	void stop() {

	}
};



#endif /* SERVER_H_ */
